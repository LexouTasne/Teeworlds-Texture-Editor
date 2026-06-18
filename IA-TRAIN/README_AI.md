# Configuracao de IA do Genus

## Onde colocar as keys

Use IA-TRAIN/genus_keys.txt, uma key por linha. Esse arquivo entra no .gitignore e nao deve ir para commit.

Prefixos aceitos:

- gsk_ = Groq
- sk-or-v1- = OpenRouter

Tambem pode usar `IA-TRAIN/.env.local`:

```env
GROQ_API_KEYS=gsk_xxx,gsk_yyy
OPENROUTER_API_KEYS=sk-or-v1-xxx,sk-or-v1-yyy
GENUS_AI_PARALLEL=6
GENUS_AI_TIMEOUT=8
GROQ_MODELS=llama-3.3-70b-versatile,openai/gpt-oss-20b
OPENROUTER_MODELS=openrouter/free
```

## Como funciona

O app chama IA-TRAIN/genus_ai_bridge.py quando voce clica em Enviar. O bridge tenta usar Groq/OpenRouter, pode testar varias keys em paralelo e devolve uma receita de agentes para o editor executar.

Saida esperada do modelo:

```text
agent-select-part hammer
clear
agent-rate
```

Se a IA online falhar, o Genus cai no parser local PT/EN e, se ainda nao souber, abre o Scratch de ensino.

## Importante

Nunca commite genus_keys.txt, .env.local, logs ou ratings com dados privados.
