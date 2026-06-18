# IA-TRAIN

Pasta local usada pelo agente `Genus` para guardar pedidos de treino do editor.

O app grava eventos em `genus-training.jsonl`. Esse arquivo fica ignorado pelo Git para nao publicar dados de treino do usuario por acidente.

Formato atual de cada linha:

```json
{"agent":"Genus","request":"pedido do usuario","template":"gameskin","part":"hook","tool":"Balde"}
```

Na versao `0.1`, o Genus registra exemplos e ja tenta executar receitas dentro do editor.

No modo-dev:

- `Pedido`: o texto natural que voce quer ensinar.
- `Receita`: os passos que o Genus deve repetir.
- `Treinar`: salva pedido + receita.
- `Tentar`: procura um treino parecido; se nao achar, usa o parser interno.

Comandos aceitos na receita:

```text
template gameskin
part Hook_Copy
whole
tool pencil
tool eraser
tool bucket
size 16
opacity 80
tolerance 25
hardness 100
shape round
shape square
color #00AAFF
fill
clear
```

Exemplo:

```text
part Hook_Copy
tool bucket
color #00AAFF
tolerance 25
fill
```
