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
PORCOVOICE_FRAME_LENGTH = 512  # porcupine expects power-of-two frames (depends on library)
SILENCE_CHUNK_SEC = 0.4    # chunk size used when recording prompt to detect silence
SILENCE_RMS_THRESH = 0.01  # adjust to taste

# Wake-word config
# Option A: use built-in keywords (pvporcupine supports some names out-of-the-box)
# Option B: use custom .ppn keyword files (preferred if you have a specific custom wakeword)
USE_CUSTOM_PPN = False
CUSTOM_PPN_PATHS = {
    # index_id: path_to_ppn_file   # example: 0: "jarvis.ppn"
}

# Names for user-readable logging & debounce
DEBOUNCE_SECONDS = 1.5
WAKE_DISPLAY_NAME = "jarvis"

# LLM + TTS config (your existing clients)
tts_client = InferenceClient(provider="fal-ai", api_key=HF_TOKEN)  # used for text-to-speech & ASR if desired
stt_model = "openai/whisper-large-v3"   # if using HF ASR
deepseek_client = OpenAI(base_url="https://router.huggingface.co/v1", api_key=HF_TOKEN)
deepseek_model = "deepseek-ai/DeepSeek-R1:novita"

# -------------------------
# HELPERS
# -------------------------

def transcribe_file_with_hf(filepath):
    """
    Use HuggingFace InferenceClient ASR (if available) to transcribe file.
    Falls back to returning empty string on failure.
    """
    try:
        result = tts_client.automatic_speech_recognition(filepath, model=stt_model)
        text = result.get("text", "") if isinstance(result, dict) else ""
        return text.lower().strip()
    except Exception as e:
        print("ASR failed:", e)
        return ""

def synthesize_tts(text, out_filename=os.path.join(OUTPUT_DIR, "response.wav")):
    """
    Synthesize text using HuggingFace 'fal-ai' provider as in your original code.
    """
    try:
        audio_bytes = tts_client.text_to_speech(text, model="hexgrad/Kokoro-82M")
        with open(out_filename, "wb") as f:
            f.write(audio_bytes)
        # play on macOS (afplay) or Linux (aplay) — try both
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
    """
    Use your DeepSeek client to generate assistant response.
    Cleans <think> tags and markdown like your original.
    """
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
    """
    Record audio into chunks of chunk_duration until we detect a short silence after some audio.
    Returns the filename of the saved wave.
    """
    print("Recording prompt (speak now)...")
    frames = []
    max_chunks = int(30 / chunk_duration)  # safety: don't record forever (30s)
    chunks_recorded = 0
    silence_count = 0
    min_non_silence_chunks = 1

    while True:
        chunk = sd.rec(int(chunk_duration * samplerate), samplerate=samplerate, channels=CHANNELS)
        sd.wait()
        chunks_recorded += 1

        # compute RMS
        rms = np.sqrt(np.mean(chunk.astype(np.float32)**2))
        frames.append(chunk)

        if rms < silence_thresh:
            silence_count += 1
        else:
            silence_count = 0

        # if we've seen some speech and a short run of silence -> stop
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
    """
    Runs in its own thread. Uses pvporcupine to detect wake words.
    When wakeword detected, puts a timestamp into wake_queue.
    """
    try:
        import pvporcupine
        # create porcupine instance
        if USE_CUSTOM_PPN and CUSTOM_PPN_PATHS:
            keyword_paths = list(CUSTOM_PPN_PATHS.values())
            porcupine = pvporcupine.create(keyword_paths=keyword_paths)
        else:
            # example built-in keyword "jarvis" : NOTE availability depends on pvporcupine model;
            # you may have to use a custom .ppn file named jarvis.ppn if built-in not present.
            porcupine = pvporcupine.create(keywords=["jarvis"])
    except Exception as e:
        print("Failed to initialize pvporcupine:", e)
        print("Wakeword listener exiting.")
        return

    print("Wake-word listener started.")
    last_trigger = 0.0

    try:
        with sd.InputStream(channels=1,
                            samplerate=porcupine.sample_rate,
                            blocksize=porcupine.frame_length,
                            dtype="int16") as stream:
            while not stop_event.is_set():
                pcm = stream.read(porcupine.frame_length)[0]
                # pcm is int16 numpy array shape (frame_length, 1) or (frame_length,)
                # pvporcupine.process expects a 1-D array of ints
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
import shutil
def main():
    wake_q = queue.Queue()
    stop_event = threading.Event()

    # start the wakeword listener thread
    listener_thread = threading.Thread(target=wakeword_listener, args=(wake_q, stop_event), daemon=True)
    listener_thread.start()

    try:
        print("Jarvis idle; listening for wake word.")
        while True:
            try:
                # block until a wake-word event or timeout (so we can check for KeyboardInterrupt)
                trigger_time = wake_q.get(timeout=0.5)
            except queue.Empty:
                continue

            # Got a wake-word event: record prompt
            prompt_file = os.path.join(OUTPUT_DIR, f"prompt_{int(time.time())}.wav")
            record_until_silence(filename=prompt_file)

            # Transcribe prompt
            print("Transcribing prompt...")
            user_text = transcribe_file_with_hf(prompt_file)
            print("User said:", user_text)

            if not user_text:
                print("No transcription; skipping.")
                continue

            # Generate LLM response
            print("Generating response from DeepSeek...")
            assistant_text = generate_deepseek_response(user_text)
            print("Assistant:", assistant_text)

            # Synthesize and play TTS
            print("Synthesizing TTS...")
            synthesize_tts(assistant_text)

    except KeyboardInterrupt:
        print("Stopping Jarvis...")
    finally:
        stop_event.set()
        listener_thread.join(timeout=2)

if __name__ == "__main__":
    main()

