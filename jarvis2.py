"""
Jarvis voice assistant - single-file example
Features:
 - Porcupine wake-word detection ("jarvis" if custom keyword file provided, otherwise built-in "computer")
 - Record audio with sounddevice
 - Whisper (Hugging Face) for STT
 - Meta-Llama-3-8B-Instruct via Hugging Face Inference API
 - Kokoro-82M for TTS via Hugging Face Inference API

Usage:
 - Set environment variables:
     export PICOVOICE_ACCESS_KEY="pv_..."  # from Picovoice Console
     export HUGGINGFACE_API_TOKEN="hf_..."  # for Whisper, Meta-Llama, and Kokoro inference
 - If you have a custom jarvis wake word `.ppn`, set:
     export JARVIS_PPN_PATH="/path/to/jarvis.ppn"
 - Run: python jarvis_voice_assistant.py
"""

import os
import queue
import sys
import tempfile
from contextlib import contextmanager

import numpy as np
import sounddevice as sd
import soundfile as sf
import requests

try:
    import pvporcupine
except Exception as e:
    pvporcupine = None
    print("Warning: pvporcupine not available. Please install pvporcupine package from Picovoice.")

SAMPLE_RATE = 16000
CHANNELS = 1
RECORD_SECONDS_AFTER_WAKE = 6

HF_TOKEN = os.environ.get("HUGGINGFACE_API_TOKEN")
ACCESS_KEY = os.environ.get("PICOVOICE_ACCESS_KEY")
JARVIS_PPN_PATH = os.environ.get("JARVIS_PPN_PATH")


def write_wav(path, data, samplerate):
    sf.write(path, data, samplerate)


@contextmanager
def record_audio(duration_seconds, samplerate=SAMPLE_RATE, channels=CHANNELS):
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp:
        filename = tmp.name
    print(f"Recording {duration_seconds}s -> {filename}")
    recording = sd.rec(int(duration_seconds * samplerate), samplerate=samplerate, channels=channels, dtype='int16')
    sd.wait()
    write_wav(filename, recording, samplerate)
    try:
        yield filename
    finally:
        pass


def transcribe_with_whisper_hf(wav_path):
    if not HF_TOKEN:
        print("HF token missing. Skipping transcription.")
        return None
    model = "openai/whisper-small"
    url = f"https://api-inference.huggingface.co/models/{model}"
    headers = {"Authorization": f"Bearer {HF_TOKEN}"}
    with open(wav_path, "rb") as f:
        data = f.read()
    print("Uploading audio to Whisper (HF) for transcription...")
    resp = requests.post(url, headers=headers, data=data)
    if resp.status_code != 200:
        print("Whisper HF API error:", resp.status_code, resp.text)
        return None
    try:
        j = resp.json()
        if isinstance(j, list) and len(j) > 0 and 'text' in j[0]:
            return j[0]['text']
        if isinstance(j, dict) and 'text' in j:
            return j['text']
        return str(j)
    except Exception as e:
        print('Failed to parse Whisper HF response:', e)
        return None


def query_meta_llama(prompt, max_tokens=512, temperature=0.2):
    if not HF_TOKEN:
        print("HF token missing. Can't call Meta-Llama.")
        return None
    model = "meta-llama/Meta-Llama-3-8B-Instruct"
    url = f"https://api-inference.huggingface.co/models/{model}"
    headers = {"Authorization": f"Bearer {HF_TOKEN}", "Content-Type": "application/json"}
    payload = {"inputs": prompt, "parameters": {"max_new_tokens": max_tokens, "temperature": temperature}}
    print("Calling Meta-Llama Inference API...")
    r = requests.post(url, headers=headers, json=payload, timeout=120)
    if r.status_code != 200:
        print("HF inference error:", r.status_code, r.text)
        return None
    try:
        j = r.json()
        if isinstance(j, list) and len(j) > 0 and 'generated_text' in j[0]:
            return j[0]['generated_text']
        if isinstance(j, dict) and 'generated_text' in j:
            return j['generated_text']
        if isinstance(j, list) and len(j) > 0 and isinstance(j[0], str):
            return j[0]
        return str(j)
    except Exception as e:
        print('Failed to parse HF response:', e)
        return None


def synthesize_kokoro(text, output_wav_path):
    if not HF_TOKEN:
        print("HF token missing. Can't call Kokoro.")
        return False
    model = "hexgrad/Kokoro-82M"
    url = f"https://api-inference.huggingface.co/models/{model}"
    headers = {"Authorization": f"Bearer {HF_TOKEN}"}
    payload = {"inputs": text}
    print("Calling Kokoro TTS inference API...")
    r = requests.post(url, headers=headers, json=payload, timeout=120)
    if r.status_code != 200:
        print("Kokoro inference error:", r.status_code, r.text)
        return False
    if r.content[:4] == b'RIFF':
        with open(output_wav_path, 'wb') as f:
            f.write(r.content)
        return True
    try:
        obj = r.json()
        print("Unexpected JSON response:", obj)
    except Exception:
        pass
    return False


def play_wav(path):
    print(f"Playing: {path}")
    data, sr = sf.read(path, dtype='int16')
    sd.play(data, sr)
    sd.wait()


def run_assistant():
    if pvporcupine is None:
        print("pvporcupine not found. Please install and try again.")
        return
    if not ACCESS_KEY:
        print("PICOVOICE_ACCESS_KEY not set. Exiting.")
        return
    try:
        if JARVIS_PPN_PATH and os.path.exists(JARVIS_PPN_PATH):
            print("Using custom Jarvis wake word file.")
            porcupine = pvporcupine.create(access_key=ACCESS_KEY, keyword_paths=[JARVIS_PPN_PATH])
        else:
            print("Using built-in 'computer' wake word.")
            porcupine = pvporcupine.create(access_key=ACCESS_KEY, keywords=["computer"])
    except Exception as e:
        print("Failed to create Porcupine:", e)
        return

    audio_queue = queue.Queue()

    def audio_callback(indata, frames, time_info, status):
        if status:
            print(status, file=sys.stderr)
        audio_queue.put(bytes(indata))

    stream = sd.InputStream(samplerate=SAMPLE_RATE, blocksize=porcupine.frame_length, channels=1, dtype='int16', callback=audio_callback)

    print("Jarvis is listening... Say wake word to activate.")
    with stream:
        try:
            while True:
                frame = audio_queue.get()
                pcm = np.frombuffer(frame, dtype=np.int16)
                result = porcupine.process(pcm)
                if result >= 0:
                    print("Wake word detected!")
                    with record_audio(RECORD_SECONDS_AFTER_WAKE) as wavfn:
                        text = transcribe_with_whisper_hf(wavfn)
                        print("User said:", text)
                        if not text:
                            continue
                        response = query_meta_llama(text)
                        if not response:
                            continue
                        print("Assistant:", response)
                        outwav = tempfile.NamedTemporaryFile(suffix='.wav', delete=False).name
                        if synthesize_kokoro(response, outwav):
                            play_wav(outwav)
        except KeyboardInterrupt:
            print("Exiting...")
        finally:
            porcupine.delete()


if __name__ == '__main__':
    run_assistant()

