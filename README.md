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
- Imagens padrao em `data/defaults/` para usar mesmo sem carregar PNG externo.
- Ferramenta Python para criar preview com foco em uma parte da textura.
- Copyright e metadados Windows no executavel: `Lex copyright 2026`.
- Icone customizavel por `.ico` no build.
- Copia automatica das DLLs do MinGW ao lado do `.exe`.
- Estrutura pronta para evoluir para editor visual.
- Roadmap do projeto.

## Compilar no Windows

### Caminho recomendado para este projeto: MSYS2 UCRT64 + Ninja

No PowerShell:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
$ninja = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe\ninja.exe"
cmake -S . -B build-ucrt -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja" -DCMAKE_CXX_COMPILER="C:\msys64\ucrt64\bin\g++.exe"
cmake --build build-ucrt
```

Executar:

```powershell
.\build-ucrt\tte.exe list
```

Criar pacote local com `.exe`, DLLs, templates e scripts:

```powershell
cmake --build build-ucrt --target package-local
.\build-ucrt\dist\TeeworldsTextureEditor-0.1.0\tte.exe about
```

O pacote fica em:

```text
build-ucrt\dist\TeeworldsTextureEditor-0.1.0
```

### Alternativa: Visual Studio Build Tools

Instale o workload C++ do Visual Studio Build Tools, abra o "Developer PowerShell for VS" e rode:

```powershell
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64
cmake --build build-msvc --config Release
.\build-msvc\Release\tte.exe list
```

### Alternativa: MSYS2 sem Ninja

Dentro do terminal `MSYS2 UCRT64`, instale as ferramentas:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

Depois compile:

```bash
cmake -S . -B build-ucrt -G Ninja
cmake --build build-ucrt
```

## Comandos do editor

Ver informacoes do client:

```powershell
.\build-ucrt\tte.exe about
```

Listar templates:

```powershell
.\build-ucrt\tte.exe list
```

Listar partes de um template:

```powershell
.\build-ucrt\tte.exe parts gameskin
```

Gerar preview focado em uma parte:

```powershell
py scripts\focus_texture.py --template gameskin --part eye_normal --input caminho\game.png --output preview.png
```

Se nao passar `--input`, o editor usa o template padrao:

```powershell
py scripts\focus_texture.py --template gameskin --part eye_normal --output preview.png
```

Ou pelo executavel C++:

```powershell
.\build-ucrt\tte.exe focus --template gameskin --part eye_normal --output preview.png
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
- `data/defaults/`: imagens padrao usadas quando o usuario nao carrega uma textura.
- `assets/icons/`: icones `.ico` para embutir no executavel.
- `cmake/`: recursos de build, metadata Windows e empacotamento.
- `docs/`: roadmap e notas tecnicas.

## Icone customizado

Para trocar o icone do `.exe`, coloque um arquivo `app.ico` na raiz do projeto ou qualquer `.ico` em `assets/icons/`.

Depois rode o CMake novamente:

```powershell
cmake -S . -B build-ucrt -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja" -DCMAKE_CXX_COMPILER="C:\msys64\ucrt64\bin\g++.exe"
cmake --build build-ucrt
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
