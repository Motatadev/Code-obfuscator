# Allow Smart App Control / WDAC for Code Obfuscator (Administrator)
# Smart App Control (Controle intelligent) blocks ANY new unsigned EXE with no reputation
Write-Host "=== Smart App Control WDAC Allow ===" -ForegroundColor Cyan

$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[!] Run as Administrator !" -ForegroundColor Red
    Start-Process powershell -Verb RunAs -ArgumentList "-ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}

Write-Host "`n[1] Checking Smart App Control status..." -ForegroundColor Yellow
try {
    $sac = Get-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\CI\Policy" -Name "VerifiedAndReputablePolicyState" -ErrorAction SilentlyContinue
    if ($sac) { Write-Host "    SAC State = $($sac.VerifiedAndReputablePolicyState) (0=Off, 1=Enforced, 2=Evaluation)" }
    else { Write-Host "    SAC not found (maybe WDAC not enforced)" }
} catch { Write-Host "    Could not read registry" }

Write-Host "`n[2] Options - choose:" -ForegroundColor Yellow
Write-Host "  [A] Disable Smart App Control (requires REBOOT, easiest)"
Write-Host "  [B] Sign EXE with self-signed cert + add to Trusted Publishers (no reboot, keeps SAC on)"
Write-Host "  [C] Just move to Program Files (may still be blocked if SAC=Enforced)"
Write-Host "  [D] Exit"
$choice = Read-Host "Choice [A/B/C/D]"

if ($choice -eq "A" -or $choice -eq "a") {
    Write-Host "`nDisabling Smart App Control via registry..." -ForegroundColor Yellow
    try {
        # SAC is controlled by Smart App Control settings - GUI is required on Win11 22H2+
        # Registry method for evaluation mode off:
        Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\CI\Policy" -Name "VerifiedAndReputablePolicyState" -Value 0 -ErrorAction Stop
        Write-Host "[OK] SAC disabled (0). REBOOT REQUIRED." -ForegroundColor Green
        Write-Host "After reboot, run .\build\obfuscator.exe --help" -ForegroundColor Cyan
        $reboot = Read-Host "Reboot now? [y/N]"
        if ($reboot -eq "y") { Restart-Computer -Force }
    } catch {
        Write-Host "[!] Registry blocked. Do manually:" -ForegroundColor Red
        Write-Host "  Settings -> Privacy & security -> Windows Security -> App & browser control"
        Write-Host "  -> Smart App Control -> Off -> Reboot"
        Write-Host "Or: Settings -> Apps -> Advanced app settings -> Smart App Control"
    }
    exit
}

if ($choice -eq "B" -or $choice -eq "b") {
    Write-Host "`n[B] Creating self-signed cert and signing EXE..." -ForegroundColor Yellow
    $exe = Join-Path $PSScriptRoot "build\obfuscator.exe"
    if (-not (Test-Path $exe)) { $exe = "C:\Users\Motata\Downloads\Obfuscator\build\obfuscator.exe" }
    if (-not (Test-Path $exe)) { Write-Host "[!] Not found: $exe - build first with .\build.bat" -ForegroundColor Red; exit }

    # Create cert
    try {
        $cert = New-SelfSignedCertificate -DnsName "Motatadev" -Subject "CN=Motatadev, O=Motatadev, C=FR" -Type CodeSigningCert -CertStoreLocation Cert:\CurrentUser\My -KeyUsage DigitalSignature -NotAfter (Get-Date).AddYears(5) -ErrorAction Stop
        Write-Host "[OK] Cert created: $($cert.Thumbprint)" -ForegroundColor Green
    } catch {
        $cert = Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert | Where-Object Subject -like "*Motatadev*" | Select-Object -First 1
        if (-not $cert) { Write-Host "[!] Cert creation failed: $_" -ForegroundColor Red; exit }
        Write-Host "[OK] Using existing cert: $($cert.Thumbprint)" -ForegroundColor Green
    }

    # Add to Trusted Publishers (CurrentUser)
    try {
        $store = New-Object System.Security.Cryptography.X509Certificates.X509Store "TrustedPublisher","CurrentUser"
        $store.Open("ReadWrite"); $store.Add($cert); $store.Close()
        Write-Host "[OK] Added to TrustedPublisher (CurrentUser)" -ForegroundColor Green
        # Also Root for Smart App Control ISG trust
        $store2 = New-Object System.Security.Cryptography.X509Certificates.X509Store "Root","CurrentUser"
        $store2.Open("ReadWrite"); $store2.Add($cert); $store2.Close()
        Write-Host "[OK] Added to Root (for ISG)" -ForegroundColor Green
    } catch { Write-Host "[!] Store add failed: $_" -ForegroundColor Yellow }

    # Sign
    $signtool = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64\signtool.exe"
    if (-not (Test-Path $signtool)) { $signtool = (Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -Recurse -Filter signtool.exe -ErrorAction SilentlyContinue | Select-Object -First 1).FullName }
    if (-not $signtool) { $signtool = "signtool" }
    Write-Host "Signing with: $signtool"
    try {
        & $signtool sign /fd SHA256 /a /sha1 $cert.Thumbprint /t http://timestamp.digicert.com /v "$exe" 2>&1 | Out-String | Write-Host
        Write-Host "[OK] Signed! Try running:" -ForegroundColor Green
        Write-Host "  $exe --help"
        # Verify
        & $signtool verify /pa "$exe" 2>&1 | Out-String | Write-Host
    } catch { Write-Host "[!] Sign failed: $_" -ForegroundColor Red; Write-Host "Install Windows SDK signtool or use signtool from VS" -ForegroundColor Yellow }
    exit
}

if ($choice -eq "C" -or $choice -eq "c") {
    $dest = "C:\Program Files\CodeObfuscator"
    Write-Host "Moving to $dest (requires Admin)..."
    New-Item -ItemType Directory -Path $dest -Force | Out-Null
    Copy-Item "C:\Users\Motata\Downloads\Obfuscator\build\obfuscator.exe" -Destination "$dest\obfuscator.exe" -Force
    Add-MpPreference -ExclusionPath $dest -ErrorAction SilentlyContinue
    Write-Host "[OK] Copied. Try: & `"$dest\obfuscator.exe`" --help" -ForegroundColor Green
    Write-Host "[Note] If SAC=Enforced, still blocked - use option B (sign) or A (disable)" -ForegroundColor Yellow
    exit
}
Write-Host "Exit."
