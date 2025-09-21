import os
import time
import sounddevice as sd
import soundfile as sf
import numpy as np
import re
from openai import OpenAI
from huggingface_hub import InferenceClient

# --- Setup clients ---
HF_TOKEN = os.environ["HF_TOKEN"]

tts_client = InferenceClient(provider="fal-ai", api_key=HF_TOKEN)
stt_model = "openai/whisper-large-v3"
deepseek_client = OpenAI(base_url="https://router.huggingface.co/v1", api_key=HF_TOKEN)
deepseek_model = "deepseek-ai/DeepSeek-R1:novita"

# --- Audio settings ---
samplerate = 44100
channels = 1
chunk_duration = 3  # seconds
silence_thresh = 0.01
TRIGGERS = ["homie", "jarvis"]

# --- Recording functions ---
def record_audio(filename, duration=chunk_duration):
    audio = sd.rec(int(duration * samplerate), samplerate=samplerate, channels=channels)
    sd.wait()
    sf.write(filename, audio, samplerate)
    return filename

def transcribe(filename):
    return tts_client.automatic_speech_recognition(filename, model=stt_model)["text"].lower()

def synthesize_tts(text, out_filename="response.wav"):
    audio_bytes = tts_client.text_to_speech(text, model="hexgrad/Kokoro-82M")
    with open(out_filename, "wb") as f:
        f.write(audio_bytes)
    os.system(f"aplay {out_filename}")  # plays audio

def detect_trigger(text):
    return any(trigger in text for trigger in TRIGGERS)

def record_until_silence(filename="prompt.wav", silence_thresh=0.01, chunk_duration=1):
    print("Recording prompt...")
    data = []
    while True:
        chunk = sd.rec(int(chunk_duration * samplerate), samplerate=samplerate, channels=channels)
        sd.wait()
        rms = np.sqrt(np.mean(chunk**2))
        if rms < silence_thresh and len(data) > 0:
            break
        data.append(chunk)
    full_audio = np.concatenate(data)
    sf.write(filename, full_audio, samplerate)
    print("Prompt recording done")
    return filename

def generate_deepseek_response(prompt_text):
    completion = deepseek_client.chat_completions.create(
        model=deepseek_model,
        messages=[{"role": "user", "content": prompt_text}],
    )

    # Extract string content
    raw_text = completion.choices[0].message.content

    # Remove <think>...</think> blocks
    clean_text = re.sub(r"<think>.*?</think>", "", raw_text, flags=re.DOTALL)

    # Remove Markdown formatting (**bold**, *italic*, `code`)
    clean_text = re.sub(r"(\*\*|__|\*|`)", "", clean_text)

    return clean_text.strip()

# --- Main assistant loop ---
def main_loop():
    print("Idle listening...")
    while True:
        record_audio("idle-record.wav")
        text = transcribe("idle-record.wav")
        if detect_trigger(text):
            print(f"Trigger detected: {text}")
            prompt_file = record_until_silence()
            finalPrompt = transcribe(prompt_file)
            print(f"User prompt: {finalPrompt}")
            response = generate_deepseek_response(finalPrompt)
            print(f"Assistant response: {response}")
            synthesize_tts(response)

if __name__ == "__main__":
    main_loop()

