import os
import re
from openai import OpenAI
from huggingface_hub import InferenceClient

# --- Setup clients ---
HF_TOKEN = os.environ["HF_TOKEN"]

tts_client = InferenceClient(provider="fal-ai", api_key=HF_TOKEN)
stt_model = "openai/whisper-large-v3"
deepseek_client = OpenAI(base_url="https://router.huggingface.co/v1", api_key=HF_TOKEN)
deepseek_model = "deepseek-ai/DeepSeek-R1:novita"

# --- Functions ---
def transcribe(filename):
    return tts_client.automatic_speech_recognition(filename, model=stt_model)["text"].lower()

def generate_deepseek_response(prompt_text):
    completion = deepseek_client.chat_completions.create(
        model=deepseek_model,
        messages=[{"role": "user", "content": prompt_text}],
    )
    raw_text = completion.choices[0].message.content
    # Clean <think> blocks and Markdown formatting
    clean_text = re.sub(r"<think>.*?</think>", "", raw_text, flags=re.DOTALL)
    clean_text = re.sub(r"(\*\*|__|\*|`)", "", clean_text)
    return clean_text.strip()

def synthesize_tts(text, out_filename="output-files/response.wav"):
    audio_bytes = tts_client.text_to_speech(text, model="hexgrad/Kokoro-82M")
    with open(out_filename, "wb") as f:
        f.write(audio_bytes)
    os.system(f"aplay {out_filename}")  # Plays audio

# --- Main ---
if __name__ == "__main__":
    audio_file = "./src/python/output-files/output.wav"
    
    print("Transcribing audio...")
    prompt_text = transcribe(audio_file)
    print(f"Transcribed text: {prompt_text}")

    print("Querying DeepSeek...")
    response = generate_deepseek_response(prompt_text)
    print(f"Assistant response:\n{response}")

    print("Generating TTS...")
    synthesize_tts(response)

