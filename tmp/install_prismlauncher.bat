@echo off
chcp 65001 >nul
setlocal EnableExtensions EnableDelayedExpansion

set "URL=https://github.com/ElyPrismLauncher/Launcher/releases/download/11.0.2/PineconeMC-Windows-MinGW-w64-Portable-11.0.2.zip"
set "APP=ElyPrismLauncher"
set "APP_DIR=%APPDATA%\%APP%"

:: ПРОВЕРКА СУЩЕСТВОВАНИЯ TAR
where tar >nul 2>&1 || call :fail "Bat cant work on this PC. URL: %URL%"

:: ЗАКРЫТИЕ ЛАУНЧЕРА
taskkill /IM %APP%.exe /F >nul 2>&1
tasklist | find /I "%APP%.exe" >nul && call :fail "Close %APP% and launch again"

:: СКАЧИВАНИЕ ЧЕРЕЗ CURL, ИНАЧЕ - POWERSHELL
echo Downloading %APP%...
curl -L -# -o %APP%.zip "%URL%" || (
    powershell -nop -c "[Net.ServicePointManager]::SecurityProtocol=3072;(New-Object Net.WebClient).DownloadFile('%URL%','%APP%.zip')" || call :fail "Download error"
)

:: РАСПАКОВКА
mkdir "%APP_DIR%" 2>nul
tar -xf %APP%.zip -C "%APP_DIR%"
del %APP%.zip

:: СОЗДАНИЕ КОНФИГА
> "%APP_DIR%\%APP%.cfg" (
    echo [General]
    echo ConfigVersion=1.3
    echo ApplicationTheme=dark
    echo IconTheme=breeze_dark
    echo StatusBarVisible=true
    echo ToolbarsLocked=true
    echo AutomaticJavaDownload=true
    echo AutomaticJavaSwitch=true
    echo IgnoreJavaCompatibility=true
    echo IgnoreJavaWizard=true
    echo UseOptimizedJvmArgs=true
    echo GarbageCollectorPreset=ZGC
    echo MaxMemAlloc=3072
    echo MinMemAlloc=3072
    echo UserAskedAboutAutomaticJavaDownload=true
    echo MainWindowGeometry=AdnQywADAAAAAAITAAAA/QAABWwAAAOYAAACEwAAARwAAAVsAAADmAAAAAAAAAAAB4AAAAITAAABHAAABWwAAAOY
    echo MainWindowState="AAAA/wAAAAD9AAAAAAAAA1oAAAJAAAAABAAAAAQAAAAIAAAACPwAAAADAAAAAQAAAAEAAAAeAGkAbgBzAHQAYQBuAGMAZQBUAG8AbwBsAEIAYQByAgAAAAD/////AAAAAAAAAAAAAAACAAAAAQAAABYAbQBhAGkAbgBUAG8AbwBsAEIAYQByAQAAAAD/////AAAAAAAAAAAAAAADAAAAAQAAABYAbgBlAHcAcwBUAG8AbwBsAEIAYQByAAAAAAD/////AAAAAAAAAAA="
    echo WideBarVisibility_instanceToolBar="MTExMTExMTExLE1URjFQcWRVWjg0RkxHQlV5a0dNTDU3RW05OD0="
    echo UseSystemLocale=true
    echo Language=ru
)

:: РЕГИСТРАЦИЯ ПРОТОКОЛОВ ДЛЯ ФИЧИ ИМПОРТА ПО ССЫЛКЕ
reg add "HKCU\Software\Classes\curseforge" /v "URL Protocol" /t REG_SZ /d "" /f
reg add "HKCU\Software\Classes\curseforge\shell\open\command" /ve /t REG_SZ /d "\"%APP_DIR%\%APP%.exe\" \"%%1\"" /f

reg add "HKCU\Software\Classes\%APP%" /v "URL Protocol" /t REG_SZ /d "" /f
reg add "HKCU\Software\Classes\%APP%\shell\open\command" /ve /t REG_SZ /d "\"%APP_DIR%\%APP%.exe\" \"%%1\"" /f

reg add "HKCU\Software\Classes\prismlauncher" /v "URL Protocol" /t REG_SZ /d "" /f
reg add "HKCU\Software\Classes\prismlauncher\shell\open\command" /ve /t REG_SZ /d "\"%APP_DIR%\%APP%.exe\" \"%%1\"" /f

:: СОЗДАНИЕ ЯРЛЫКА
set "PRISM_PATH=%APP_DIR%\%APP%.exe"
set "SHORTCUT=%USERPROFILE%\Desktop\PrismLauncher.lnk"

powershell -nop -c "$s=(New-Object -ComObject WScript.Shell).CreateShortcut('%SHORTCUT%');$s.TargetPath='%PRISM_PATH%';$s.WorkingDirectory=(Split-Path '%PRISM_PATH%');$s.IconLocation='%PRISM_PATH%';$s.Save()"

start "" "%PRISM_PATH%"
exit /b

:fail
echo [ERROR] %~1
pause
exit /b 1