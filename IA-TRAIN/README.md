# IA-TRAIN

Pasta local usada pelo agente `Genus` para guardar ensino, avaliacoes e simulacoes do editor.

O app grava dados em JSONL e esses arquivos ficam ignorados pelo Git para nao publicar pedidos do usuario por acidente.

## Fluxo do Genus

1. O usuario escreve o pedido em portugues ou ingles.
2. Clica em `Enviar`.
3. O Genus tenta executar usando treino salvo, IA externa e parser PT/EN.
4. Se nao souber fazer com seguranca, ele abre o modo de ensino com uma receita `agent-*` editavel.
5. O usuario ajusta os agentes e clica em `Salvar ensino`.
6. Depois de executar, o usuario avalia com `nota 8 obs acertou a cor, errou a sombra` ou `rate 8 comment good target`.

## Arquivos locais

- `genus-training.jsonl`: receitas ensinadas pelo usuario.
- `genus-ratings.jsonl`: notas de 0 a 10 e observacoes para acoes executadas.
- `genus-simulations.jsonl`: simulacoes locais usadas como exemplos fracos de treino.

## Agentes aceitos

```text
agent-select-template gameskin
agent-select-part body
agent-auto-select-part weapon
agent-select-current-part
agent-select-all
agent-select-current-layer
agent-layer-base
agent-focus-part
agent-select-ferramenta pencil
agent-select-ferramenta brush
agent-select-ferramenta eraser
agent-select-ferramenta bucket
agent-select-ferramenta select
agent-select-ferramenta crop
agent-fill
agent-fill-selection
agent-pintar
agent-paint-selection
agent-crop-layer
clear
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
nota 8 obs fez certo a selecao, mas errou a cor
```

## Exemplos PT/EN

### Preencher arma/weapon com azul

```text
agent-auto-select-part weapon
agent-select-ferramenta bucket
color #267EFF
agent-fill
agent-rate
```

### Focar uma part

```text
agent-select-part Effect_9
agent-focus-part
agent-rate
```

### Recolorir amarelo para azul

```text
agent-select-current-part
agent-select-ferramenta pencil
source-color yellow
color blue
confidence 60
agent-pintar
agent-rate
```

Pedidos que o parser entende:

```text
preenche a part weapon com azul
foca para mim o effect 9
fill the weapon part with blue
pinte tudo que e amarelo para azul com confianca 60
paint every yellow part blue with confidence 60
```

### Recortar uma part para camada movel

```text
agent-select-current-part
agent-select-ferramenta crop
agent-crop-layer
agent-rate
```

Depois do recorte, use a ferramenta `Selecionar` e arraste a camada. Ao salvar PNG, a camada e mesclada automaticamente.
