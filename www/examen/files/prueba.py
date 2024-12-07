from pyllamacpp.model import Model

def new_text_callback(text: str):
    print(text, end="", flush=True)

model = Model(ggml_model='./models/gpt4all-model.bin', n_ctx=512)
model.generate("Once upon a time, ", n_predict=55, new_text_callback=new_text_callback, n_threads=8)