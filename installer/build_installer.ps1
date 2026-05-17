<#
.SYNOPSIS
    Build FFmAdobe MSI installer

.DESCRIPTION
    1. Builds FFmpegFreeUI (VB.NET, self-contained single-file)
    2. Builds FFmpegExporter.prm (C++ CMake)
    3. Stages outputs
    4. Compiles WiX MSI with wix CLI

.EXAMPLE
    .\build_installer.ps1
    .\build_installer.ps1 -SkipBuild   # Use existing staged files
#>
param(
    [switch]$SkipBuild,
    [string]$Version = "1.0.0"
)

$ErrorActionPreference = "Stop"
$Root   = Split-Path $PSScriptRoot -Parent
$Inst   = $PSScriptRoot
$Msbuild = "D:\Program Files\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
$Cmake   = "D:\Program Files\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

Write-Host "=== FFmAdobe Installer Build ===" -ForegroundColor Cyan

if (-not $SkipBuild) {
    # ---- 1. Build FFmpegFreeUI (self-contained single-file publish) ----
    Write-Host "`n[1/3] Building FFmpegFreeUI..." -ForegroundColor Yellow
    $ffuiProj = "$Root\FFmpegFreeUI\FFmpegFreeUI\FFmpegFreeUI.vbproj"
    $ffuiOut  = "$Inst\staging\FFmpegFreeUI"
    dotnet publish $ffuiProj `
        -c Release -r win-x64 --self-contained true `
        -p:PublishSingleFile=true `
        -p:PublishReadyToRun=false `
        -o $ffuiOut
    # Remove PDB from staging
    Remove-Item "$ffuiOut\*.pdb" -Force -ErrorAction SilentlyContinue

    # ---- 2. Build FFmpegExporter plugin ----
    Write-Host "`n[2/3] Building FFmpegExporter plugin..." -ForegroundColor Yellow
    $buildDir = "$Root\cmake-build"
    if (-not (Test-Path $buildDir)) {
        & $Cmake -B $buildDir -G "Visual Studio 18 2026" `
            -DADOBE_PP_SDK="E:/Code/Premiere Pro 26.0 C++ SDK"
    }
    Remove-Item "$buildDir\Release\FFmpegExporter.pdb" -Force -ErrorAction SilentlyContinue
    & $Cmake --build $buildDir --config Release

    $pluginOut = "$Inst\staging\Plugin"
    New-Item -ItemType Directory -Force $pluginOut | Out-Null
    Copy-Item "$buildDir\Release\FFmpegExporter.prm" $pluginOut -Force
}

# ---- 3. Build MSI ----
Write-Host "`n[3/3] Building MSI installer..." -ForegroundColor Yellow
$outDir = "$Root\installer\output"
New-Item -ItemType Directory -Force $outDir | Out-Null

Push-Location $Inst
try {
    wix build FFmAdobe.wxs `
        -ext WixToolset.UI.wixext `
        -d Version=$Version `
        -o "$outDir\FFmAdobe-$Version-x64.msi" `
        -arch x64
    Write-Host "`n=== BUILD SUCCESS ===" -ForegroundColor Green
    Write-Host "MSI: $outDir\FFmAdobe-$Version-x64.msi"
} finally {
    Pop-Location
}
