import os
from huggingface_hub import InferenceClient
import sounddevice as sd
import soundfile as sf

# Initialize the client
client = InferenceClient(
    provider="fal-ai",
    api_key=os.environ["HF_TOKEN"],
)

# Record audio from mic (5 seconds example)
fs = 16000  # Sample rate
seconds = 5
print("Recording...")
audio = sd.rec(int(seconds * fs), samplerate=fs, channels=1)
sd.wait()
sf.write("input.wav", audio, fs)

# Read the audio file
with open("input.wav", "rb") as f:
    audio_bytes = f.read()

# Run speech-to-text
result = client.speech_to_text(
    audio_bytes,
    model="openai/whisper-large-v2"  # or any Hugging Face Whisper model
)

print("Transcription:", result)

