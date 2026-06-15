# Icones do executavel

Coloque um arquivo `.ico` nesta pasta para o CMake embutir no `tte.exe`.

Regras da 0.1:

- `app.ico` na raiz do projeto tem prioridade.
- Se nao existir `app.ico`, o CMake usa o primeiro `.ico` encontrado em `assets/icons/`.
- Depois de trocar o icone, rode o CMake de novo para regenerar o recurso do Windows.

Exemplo:

```powershell
cmake -S . -B build-ucrt -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja" -DCMAKE_CXX_COMPILER="C:\msys64\ucrt64\bin\g++.exe"
cmake --build build-ucrt
```
