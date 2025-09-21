import os
from huggingface_hub import InferenceClient

# Make sure HF_TOKEN is exported in your shell
client = InferenceClient(
    provider="fal-ai",
    api_key=os.environ["HF_TOKEN"],
)

# Path to your audio file
audio_file = "./output-files/output.wav"

# Run transcription
output = client.automatic_speech_recognition(audio_file, model="openai/whisper-large-v3")

# Print the recognized text
print(output['text'])

