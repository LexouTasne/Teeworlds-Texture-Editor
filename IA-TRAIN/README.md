# IA-TRAIN

Pasta local usada pelo agente `Genus` para guardar ensino, avaliações e simulações do editor.

O app grava dados em JSONL e esses arquivos ficam ignorados pelo Git para não publicar pedidos do usuário por acidente.

## Fluxo novo do Genus

Agora o Genus trabalha com um fluxo único:

1. O usuário escreve o pedido em português ou inglês.
2. Clica em `Enviar`.
3. O Genus tenta executar usando treino salvo ou parser PT/EN.
4. Se não souber fazer, ele abre o modo de ensino com uma receita `agent-*` editável.
5. O usuário ajusta os agentes e clica em `Salvar ensino`.
6. Depois de executar, o usuário pode avaliar com `nota 8`, `rate 8`, etc.

## Arquivos locais

- `genus-training.jsonl`: receitas ensinadas pelo usuário.
- `genus-ratings.jsonl`: notas de 0 a 10 para ações executadas.
- `genus-simulations.jsonl`: simulações geradas localmente para treino/teste inicial.

## Agentes aceitos

```text
agent-select-template gameskin
agent-select-part
agent-select-part body
agent-select-ferramenta pencil
agent-select-ferramenta brush
agent-select-ferramenta eraser
agent-select-ferramenta bucket
agent-select-ferramenta select
agent-select-ferramenta crop
agent-fill
agent-pintar
agent-crop-layer
agent-rate
```

## Comandos auxiliares

```text
whole
part Hook_Copy
tool pencil
tool eraser
tool bucket
tool crop
size 16
opacity 80
confidence 60
tolerance 25
hardness 100
shape round
shape square
color #267EFF
source-color yellow
fill
clear
crop
```

## Exemplos PT/EN

### Recolorir amarelo para azul

```text
agent-select-part
agent-select-ferramenta pencil
source-color yellow
color blue
confidence 60
agent-pintar
agent-rate
```

Pedidos que o parser entende:

```text
pinte tudo que é amarelo para azul com confiança 60
paint every yellow part blue with confidence 60
```

### Recortar uma part para camada movível

```text
agent-select-part
agent-select-ferramenta crop
agent-crop-layer
agent-rate
```

Depois do recorte, use a ferramenta `Selecionar` e arraste a camada. Ao salvar PNG, a camada é mesclada automaticamente.
