# Allow Defender - Add exclusions for Code Obfuscator (run as Administrator)
# This prevents false positive detection (HackTool:Obfuscator is common for any obfuscator)
Write-Host "=== Code Obfuscator - Defender Allow Script ===" -ForegroundColor Cyan
Write-Host "Adding Windows Defender exclusions for legitimate obfuscator..." -ForegroundColor Yellow

$buildPath = Join-Path $PSScriptRoot "build"
$exePath = Join-Path $buildPath "obfuscator.exe"

# Check admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[!] Please run as Administrator (right-click -> Run as Administrator)" -ForegroundColor Red
    Write-Host "    Trying to elevate..."
    Start-Process powershell -Verb RunAs -ArgumentList "-ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}

try {
    # Add folder exclusion
    Add-MpPreference -ExclusionPath $buildPath -ErrorAction Stop
    Write-Host "[OK] Exclusion added: $buildPath" -ForegroundColor Green
} catch { Write-Host "[!] Failed to add path exclusion: $_" -ForegroundColor Red }

try {
    Add-MpPreference -ExclusionPath $PSScriptRoot -ErrorAction Stop
    Write-Host "[OK] Exclusion added: $PSScriptRoot" -ForegroundColor Green
} catch { Write-Host "[!] Failed: $_" -ForegroundColor Red }

try {
    if (Test-Path $exePath) {
        Add-MpPreference -ExclusionProcess "obfuscator.exe" -ErrorAction Stop
        Write-Host "[OK] Process exclusion added: obfuscator.exe" -ForegroundColor Green
    }
} catch { Write-Host "[!] Process exclusion failed: $_" -ForegroundColor Red }

Write-Host "`n[Info] To verify:" -ForegroundColor Cyan
Write-Host "  Get-MpPreference | Select-Object -ExpandProperty ExclusionPath"
Write-Host "  Get-MpPreference | Select-Object -ExpandProperty ExclusionProcess"
Write-Host "`n[Note] This is a legitimate open-source tool (MIT). If Defender still flags it," -ForegroundColor Yellow
Write-Host "  submit as false positive at: https://www.microsoft.com/wdsi/filesubmission" -ForegroundColor Yellow
Write-Host "  Choose 'I believe this file should not be detected as malware'" -ForegroundColor Yellow
Write-Host "`nDone. You can now run .\build\obfuscator.exe --help" -ForegroundColor Green
Pause
