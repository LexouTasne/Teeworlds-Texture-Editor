#!/usr/bin/env python3
import argparse, concurrent.futures, json, os, random, re, sys, time, urllib.error, urllib.request
from pathlib import Path

TRAIN = Path(__file__).resolve().parent
ROOT = TRAIN.parent
KEY_FILE = TRAIN / "genus_keys.txt"
ENV_FILE = TRAIN / ".env.local"
LOG_FILE = TRAIN / "genus-ai-calls.jsonl"

SYSTEM_PROMPT = """
Voce e o roteador treinavel do Genus. Use principalmente os exemplos aprendidos.
Responda SOMENTE com uma receita agent-* executavel, uma linha por comando, sem markdown.
Sempre termine com agent-rate para o usuario dar nota e obs.
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
        "agent-select-tool", "agent-select-all", "agent-select-tudo",
        "agent-select-current-part", "agent-current-part", "agent-auto-select-part",
        "agent-select-current-layer", "agent-current-layer",
        "agent-select-layer", "agent-layer-base", "agent-focus-part", "agent-focus",
        "color", "cor", "source-color", "confidence", "confianca",
        "tolerance", "tolerancia", "size", "opacity", "shape", "agent-pintar",
        "agent-paint", "agent-paint-selection", "agent-fill", "agent-fill-selection",
        "agent-crop-layer", "agent-recorte", "clear",
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

def wanted_color(prompt: str) -> str:
    n = norm(prompt)
    for word, value in COLOR_WORDS.items():
        if word in n:
            return value
    m = re.search(r"#[0-9a-fA-F]{6}", prompt)
    return m.group(0).upper() if m else ""

def extract_target(prompt: str) -> str:
    raw = prompt.strip()
    patterns = [
        r"(?:foca|focar|foco|focus|zoom)\s+(?:para\s+mim\s+)?(?:no|na|o|a|the)?\s*([a-zA-Z0-9_\- ]{2,48})",
        r"(?:part|parte)\s+([a-zA-Z0-9_\- ]{2,48})",
        r"(?:o|a|the)\s+([a-zA-Z0-9_\-]{2,32})",
    ]
    for pat in patterns:
        m = re.search(pat, raw, re.I)
        if not m:
            continue
        target = m.group(1).strip(" .,;:!?")
        target = re.split(r"\s+(?:com|para|pra|de|do|da|with|to|from|em|in)\s+", target, maxsplit=1, flags=re.I)[0].strip()
        if target and target.lower() not in {"part", "parte", "imagem", "template", "tudo", "all"}:
            return target
    if any(w in norm(raw) for w in ["part atual", "parte atual", "selecionada", "selected part"]):
        return "selected"
    return ""

def local_rules(prompt: str) -> str:
    n = norm(prompt)
    target = extract_target(prompt)
    target_line = f"agent-select-part {target}" if target else "agent-select-current-part"
    if any(w in n for w in ["foca", "focar", "foco", "focus", "zoom"]):
        return f"{target_line}\nagent-focus-part\nagent-rate"
    delete = any(w in n for w in ["apaga", "apagar", "remove", "remover", "deleta", "deletar", "delete", "erase"])
    if delete:
        return f"{target_line}\nclear\nagent-rate"
    if any(w in n for w in ["recorte", "recortar", "crop", "cut out"]):
        return f"{target_line}\nagent-select-ferramenta crop\nagent-crop-layer\nagent-rate"
    if any(w in n for w in ["preenche", "preencher", "balde", "fill", "bucket"]):
        color = wanted_color(prompt)
        color_line = f"\ncolor {color}" if color else ""
        return f"{target_line}\nagent-select-ferramenta bucket{color_line}\nagent-fill\nagent-rate"
    if any(w in n for w in ["troca", "muda", "recolor", "pintar", "paint"]):
        src = "#FFD200" if ("amarelo" in n or "yellow" in n) else "#FFD200"
        dst = wanted_color(prompt)
        if dst:
            return f"{target_line}\nsource-color {src}\ncolor {dst}\nconfidence 65\nagent-pintar\nagent-rate"
    return ""

def score_example(prompt: str, request: str) -> int:
    pt = {x for x in norm(prompt).split() if len(x) >= 3}
    rt = {x for x in norm(request).split() if len(x) >= 3}
    return len(pt & rt)

def read_jsonl_examples(path: Path):
    if not path.exists():
        return []
    out = []
    for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        try:
            obj = json.loads(line)
            req = obj.get("request", "")[:180]
            rec = obj.get("recipe", "")[:520]
            if req and rec:
                out.append((req, rec))
        except Exception:
            pass
    return out

def read_training_context(prompt: str, limit=18):
    examples = []
    examples += read_jsonl_examples(TRAIN / "genus-training.jsonl")
    examples += read_jsonl_examples(TRAIN / "genus-simulations.jsonl")
    ranked = sorted(examples, key=lambda x: score_example(prompt, x[0]), reverse=True)
    examples = []
    seen = set()
    for req, rec in ranked[:limit * 3]:
        if (req, rec) in seen:
            continue
        seen.add((req, rec))
        examples.append(f"Pedido: {req}\nReceita:\n{rec}")
        if len(examples) >= limit:
            break
    return "\n\n".join(examples)

def post_chat(url, key, model, prompt, provider, timeout=None):
    if timeout is None:
        timeout = float(os.environ.get("GENUS_AI_TIMEOUT", "8"))
    training = read_training_context(prompt)
    user = "Pedido do usuario:\n" + prompt.strip()
    if training:
        user += "\n\nExemplos aprendidos localmente:\n" + training
    user += "\n\nComandos validos: agent-select-part, agent-auto-select-part, agent-select-current-part, agent-select-all, agent-select-current-layer, agent-layer-base, agent-select-ferramenta, color, source-color, confidence, tolerance, size, opacity, shape, agent-fill, agent-fill-selection, agent-pintar, agent-paint-selection, agent-crop-layer, clear, agent-rate."
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
