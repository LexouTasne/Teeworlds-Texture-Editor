# Teeworlds Texture Editor

Editor visual de texturas focado 100% no Teeworlds e derivados, criado para editar `game.png`, skins, HUDs e outros assets do jogo com templates prontos.

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

- App Windows em C++ com interface visual.
- Canvas para visualizar PNGs e focar partes especificas.
- Lista de templates e partes editaveis.
- Modo-dev para configurar `id`, `label`, `x`, `y`, `w` e `h`.
- Banco de templates em JSON.
- Imagens padrao em `data/defaults/` para usar mesmo sem carregar PNG externo.
- Render pixel-perfect: sem esticar textura para preencher tela.
- Ferramenta Python para criar preview com foco em uma parte da textura.
- Copyright e metadados Windows no executavel: `Lex copyright 2026`.
- Icone customizavel por `.ico` no build.
- Copia automatica das DLLs do MinGW ao lado do `.exe`.
- Estrutura pronta para evoluir para editor visual.
- Roadmap do projeto.

## Compilar no Windows

### VS Code / CMake Tools

Este repo ja vem com `CMakePresets.json`. No VS Code:

1. Abra a Command Palette.
2. Rode `CMake: Select Configure Preset`.
3. Escolha `Windows UCRT64 Ninja`.
4. Rode `CMake: Build`.

O executavel visual fica em:

```text
build\tte.exe
```

Ao abrir `tte.exe`, ele abre a janela do editor. Nada de terminal piscando e fugindo como ninja de bug.

### PowerShell manual

No PowerShell:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
cmake --preset windows-ucrt-ninja
cmake --build build
```

Executar:

```powershell
.\build\tte.exe
```

Criar pacote local com `.exe`, DLLs, templates e scripts:

```powershell
cmake --build build --target package-local
.\build\dist\TeeworldsTextureEditor-0.1.0\tte.exe
```

O pacote fica em:

```text
build\dist\TeeworldsTextureEditor-0.1.0
```

### Alternativa: MSYS2 sem Ninja

Dentro do terminal `MSYS2 UCRT64`, instale as ferramentas:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

Depois compile:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Usar o editor

Ao abrir `build\tte.exe`:

- escolha o template na lista da esquerda;
- escolha uma parte, como `eye_normal`, `gun`, `health` ou `explosion`;
- clique em `Abrir PNG` para carregar uma textura sua;
- use `Selecionar`, `Lapis` ou `Borracha` na barra superior;
- cada ferramenta tem um preview visual no proprio botao;
- use `-`, `+` e `Fit` para controlar o zoom;
- use `Ctrl + roda do mouse` para zoom rapido;
- use botao direito e arraste para mover a textura quando estiver com zoom;
- clique em `Template padrao` para voltar ao template interno;
- use o `Modo-dev` na direita para ajustar IDs e coordenadas;
- use `Pincel` para controlar o tamanho do lapis/borracha;
- clique em `Aplicar parte` para atualizar a selecao;
- clique em `Salvar JSON` para gravar em `data/templates/teeworlds_textures.json`.
- clique em `Salvar PNG` para exportar a textura editada.

O foco visual escurece o resto da textura e amplia a parte selecionada no painel de preview.

A interface usa repaint com buffer e atualizacao por regiao suja para evitar aquele efeito de imagem redesenhando de cima para baixo. Clicar no mesmo template tambem nao recarrega a textura inteira sem necessidade.

Nos campos de texto do modo-dev:

- `Ctrl + A` seleciona tudo;
- `Ctrl + C`, `Ctrl + V` e `Ctrl + X` usam o comportamento nativo do Windows;
- `Ctrl + Backspace` apaga a palavra anterior.

Importante: o editor usa os PNGs reais que estiverem em `data/defaults/`. Ele nao redesenha essas texturas em runtime. Para usar as imagens oficiais/exatas, substitua os arquivos desta pasta mantendo os nomes:

- `gameskin.png`
- `skin.png`
- `hud.png`
- `emoticons.png`
- `particles.png`
- `ddrace_logo.png`

Os defaults incluidos na 0.1 foram baixados do repositório oficial do DDNet:

- https://github.com/ddnet/ddnet/blob/master/data/game.png
- https://github.com/ddnet/ddnet/blob/master/data/emoticons.png
- https://github.com/ddnet/ddnet/blob/master/data/hud.png
- https://github.com/ddnet/ddnet/blob/master/data/particles.png
- https://github.com/ddnet/ddnet/blob/master/data/gui_logo.png
- https://github.com/ddnet/ddnet/blob/master/data/skins/default.png

## Ferramenta Python opcional

Gerar preview focado em uma parte:

```powershell
py scripts\focus_texture.py --template gameskin --part eye_normal --input caminho\game.png --output preview.png
```

Se nao passar `--input`, o editor usa o template padrao:

```powershell
py scripts\focus_texture.py --template gameskin --part eye_normal --output preview.png
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

- `src/`: nucleo C++ do editor visual.
- `scripts/`: ferramentas Python para imagem e IA.
- `data/templates/`: templates e regioes das texturas.
- `data/defaults/`: imagens padrao usadas quando o usuario nao carrega uma textura.
- `assets/icons/`: icones `.ico` para embutir no executavel.
- `cmake/`: recursos de build, metadata Windows e empacotamento.
- `docs/`: roadmap e notas tecnicas.

## Icone customizado

Para trocar o icone do `.exe`, coloque um arquivo `app.ico` na raiz do projeto ou qualquer `.ico` em `assets/icons/`.

Depois rode o CMake novamente:

```powershell
cmake --preset windows-ucrt-ninja
cmake --build build
```

Importante: o Windows exige que o icone seja embutido na hora do build. O executavel nao altera o proprio icone em runtime porque isso seria editar o proprio arquivo `.exe` aberto, uma receita boa para criar erro e cafe frio.

## DLLs do Windows

Quando compilado com MSYS2 UCRT64/GCC, o CMake copia automaticamente:

- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`
- `libwinpthread-1.dll`

Elas ficam ao lado de `tte.exe`, entao o usuario nao precisa instalar o MSYS2 para abrir o client.

## Futuro da IA

A ideia e ter uma IA dentro do editor para:

- explicar o que cada parte do template faz;
- sugerir correcoes de contraste, contorno e alinhamento;
- gerar variantes de skins/HUDs;
- ajudar o usuario a encontrar regioes dentro do template;
- automatizar tarefas repetitivas sem tirar o controle do artista.

## Licenca

MIT.

