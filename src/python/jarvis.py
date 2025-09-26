#!/usr/bin/env python3
"""
Jarvis assistant:
- Low-latency wake-word detection with pvporcupine
- Record user prompt until short silence
- Send prompt to LLM (DeepSeek) and synthesize response
"""

import os
import time
import queue
import threading
import re
import numpy as np
import sounddevice as sd
import soundfile as sf
import shutil
from huggingface_hub import InferenceClient
from openai import OpenAI

# -------------------------
# CONFIG
# -------------------------
HF_TOKEN = os.environ.get("HF_TOKEN")
if not HF_TOKEN:
    raise RuntimeError("Please set HF_TOKEN environment variable")

OUTPUT_DIR = "output-files"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Audio settings
SAMPLERATE = 16000         # 16kHz is sufficient for speech + lower bandwidth for porcupine
CHANNELS = 1
SILENCE_CHUNK_SEC = 0.4    # chunk size used when recording prompt to detect silence
SILENCE_RMS_THRESH = 0.01  # adjust to taste

# Wake-word config
USE_CUSTOM_PPN = False
CUSTOM_PPN_PATHS = {}  # e.g., {0: "jarvis.ppn"}

DEBOUNCE_SECONDS = 1.5
WAKE_DISPLAY_NAME = "jarvis"

# LLM + TTS config
tts_client = InferenceClient(provider="fal-ai", api_key=HF_TOKEN)
deepseek_client = OpenAI(base_url="https://router.huggingface.co/v1", api_key=HF_TOKEN)
deepseek_model = "deepseek-ai/DeepSeek-R1:novita"

# -------------------------
# HELPERS
# -------------------------
def transcribe_file_with_api(filepath):
    try:
        client = OpenAI()
        with open(filepath, "rb") as f:
            transcript = client.audio.transcriptions.create(
                model="gpt-4o-transcribe",   # or "whisper-1" if you want Whisper specifically
                file=f
            )
        return transcript.text.lower().strip()
    except Exception as e:
        print("ASR failed:", e)
        return ""


def synthesize_tts(text, out_filename=os.path.join(OUTPUT_DIR, "response.wav")):
    try:
        audio_bytes = tts_client.text_to_speech(text, model="hexgrad/Kokoro-82M")
        with open(out_filename, "wb") as f:
            f.write(audio_bytes)
        # play on macOS (afplay) or Linux (aplay)
        if os.name == "posix":
            if shutil.which("afplay"):
                os.system(f"afplay {out_filename} &")
            elif shutil.which("aplay"):
                os.system(f"aplay {out_filename} &")
            else:
                print("No system audio player found; saved to", out_filename)
        else:
            print("Saved TTS to", out_filename)
    except Exception as e:
        print("TTS error:", e)

def generate_deepseek_response(prompt_text):
    try:
        completion = deepseek_client.chat.completions.create(
            model=deepseek_model,
            messages=[{"role": "user", "content": prompt_text}],
        )
        raw_text = completion.choices[0].message.content
        clean_text = re.sub(r"<think>.*?</think>", "", raw_text, flags=re.DOTALL)
        clean_text = re.sub(r"(\*\*|__|\*|`)", "", clean_text)
        return clean_text.strip()
    except Exception as e:
        print("DeepSeek request failed:", e)
        return "Sorry, I couldn't get a response."

# -------------------------
# RECORDING PROMPT UNTIL SILENCE
# -------------------------
def record_until_silence(filename=os.path.join(OUTPUT_DIR, "prompt.wav"),
                         silence_thresh=SILENCE_RMS_THRESH,
                         chunk_duration=SILENCE_CHUNK_SEC,
                         samplerate=SAMPLERATE):
    print("Recording prompt (speak now)...")
    frames = []
    max_chunks = int(30 / chunk_duration)
    chunks_recorded = 0
    silence_count = 0
    min_non_silence_chunks = 1

    while True:
        chunk = sd.rec(int(chunk_duration * samplerate), samplerate=samplerate, channels=CHANNELS)
        sd.wait()
        chunks_recorded += 1
        rms = np.sqrt(np.mean(chunk.astype(np.float32)**2))
        frames.append(chunk)

        if rms < silence_thresh:
            silence_count += 1
        else:
            silence_count = 0

        if chunks_recorded >= min_non_silence_chunks and silence_count >= 1:
            break

        if chunks_recorded >= max_chunks:
            print("Reached max prompt length.")
            break

    audio = np.concatenate(frames, axis=0)
    sf.write(filename, audio, samplerate)
    print("Saved prompt to", filename)
    return filename

# -------------------------
# WAKE-WORD THREAD (pvporcupine)
# -------------------------
def wakeword_listener(wake_queue, stop_event):
    try:
        import pvporcupine
        # Auto-select input device
        input_devices = [i for i, d in enumerate(sd.query_devices()) if d['max_input_channels'] > 0]
        if not input_devices:
            raise RuntimeError("No input devices found. Please connect a microphone.")
        device_index = input_devices[0]
        print(f"Using input device index {device_index}: {sd.query_devices(device_index)['name']}")

        # Create porcupine instance
        if USE_CUSTOM_PPN and CUSTOM_PPN_PATHS:
            keyword_paths = list(CUSTOM_PPN_PATHS.values())
            porcupine = pvporcupine.create(
                access_key="WhhwNjiQoDjyek+11peoVRw7Vb99990zYiBp7ykmerFTfToxee6lTg==",
                keyword_paths=keyword_paths
            )
        else:
            porcupine = pvporcupine.create(
                access_key="WhhwNjiQoDjyek+11peoVRw7Vb99990zYiBp7ykmerFTfToxee6lTg==",
                keywords=["jarvis"]
            )

    except Exception as e:
        print("Failed to initialize pvporcupine:", e)
        return

    print("Wake-word listener started.")
    last_trigger = 0.0

    try:
        with sd.InputStream(
            device=device_index,
            channels=1,
            samplerate=porcupine.sample_rate,
            blocksize=porcupine.frame_length,
            dtype="int16"
        ) as stream:
            while not stop_event.is_set():
                pcm = stream.read(porcupine.frame_length)[0]
                pcm = np.frombuffer(pcm, dtype=np.int16)
                result = porcupine.process(pcm)
                if result >= 0:
                    now = time.time()
                    if now - last_trigger < DEBOUNCE_SECONDS:
                        continue
                    last_trigger = now
                    print(f"Wake word detected ({WAKE_DISPLAY_NAME}) at {time.strftime('%X')}")
                    wake_queue.put(now)

    except Exception as e:
        print("Wake-word listener runtime error:", e)
    finally:
        porcupine.delete()
        print("Wake-word listener stopped.")

# -------------------------
# MAIN COORDINATOR
# -------------------------
def main():
    wake_q = queue.Queue()
    stop_event = threading.Event()

    listener_thread = threading.Thread(target=wakeword_listener, args=(wake_q, stop_event), daemon=True)
    listener_thread.start()

    try:
        print("Jarvis idle; listening for wake word.")
        while True:
            try:
                trigger_time = wake_q.get(timeout=0.5)
            except queue.Empty:
                continue

            prompt_file = os.path.join(OUTPUT_DIR, f"prompt_{int(time.time())}.wav")
            record_until_silence(filename=prompt_file)

            print("Transcribing prompt...")
            user_text = transcribe_file_with_api(prompt_file)
            print("User said:", user_text)

            if not user_text:
                print("No transcription; skipping.")
                continue

            print("Generating response from DeepSeek...")
            assistant_text = generate_deepseek_response(user_text)
            print("Assistant:", assistant_text)

            print("Synthesizing TTS...")
            synthesize_tts(assistant_text)

    except KeyboardInterrupt:
        print("Stopping Jarvis...")
    finally:
        stop_event.set()
        listener_thread.join(timeout=2)

if __name__ == "__main__":
    main()

