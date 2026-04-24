package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

const (
	url = "https://github.com/ElyPrismLauncher/Launcher/releases/download/11.0.2/PineconeMC-Windows-MinGW-w64-Portable-11.0.2.zip"
	app = "ElyPrismLauncher"
)

func main() {
	exec.Command("chcp", "65001").Run()

	appDir := filepath.Join(os.Getenv("APPDATA"), app)

	// Проверка наличия tar
	if _, err := exec.LookPath("tar"); err != nil {
		fail("Bat не может работать на этом ПК.\nURL: " + url)
	}

	// Закрыть процесс, если запущен
	exec.Command("taskkill", "/IM", app+".exe", "/F").Run()
	if procRunning(app + ".exe") {
		fail("Закройте " + app + " и запустите снова")
	}

	// Скачивание
	fmt.Println("Загрузка", app+"...")
	zipPath := app + ".zip"
	if err := downloadFile(url, zipPath); err != nil {
		fail("Ошибка загрузки: " + err.Error())
	}

	// Распаковка
	fmt.Println("Распаковка...")
	os.MkdirAll(appDir, 0755)
	if err := extractZip(zipPath, appDir); err != nil {
		fail("Ошибка распаковки: " + err.Error())
	}
	os.Remove(zipPath)

	// Конфиг
	cfgPath := filepath.Join(appDir, app+".cfg")
	if err := writeConfig(cfgPath); err != nil {
		fail("Ошибка создания конфига: " + err.Error())
	}

	// Регистрация протоколов
	registerProtocols(appDir)

	// Ярлык
	createShortcut(appDir)

	// Запуск
	exePath := filepath.Join(appDir, app+".exe")
	exec.Command(exePath).Start()
	fmt.Println("Готово!")
}

func fail(msg string) {
	fmt.Println("[ОШИБКА]", msg)
	fmt.Print("Нажмите Enter...")
	fmt.Scanln()
	os.Exit(1)
}

func procRunning(name string) bool {
	out, err := exec.Command("tasklist", "/FI", "IMAGENAME eq "+name).Output()
	if err != nil {
		return false
	}
	return strings.Contains(string(out), name)
}

// downloadFile использует curl, а при его отсутствии — PowerShell
func downloadFile(url, dest string) error {
	// Пробуем curl
	if _, err := exec.LookPath("curl"); err == nil {
		cmd := exec.Command("curl", "-L", "-#", "-o", dest, url)
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		return cmd.Run()
	}
	// Fallback на PowerShell
	psCmd := `[Net.ServicePointManager]::SecurityProtocol=3072;(New-Object Net.WebClient).DownloadFile('` + url + `','` + dest + `')`
	cmd := exec.Command("powershell", "-nop", "-c", psCmd)
	return cmd.Run()
}

// extractZip использует tar (работает с zip, т.к. tar в Windows поддерживает zip)
func extractZip(src, dest string) error {
	cmd := exec.Command("tar", "-xf", src, "-C", dest)
	return cmd.Run()
}

func writeConfig(path string) error {
	content := `[General]
ConfigVersion=1.3
ApplicationTheme=dark
IconTheme=breeze_dark
StatusBarVisible=true
ToolbarsLocked=true
AutomaticJavaDownload=true
AutomaticJavaSwitch=true
IgnoreJavaCompatibility=true
IgnoreJavaWizard=true
UseOptimizedJvmArgs=true
GarbageCollectorPreset=ZGC
MaxMemAlloc=3072
MinMemAlloc=3072
UserAskedAboutAutomaticJavaDownload=true
MainWindowGeometry=AdnQywADAAAAAAITAAAA/QAABWwAAAOYAAACEwAAARwAAAVsAAADmAAAAAAAAAAAB4AAAAITAAABHAAABWwAAAOY
MainWindowState="AAAA/wAAAAD9AAAAAAAAA1oAAAJAAAAABAAAAAQAAAAIAAAACPwAAAADAAAAAQAAAAEAAAAeAGkAbgBzAHQAYQBuAGMAZQBUAG8AbwBsAEIAYQByAgAAAAD/////AAAAAAAAAAAAAAACAAAAAQAAABYAbQBhAGkAbgBUAG8AbwBsAEIAYQByAQAAAAD/////AAAAAAAAAAAAAAADAAAAAQAAABYAbgBlAHcAcwBUAG8AbwBsAEIAYQByAAAAAAD/////AAAAAAAAAAA="
WideBarVisibility_instanceToolBar="MTExMTExMTExLE1URjFQcWRVWjg0RkxHQlV5a0dNTDU3RW05OD0="
UseSystemLocale=true
Language=ru
`
	return os.WriteFile(path, []byte(content), 0644)
}

func registerProtocols(appDir string) {
	exePath := filepath.Join(appDir, app+".exe")
	cmds := [][]string{
		{"reg", "add", "HKCU\\Software\\Classes\\curseforge", "/v", "URL Protocol", "/t", "REG_SZ", "/d", "", "/f"},
		{"reg", "add", "HKCU\\Software\\Classes\\curseforge\\shell\\open\\command", "/ve", "/t", "REG_SZ", "/d", "\"" + exePath + "\" \"%1\"", "/f"},
		{"reg", "add", "HKCU\\Software\\Classes\\" + app, "/v", "URL Protocol", "/t", "REG_SZ", "/d", "", "/f"},
		{"reg", "add", "HKCU\\Software\\Classes\\" + app + "\\shell\\open\\command", "/ve", "/t", "REG_SZ", "/d", "\"" + exePath + "\" \"%1\"", "/f"},
		{"reg", "add", "HKCU\\Software\\Classes\\prismlauncher", "/v", "URL Protocol", "/t", "REG_SZ", "/d", "", "/f"},
		{"reg", "add", "HKCU\\Software\\Classes\\prismlauncher\\shell\\open\\command", "/ve", "/t", "REG_SZ", "/d", "\"" + exePath + "\" \"%1\"", "/f"},
	}
	for _, args := range cmds {
		exec.Command(args[0], args[1:]...).Run()
	}
}

func createShortcut(appDir string) {
	exePath := filepath.Join(appDir, app+".exe")
	desktop := filepath.Join(os.Getenv("USERPROFILE"), "Desktop", "PrismLauncher.lnk")
	psCmd := fmt.Sprintf(`$s=(New-Object -ComObject WScript.Shell).CreateShortcut('%s');$s.TargetPath='%s';$s.WorkingDirectory='%s';$s.IconLocation='%s';$s.Save()`,
		desktop, exePath, appDir, exePath)
	exec.Command("powershell", "-nop", "-c", psCmd).Run()
}