import re

raw_text = "<think>Some thinking text</think> The answer is Paris."

clean_text = re.sub(r"<think>.*?</think>", "", raw_text, flags=re.DOTALL)
print(clean_text)

