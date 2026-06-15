param(
    [string]$BuildDir = "build-ucrt"
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$BuildPath = Join-Path $ProjectRoot $BuildDir

if (-not (Test-Path (Join-Path $BuildPath "tte.exe"))) {
    throw "tte.exe nao encontrado em $BuildPath. Compile antes de empacotar."
}

cmake --build $BuildPath --target package-local

$PackagePath = Join-Path $BuildPath "dist\TeeworldsTextureEditor-0.1.0"
Write-Host "Pacote pronto em: $PackagePath"
