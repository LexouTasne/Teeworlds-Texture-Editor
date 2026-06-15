# Compilacao no Windows

Este projeto usa CMake e C++17.

## Melhor caminho para a 0.1

Para esta base, o caminho mais direto e:

1. MSYS2 UCRT64 para `g++`.
2. Ninja como gerador rapido.
3. CMake apontando explicitamente para os dois.

PowerShell:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
$ninja = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe\ninja.exe"
cmake -S . -B build-ucrt -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja" -DCMAKE_CXX_COMPILER="C:\msys64\ucrt64\bin\g++.exe"
cmake --build build-ucrt
```

O detalhe importante e adicionar `C:\msys64\ucrt64\bin` ao `PATH` antes do CMake. Sem isso, o compilador existe, mas pode falhar porque nao acha as DLLs do proprio runtime.

## Visual Studio Build Tools

Tambem funciona bem para usuarios que preferem MSVC:

```powershell
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64
cmake --build build-msvc --config Release
```

Esse caminho exige abrir o terminal de desenvolvedor do Visual Studio ou instalar o Build Tools com suporte a C++.

## MSYS2 completo

No terminal `MSYS2 UCRT64`:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
cmake -S . -B build-ucrt -G Ninja
cmake --build build-ucrt
```

## Verificacao

```powershell
.\build-ucrt\tte.exe list
.\build-ucrt\tte.exe parts gameskin
.\build-ucrt\tte.exe focus --template gameskin --part eye_normal --output out\focus-from-default.png
```

## Pacote sem erro de DLL

Depois do build, gere um pacote local:

```powershell
cmake --build build-ucrt --target package-local
```

Saida:

```text
build-ucrt\dist\TeeworldsTextureEditor-0.1.0
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
cd build-ucrt\dist\TeeworldsTextureEditor-0.1.0
.\tte.exe about
.\tte.exe list
.\tte.exe focus --template particles --part explosion --output package-particles-focus.png
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
