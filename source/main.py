import whisper
import torch

is_cuda_available = torch.cuda.is_available()

if is_cuda_available:
    print(f"Using GPU: {torch.cuda.get_device_name(0)}")
    model = whisper.load_model(name="base", device='cuda')
else:
    print("Using CPU")
    model = whisper.load_model("base")

result = model.transcribe("audio.wav")

print(result["text"])

with open("result.txt","w+",encoding="utf-8") as file:
    file.write(result["text"])

print("Finished!!") 