# Teeworlds Texture Editor

Editor de texturas focado 100% no Teeworlds e derivados, criado para editar `game.png`, skins, HUDs e outros assets do jogo com templates prontos.

Versao atual: `0.1.0`

## Ideia

O objetivo do projeto e ser uma IDE/editor para artistas e modders de Teeworlds:

- abrir templates oficiais de textura;
- focar partes especificas, como olho, arma, cursor, coracao do HUD ou sprite do tee;
- escurecer o resto da imagem para facilitar edicao;
- dar zoom visual na regiao escolhida;
- futuramente incluir uma IA integrada para sugerir ajustes, limpar bordas, gerar variacoes e explicar o template.

Em resumo: um editor para parar de procurar sprite no pixel errado como se fosse caca ao tesouro em PNG.

## O que ja existe na 0.1

- CLI em C++ com comandos iniciais.
- Banco de templates em JSON.
- Ferramenta Python para criar preview com foco em uma parte da textura.
- Estrutura pronta para evoluir para editor visual.
- Roadmap do projeto.

## Comandos

Listar templates:

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\tte.exe list
```

Listar partes de um template:

```powershell
.\build\Debug\tte.exe parts gameskin
```

Gerar preview focado em uma parte:

```powershell
py scripts\focus_texture.py --template gameskin --part eye_normal --input caminho\game.png --output preview.png
```

Ou pelo executavel C++:

```powershell
.\build\Debug\tte.exe focus --template gameskin --part eye_normal --input caminho\game.png --output preview.png
```

## Dependencias

- CMake 3.20+
- Compilador C++17
- Python 3.10+
- Pillow para processamento de imagem

Instalar dependencias Python:

```powershell
py -m pip install -r requirements.txt
```

## Arquitetura planejada

- `src/`: nucleo C++ do editor e comandos locais.
- `scripts/`: ferramentas Python para imagem e IA.
- `data/templates/`: templates e regioes das texturas.
- `docs/`: roadmap e notas tecnicas.

## Futuro da IA

A ideia e ter uma IA dentro do editor para:

- explicar o que cada parte do template faz;
- sugerir correcoes de contraste, contorno e alinhamento;
- gerar variantes de skins/HUDs;
- ajudar o usuario a encontrar regioes dentro do template;
- automatizar tarefas repetitivas sem tirar o controle do artista.

## Licenca

MIT.
