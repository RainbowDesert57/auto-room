import os
from huggingface_hub import InferenceClient

client = InferenceClient(
    provider="fal-ai",
    api_key=os.environ["HF_TOKEN"],  # or your key directly
)

# Generate audio
audio = client.text_to_speech(
    "The answer to the universe is 42",
    model="hexgrad/Kokoro-82M",
)

# Save to a file
with open("output.wav", "wb") as f:
    f.write(audio)

print("Audio saved to output.wav")

