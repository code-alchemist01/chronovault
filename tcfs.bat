@echo off
echo ========================================
echo    Time Capsule File System (TCFS)
echo ========================================
echo.

if "%1"=="" (
    echo Kullanim:
    echo   tcfs init --owner "email@example.com"  - TCFS store'u baslatir
    echo   tcfs lock [dosya] --unlock-at [tarih]  - Dosyayi zaman kapsulune kilitler
    echo   tcfs unlock [dosya]                    - Zaman kapsulu dosyasini acar
    echo   tcfs status [dosya]                    - Dosya durumunu kontrol eder
    echo   tcfs list                             - Tum zaman kapsulu dosyalarini listeler
    echo.
    echo Detayli Ornekler:
    echo   tcfs init --owner "excalibur@example.com"
    echo   tcfs lock test.txt --unlock-at "2024-12-28T16:00:00Z" --label "Test"
    echo   tcfs list
    echo.
    echo Tarih Formati: RFC3339 (YYYY-MM-DDTHH:MM:SSZ)
    echo Ornek Tarih: 2024-12-28T15:30:00Z
    echo.
    .\build\src\cli\Release\tcfs.exe --help
) else (
    echo Calistiriliyor: tcfs %*
    echo.
    .\build\src\cli\Release\tcfs.exe %*
    if errorlevel 1 (
        echo.
        echo HATA: Komut basarisiz oldu. Yukaridaki hata mesajini kontrol edin.
    ) else (
        echo.
        echo Islem basarili!
    )
)

echo.
pause