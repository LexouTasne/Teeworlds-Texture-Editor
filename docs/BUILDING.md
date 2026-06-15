# Compilacao no Windows

Este projeto usa CMake e C++17.

## Melhor caminho para a 0.1

Para esta base, o caminho mais direto e:

1. MSYS2 UCRT64 para `g++`.
2. Ninja como gerador rapido.
3. CMake apontando explicitamente para os dois.

## VS Code / CMake Tools

Use o preset incluido:

```text
Windows UCRT64 Ninja
```

Ele sempre usa:

```text
build
```

Fluxo:

1. `CMake: Select Configure Preset`
2. `Windows UCRT64 Ninja`
3. `CMake: Build`
4. Abrir `build\tte.exe`

O executavel abre a janela do editor visual.

## PowerShell manual

PowerShell:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
cmake --preset windows-ucrt-ninja
cmake --build build
```

O detalhe importante e adicionar `C:\msys64\ucrt64\bin` ao `PATH` antes do CMake. Sem isso, o compilador existe, mas pode falhar porque nao acha as DLLs do proprio runtime.

## MSYS2 completo

No terminal `MSYS2 UCRT64`:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
cmake -S . -B build -G Ninja
cmake --build build
```

## Verificacao

```powershell
.\build\tte.exe
py scripts\focus_texture.py --template gameskin --part eye_normal --output out\focus-from-default.png
```

Na versao atual, o fluxo principal e a janela Windows. A ferramenta Python continua existindo como apoio para gerar previews por script.

## Pacote sem erro de DLL

Depois do build, gere um pacote local:

```powershell
cmake --build build --target package-local
```

Saida:

```text
build\dist\TeeworldsTextureEditor-0.1.0
```

Essa pasta contem:

- `tte.exe`
- DLLs necessarias do MinGW/MSYS2
- `data/`
- `scripts/`
- `requirements.txt`
- `README.md`
- `LICENSE`

Teste de dentro do pacote:

```powershell
cd build\dist\TeeworldsTextureEditor-0.1.0
.\tte.exe
```

## Copyright e icone

O executavel recebe metadados Windows com:

```text
Lex copyright 2026
```

Para trocar o icone:

1. Coloque `app.ico` na raiz do projeto, ou um `.ico` em `assets/icons/`.
2. Rode o CMake novamente.
3. Compile.

O CMake embute o icone no recurso Windows do `tte.exe`.

