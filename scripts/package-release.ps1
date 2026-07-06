# Package urFileManager for all platforms
# Run from project root: .\scripts\package-release.ps1

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$DesktopDir = Join-Path $Root "frontend-desktop"
$PublicDir = Join-Path $Root "frontend-web\public"
$ConfigFile = Join-Path $Root "config.json"

New-Item -ItemType Directory -Path $PublicDir -Force | Out-Null

Write-Host "=== urFileManager Release Packager ===" -ForegroundColor Cyan

# ── Windows: build EXE and package into ZIP ────────────────────────────
Write-Host "`n[Windows] Packaging..." -ForegroundColor Yellow
$ExePath = Join-Path $DesktopDir "ufmgr.exe"
if (-not (Test-Path $ExePath)) {
    Write-Host "  Building ufmgr.exe..." -ForegroundColor Yellow
    Push-Location $DesktopDir
    & cmd /c "build.bat"
    Pop-Location
    if (-not (Test-Path $ExePath)) {
        Write-Error "  Build failed - ufmgr.exe still missing."
    }
}

$WinStage = Join-Path $env:TEMP "urfm-win-stage-$(Get-Random)"
New-Item -ItemType Directory -Path $WinStage -Force | Out-Null
foreach ($f in @("ufmgr.exe", "run.bat", "ufmgr.bat")) {
    Copy-Item (Join-Path $DesktopDir $f) $WinStage
}
Copy-Item $ConfigFile $WinStage
Copy-Item (Join-Path $DesktopDir "RELEASE_README.md") (Join-Path $WinStage "README.txt")

$WinZip = Join-Path $PublicDir "urfm-windows.zip"
if (Test-Path $WinZip) { Remove-Item $WinZip -Force }
Compress-Archive -Path (Join-Path $WinStage "*") -DestinationPath $WinZip -CompressionLevel Optimal
Remove-Item $WinStage -Recurse -Force
$SizeMb = [math]::Round((Get-Item $WinZip).Length / 1MB, 2)
Write-Host "  Created: urfm-windows.zip - $SizeMb MB" -ForegroundColor Green

# ── Linux: source tarball with build.sh ────────────────────────────────
Write-Host "`n[Linux] Packaging source tarball..." -ForegroundColor Yellow
$LinuxStage = Join-Path $env:TEMP "urfm-linux-stage-$(Get-Random)"
New-Item -ItemType Directory -Path $LinuxStage -Force | Out-Null
foreach ($f in @("core.cpp", "core.h", "gui_fltk.cpp", "build.sh")) {
    Copy-Item (Join-Path $DesktopDir $f) $LinuxStage
}
Copy-Item $ConfigFile $LinuxStage
Copy-Item (Join-Path $DesktopDir "RELEASE_README.md") (Join-Path $LinuxStage "README.txt")

$LinuxTar = Join-Path $PublicDir "urfm-linux.tar.gz"
Push-Location $LinuxStage
tar -czf $LinuxTar *
Pop-Location
Remove-Item $LinuxStage -Recurse -Force
Write-Host "  Created: urfm-linux.tar.gz" -ForegroundColor Green

# ── Manifest for the website ───────────────────────────────────────────
$Manifest = @{
    version = "1.0.0"
    generated = (Get-Date -Format "yyyy-MM-ddTHH:mm:ssZ")
    files = @(
        @{ name = "urfm-windows.zip"; platform = "windows"; size = (Get-Item (Join-Path $PublicDir "urfm-windows.zip")).Length }
        @{ name = "urfm-linux.tar.gz"; platform = "linux"; size = (Get-Item (Join-Path $PublicDir "urfm-linux.tar.gz")).Length }
    )
}
$Manifest | ConvertTo-Json -Depth 3 | Set-Content (Join-Path $PublicDir "downloads.json") -Encoding UTF8
Write-Host "  Created: downloads.json" -ForegroundColor Green

Write-Host "`n=== All platform packages created ===" -ForegroundColor Cyan
