import os
from openai import OpenAI
import re
client = OpenAI(
        base_url="https://router.huggingface.co/v1",
        api_key=os.environ["HF_TOKEN"],
        )
completion = client.chat.completions.create(
        model="deepseek-ai/DeepSeek-R1:novita",
        messages=[ { "role": "user",
                    "content": "What is the capital of France?"
                    }
                  ],
        )

raw_text = completion.choices[0].message.content

# Remove <think>...</think> blocks
clean_text = re.sub(r"<think>.*?</think>", "", raw_text, flags=re.DOTALL)

# Remove Markdown formatting (**bold**, *italic*, `code`)
clean_text = re.sub(r"(\*\*|__|\*|`)", "", clean_text)

clean_text = clean_text.strip()

print(clean_text)

