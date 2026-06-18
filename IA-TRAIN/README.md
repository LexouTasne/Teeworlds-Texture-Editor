# IA-TRAIN

Pasta local usada pelo agente `Genus` para guardar pedidos de treino do editor.

O app grava eventos em `genus-training.jsonl`. Esse arquivo fica ignorado pelo Git para nao publicar dados de treino do usuario por acidente.

Formato atual de cada linha:

```json
{"agent":"Genus","request":"pedido do usuario","template":"gameskin","part":"hook","tool":"Balde"}
```

Na versao `0.1`, o Genus ainda registra exemplos de treino. A execucao de IA em tempo real entra nas proximas etapas.
