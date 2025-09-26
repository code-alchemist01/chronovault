param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Arguments
)

# TCFS executable path
$tcfsPath = Join-Path $PSScriptRoot "build\src\cli\Release\tcfs.exe"

# Check if TCFS executable exists
if (-not (Test-Path $tcfsPath)) {
    Write-Host "HATA: TCFS executable bulunamadi: $tcfsPath" -ForegroundColor Red
    Write-Host "Lutfen once projeyi derleyin (cmake --build . --target tcfs_cli --config Release)" -ForegroundColor Yellow
    exit 1
}

# Display header
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "    Time Capsule File System (TCFS)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# If no arguments provided, show help
if (-not $Arguments -or $Arguments.Count -eq 0) {
    Write-Host "Kullanim:" -ForegroundColor Green
    Write-Host "  .\tcfs.ps1 init                    - TCFS store'u baslatir" -ForegroundColor White
    Write-Host "  .\tcfs.ps1 lock [dosya]           - Dosyayi zaman kapsulune kilitler" -ForegroundColor White
    Write-Host "  .\tcfs.ps1 unlock [dosya]         - Zaman kapsulu dosyasini acar" -ForegroundColor White
    Write-Host "  .\tcfs.ps1 status [dosya]         - Dosya durumunu kontrol eder" -ForegroundColor White
    Write-Host "  .\tcfs.ps1 list                   - Tum zaman kapsulu dosyalarini listeler" -ForegroundColor White
    Write-Host ""
    Write-Host "Ornek:" -ForegroundColor Yellow
    Write-Host "  .\tcfs.ps1 init" -ForegroundColor Gray
    Write-Host "  .\tcfs.ps1 lock `"C:\Users\excalibur\Desktop\test.txt`"" -ForegroundColor Gray
    Write-Host ""
    
    # Show detailed help
    & $tcfsPath --help
} else {
    # Execute TCFS with provided arguments
    Write-Host "Calistiriliyor: tcfs $($Arguments -join ' ')" -ForegroundColor Yellow
    Write-Host ""
    & $tcfsPath $Arguments
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "Islem basarili!" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "HATA: Komut basarisiz oldu. Yukaridaki hata mesajini kontrol edin." -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Devam etmek icin bir tusa basin..." -ForegroundColor DarkGray
Read-Host