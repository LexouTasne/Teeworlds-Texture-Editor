# Genus patch notes

Mudanças aplicadas nesta versão:

- Genus agora usa um único botão `Enviar`.
- Parser bilíngue PT/EN para pedidos simples: cor, ferramenta, preencher, recortar, apagar, selecionar e recolorir.
- Fallback de ensino por receita `agent-*` quando o Genus não souber executar.
- `Salvar ensino` grava pedidos em `IA-TRAIN/genus-training.jsonl`.
- `agent-rate` aceita avaliação por texto: `nota 8`, `rate 8`, etc.
- Nova ferramenta `Recorte` na barra lateral.
- Recorte transforma a part selecionada em camada movível.
- Ferramenta `Selecionar` move a camada recortada e não abre opções com botão direito.
- Salvar PNG mescla automaticamente a camada recortada.
- `agent-pintar` recolore pixels parecidos com `source-color` usando `confidence`.
- `agent-fill` preenche a região selecionada.
- 5.000 simulações locais em `IA-TRAIN/genus-simulations.jsonl`.
- Preview mais leve: remove `UpdateWindow` do mouse move e muda o timer ativo para 16 ms.
- UI do painel do Genus ajustada para fluxo mais limpo.

Exemplo de pedido:

```text
pinte tudo que é amarelo para azul com confiança 60
```

Receita equivalente:

```text
agent-select-part
agent-select-ferramenta pencil
source-color yellow
color blue
confidence 60
agent-pintar
agent-rate
```
