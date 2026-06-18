# IA real / provider bridge

O Genus agora pode chamar modelos externos OpenAI-compatible por meio de IA-TRAIN/genus_ai_bridge.py.

- Groq usa endpoint OpenAI-compatible: https://api.groq.com/openai/v1/chat/completions
- OpenRouter usa endpoint OpenAI-compatible: https://openrouter.ai/api/v1/chat/completions
- OpenRouter tambem aceita o modelo/router gratuito openrouter/free

Arquivos secretos ficam em IA-TRAIN/genus_keys.txt ou IA-TRAIN/.env.local, ambos ignorados pelo Git.
