#!/usr/bin/env python3
import argparse, concurrent.futures, json, os, random, re, sys, time, urllib.error, urllib.request
from pathlib import Path

TRAIN = Path(__file__).resolve().parent
ROOT = TRAIN.parent
KEY_FILE = TRAIN / "genus_keys.txt"
ENV_FILE = TRAIN / ".env.local"
LOG_FILE = TRAIN / "genus-ai-calls.jsonl"

SYSTEM_PROMPT = """
Voce e o cerebro do Teeworlds Texture Editor Genus.
Responda SOMENTE com receita de agentes, uma linha por comando. Sem markdown, sem explicacao.
A receita sera executada por um editor C++.

Comandos aceitos:
agent-select-template <id ou texto>
agent-select-part <id ou texto da part>
agent-select-ferramenta select|pencil|brush|eraser|bucket|crop
color #RRGGBB
source-color #RRGGBB|yellow|blue|red|green|black|white
confidence 0-100
tolerance 0-255
size 1-128
opacity 0-100
shape round|square
agent-pintar
agent-fill
agent-crop-layer
clear
agent-rate

Regras importantes:
- Se o usuario pedir para apagar/remover/deletar uma part, selecione a part e use clear.
- Exemplo: "apaga o hammer" =>
agent-select-part hammer
clear
agent-rate
- Se pedir recorte, use agent-select-ferramenta crop e agent-crop-layer.
- Se pedir para trocar uma cor por outra, use source-color, color, confidence e agent-pintar.
- Use PT-BR ou ingles naturalmente, mas a saida deve ser receita.
""".strip()

COLOR_WORDS = {
    "amarelo": "#FFD200", "yellow": "#FFD200",
    "azul": "#267EFF", "blue": "#267EFF",
    "vermelho": "#E53B3B", "red": "#E53B3B",
    "verde": "#30C96B", "green": "#30C96B",
    "preto": "#000000", "black": "#000000",
    "branco": "#FFFFFF", "white": "#FFFFFF",
}

def norm(s: str) -> str:
    table = str.maketrans("áàâãäéèêëíìîïóòôõöúùûüçÁÀÂÃÄÉÈÊËÍÌÎÏÓÒÔÕÖÚÙÛÜÇ", "aaaaaeeeeiiiiooooouuuucAAAAAEEEEIIIIOOOOOUUUUC")
    return s.translate(table).lower()

def load_env_file(path: Path):
    if not path.exists():
        return
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        k, v = line.split("=", 1)
        os.environ.setdefault(k.strip(), v.strip().strip('"').strip("'"))

def split_keys(value: str):
    return [x.strip() for x in re.split(r"[,;\s]+", value or "") if x.strip()]

def collect_keys():
    load_env_file(ENV_FILE)
    keys = []
    if KEY_FILE.exists():
        for raw in KEY_FILE.read_text(encoding="utf-8", errors="ignore").splitlines():
            k = raw.strip()
            if k and not k.startswith("#"):
                keys.append(k)
    keys += split_keys(os.environ.get("GROQ_API_KEYS", ""))
    keys += split_keys(os.environ.get("OPENROUTER_API_KEYS", ""))
    keys += split_keys(os.environ.get("GROQ_API_KEY", ""))
    keys += split_keys(os.environ.get("OPENROUTER_API_KEY", ""))
    seen, out = set(), []
    for k in keys:
        if k not in seen:
            seen.add(k); out.append(k)
    return out

def valid_recipe(text: str) -> str:
    if not text:
        return ""
    text = text.strip().replace("```txt", "").replace("```", "").strip()
    lines = []
    allowed = (
        "agent-select-template", "agent-select-part", "agent-select-ferramenta",
        "agent-select-tool", "color", "cor", "source-color", "confidence", "confianca",
        "tolerance", "tolerancia", "size", "opacity", "shape", "agent-pintar",
        "agent-paint", "agent-fill", "agent-crop-layer", "agent-recorte", "clear",
        "limpar", "crop", "recorte", "agent-rate", "whole", "full", "part", "tool",
    )
    for raw in text.splitlines():
        line = raw.strip().strip("-").strip()
        if not line or line.startswith("#"):
            continue
        token = norm(line).split()[0] if norm(line).split() else ""
        if token in allowed:
            lines.append(line)
    if not lines:
        return ""
    recipe = "\n".join(lines)
    if "agent-rate" not in norm(recipe):
        recipe += "\nagent-rate"
    return recipe

def local_rules(prompt: str) -> str:
    n = norm(prompt)
    delete = any(w in n for w in ["apaga", "apagar", "remove", "remover", "deleta", "deletar", "delete", "erase"])
    if delete:
        m = re.search(r"(?:apaga|apagar|remove|remover|deleta|deletar|delete|erase)\s+(?:o|a|os|as|the)?\s*([a-zA-Z0-9_\- ]{2,48})", prompt, re.I)
        target = (m.group(1).strip() if m else "").strip(".,;:!")
        if not target:
            target = "selected"
        return f"agent-select-part {target}\nclear\nagent-rate"
    if any(w in n for w in ["recorte", "recortar", "crop", "cut out"]):
        return "agent-select-ferramenta crop\nagent-crop-layer\nagent-rate"
    if any(w in n for w in ["preencher", "balde", "fill", "bucket"]):
        return "agent-select-part\nagent-select-ferramenta bucket\nagent-fill\nagent-rate"
    if any(w in n for w in ["troca", "muda", "recolor", "pintar", "paint"]):
        src = "#FFD200" if ("amarelo" in n or "yellow" in n) else "#FFD200"
        dst = None
        for word, value in COLOR_WORDS.items():
            if word in n:
                dst = value
        if dst:
            return f"agent-select-part\nsource-color {src}\ncolor {dst}\nconfidence 65\nagent-pintar\nagent-rate"
    return ""

def read_training_context(limit=12):
    f = TRAIN / "genus-training.jsonl"
    if not f.exists():
        return ""
    lines = f.read_text(encoding="utf-8", errors="ignore").splitlines()[-limit:]
    examples = []
    for line in lines:
        try:
            obj = json.loads(line)
            req = obj.get("request", "")[:180]
            rec = obj.get("recipe", "")[:500]
            if req and rec:
                examples.append(f"Pedido: {req}\nReceita:\n{rec}")
        except Exception:
            pass
    return "\n\n".join(examples)

def post_chat(url, key, model, prompt, provider, timeout=None):
    if timeout is None:
        timeout = float(os.environ.get("GENUS_AI_TIMEOUT", "8"))
    training = read_training_context()
    user = "Pedido do usuario:\n" + prompt.strip()
    if training:
        user += "\n\nExemplos aprendidos localmente:\n" + training
    body = {
        "model": model,
        "temperature": 0.1,
        "max_tokens": 260,
        "messages": [
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": user},
        ],
    }
    headers = {"Content-Type": "application/json", "Authorization": "Bearer " + key}
    if provider == "openrouter":
        headers["HTTP-Referer"] = "http://localhost/genus"
        headers["X-OpenRouter-Title"] = "Teeworlds Texture Editor Genus"
    req = urllib.request.Request(url, data=json.dumps(body).encode("utf-8"), headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=timeout) as r:
        data = json.loads(r.read().decode("utf-8", errors="ignore"))
    text = data.get("choices", [{}])[0].get("message", {}).get("content", "")
    recipe = valid_recipe(text)
    if not recipe:
        raise RuntimeError("modelo respondeu sem receita valida")
    log_call(provider, model, True, "")
    return recipe

def log_call(provider, model, ok, err):
    try:
        with LOG_FILE.open("a", encoding="utf-8") as f:
            f.write(json.dumps({"ts": int(time.time()), "provider": provider, "model": model, "ok": ok, "err": str(err)[:180]}, ensure_ascii=False) + "\n")
    except Exception:
        pass

def cloud_recipe(prompt: str) -> str:
    keys = collect_keys()
    groq = [k for k in keys if k.startswith("gsk_")]
    openrouter = [k for k in keys if k.startswith("sk-or-v1-")]
    groq_models = split_keys(os.environ.get("GROQ_MODELS", "llama-3.3-70b-versatile,openai/gpt-oss-20b"))
    or_models = split_keys(os.environ.get("OPENROUTER_MODELS", "openrouter/free"))
    jobs = []
    for k in groq:
        for m in groq_models:
            jobs.append(("groq", "https://api.groq.com/openai/v1/chat/completions", k, m))
    for k in openrouter:
        for m in or_models:
            jobs.append(("openrouter", "https://openrouter.ai/api/v1/chat/completions", k, m))
    if not jobs:
        return ""
    random.shuffle(jobs)
    parallel = max(1, min(int(os.environ.get("GENUS_AI_PARALLEL", "6")), len(jobs)))
    with concurrent.futures.ThreadPoolExecutor(max_workers=parallel) as ex:
        futs = {ex.submit(post_chat, url, key, model, prompt, provider): (provider, model) for provider, url, key, model in jobs}
        for fut in concurrent.futures.as_completed(futs):
            provider, model = futs[fut]
            try:
                return fut.result()
            except Exception as e:
                log_call(provider, model, False, e)
    return ""

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--prompt-file")
    ap.add_argument("prompt", nargs="*")
    args = ap.parse_args()
    if args.prompt_file:
        prompt = Path(args.prompt_file).read_text(encoding="utf-8", errors="ignore")
    else:
        prompt = " ".join(args.prompt) or sys.stdin.read()
    prompt = prompt.strip()
    if not prompt:
        print("#AI_EMPTY#")
        return 2
    quick = local_rules(prompt)
    recipe = cloud_recipe(prompt)
    if recipe:
        print(recipe)
        return 0
    if quick:
        print(quick)
        return 0
    print("#AI_OFFLINE#")
    return 1

if __name__ == "__main__":
    raise SystemExit(main())
