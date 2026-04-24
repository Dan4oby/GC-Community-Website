#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define URL "https://github.com/ElyPrismLauncher/Launcher/releases/download/11.0.2/PineconeMC-Windows-MinGW-w64-Portable-11.0.2.zip"
#define APP_NAME "ElyPrismLauncher"

// Прототипы
static void fail(const char *msg);
static int proc_running(const char *exename);
static int download_file(const char *url, const char *dest);
static int write_config(const char *cfg_path);
static void register_protocols(const char *app_dir);
static void create_shortcut(const char *app_dir);

int main(void) {
    // Устанавливаем UTF-8 кодовую страницу консоли
    system("chcp 65001 > nul");

    // Путь к %APPDATA%
    const char *appdata = getenv("APPDATA");
    if (!appdata) fail("Не удалось получить APPDATA");

    char app_dir[MAX_PATH];
    snprintf(app_dir, sizeof(app_dir), "%s\\%s", appdata, APP_NAME);

    // Проверка наличия tar
    char tar_cmd[MAX_PATH];
    if (SearchPath(NULL, "tar.exe", NULL, sizeof(tar_cmd), tar_cmd, NULL) == 0) {
        fail("Bat не может работать на этом ПК.\nURL: " URL);
    }

    // Завершаем процесс, если запущен
    system("taskkill /IM " APP_NAME ".exe /F >nul 2>&1");
    if (proc_running(APP_NAME ".exe")) {
        fail("Закройте " APP_NAME " и запустите снова");
    }

    // Скачивание
    printf("Загрузка %s...\n", APP_NAME);
    char zip_path[MAX_PATH];
    snprintf(zip_path, sizeof(zip_path), "%s.zip", APP_NAME);
    if (download_file(URL, zip_path) != 0) {
        fail("Ошибка загрузки");
    }

    // Распаковка
    printf("Распаковка...\n");
    char mkdir_cmd[MAX_PATH + 20];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir \"%s\" 2>nul", app_dir);
    system(mkdir_cmd);

    char extract_cmd[MAX_PATH * 2 + 50];
    snprintf(extract_cmd, sizeof(extract_cmd), "tar -xf \"%s\" -C \"%s\"", zip_path, app_dir);
    if (system(extract_cmd) != 0) fail("Ошибка распаковки");
    remove(zip_path);

    // Конфигурационный файл
    char cfg_path[MAX_PATH];
    snprintf(cfg_path, sizeof(cfg_path), "%s\\%s.cfg", app_dir, APP_NAME);
    if (write_config(cfg_path) != 0) fail("Ошибка создания конфига");

    // Регистрация протоколов
    register_protocols(app_dir);

    // Создание ярлыка
    create_shortcut(app_dir);

    // Запуск приложения
    char exe_path[MAX_PATH];
    snprintf(exe_path, sizeof(exe_path), "%s\\%s.exe", app_dir, APP_NAME);
    char launch_cmd[MAX_PATH + 20];
    snprintf(launch_cmd, sizeof(launch_cmd), "start \"\" \"%s\"", exe_path);
    system(launch_cmd);

    printf("Готово!\n");
    return 0;
}

static void fail(const char *msg) {
    printf("[ОШИБКА] %s\n", msg);
    printf("Нажмите Enter...");
    getchar();
    exit(EXIT_FAILURE);
}

static int proc_running(const char *exename) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "tasklist /FI \"IMAGENAME eq %s\" 2>nul | find /i \"%s\" >nul",
             exename, exename);
    return system(cmd) == 0;
}

static int download_file(const char *url, const char *dest) {
    char cmd[2048];
    // Пробуем curl
    if (SearchPath(NULL, "curl.exe", NULL, 0, NULL, NULL) != 0) {
        snprintf(cmd, sizeof(cmd), "curl -L -# -o \"%s\" \"%s\"", dest, url);
        return system(cmd);
    }
    // Fallback – PowerShell
    snprintf(cmd, sizeof(cmd),
             "powershell -NoProfile -Command "
             "\"[Net.ServicePointManager]::SecurityProtocol=3072;"
             "(New-Object Net.WebClient).DownloadFile('%s','%s')\"",
             url, dest);
    return system(cmd);
}

static int write_config(const char *cfg_path) {
    const char *content =
        "[General]\n"
        "ConfigVersion=1.3\n"
        "ApplicationTheme=dark\n"
        "IconTheme=breeze_dark\n"
        "StatusBarVisible=true\n"
        "ToolbarsLocked=true\n"
        "AutomaticJavaDownload=true\n"
        "AutomaticJavaSwitch=true\n"
        "IgnoreJavaCompatibility=true\n"
        "IgnoreJavaWizard=true\n"
        "UseOptimizedJvmArgs=true\n"
        "GarbageCollectorPreset=ZGC\n"
        "MaxMemAlloc=3072\n"
        "MinMemAlloc=3072\n"
        "UserAskedAboutAutomaticJavaDownload=true\n"
        "MainWindowGeometry=AdnQywADAAAAAAITAAAA/QAABWwAAAOYAAACEwAAARwAAAVsAAADmAAAAAAAAAAAB4AAAAITAAABHAAABWwAAAOY\n"
        "MainWindowState=\"AAAA/wAAAAD9AAAAAAAAA1oAAAJAAAAABAAAAAQAAAAIAAAACPwAAAADAAAAAQAAAAEAAAAeAGkAbgBzAHQAYQBuAGMAZQBUAG8AbwBsAEIAYQByAgAAAAD/////AAAAAAAAAAAAAAACAAAAAQAAABYAbQBhAGkAbgBUAG8AbwBsAEIAYQByAQAAAAD/////AAAAAAAAAAAAAAADAAAAAQAAABYAbgBlAHcAcwBUAG8AbwBsAEIAYQByAAAAAAD/////AAAAAAAAAAA=\"\n"
        "WideBarVisibility_instanceToolBar=\"MTExMTExMTExLE1URjFQcWRVWjg0RkxHQlV5a0dNTDU3RW05OD0=\"\n"
        "UseSystemLocale=true\n"
        "Language=ru\n";

    FILE *f = fopen(cfg_path, "w");
    if (!f) return 1;
    fputs(content, f);
    fclose(f);
    return 0;
}

static void register_protocols(const char *app_dir) {
    char exe_path[MAX_PATH];
    snprintf(exe_path, sizeof(exe_path), "%s\\%s.exe", app_dir, APP_NAME);

    char cmd[2048];

    // curseforge
    system("reg add HKCU\\Software\\Classes\\curseforge /v \"URL Protocol\" /t REG_SZ /d \"\" /f");
    snprintf(cmd, sizeof(cmd),
             "reg add HKCU\\Software\\Classes\\curseforge\\shell\\open\\command /ve /t REG_SZ "
             "/d \"\\\"%s\\\" \\\"%%1\\\"\" /f", exe_path);
    system(cmd);

    // ElyPrismLauncher (протокол app)
    snprintf(cmd, sizeof(cmd),
             "reg add HKCU\\Software\\Classes\\%s /v \"URL Protocol\" /t REG_SZ /d \"\" /f",
             APP_NAME);
    system(cmd);
    snprintf(cmd, sizeof(cmd),
             "reg add HKCU\\Software\\Classes\\%s\\shell\\open\\command /ve /t REG_SZ "
             "/d \"\\\"%s\\\" \\\"%%1\\\"\" /f", APP_NAME, exe_path);
    system(cmd);

    // prismlauncher
    system("reg add HKCU\\Software\\Classes\\prismlauncher /v \"URL Protocol\" /t REG_SZ /d \"\" /f");
    snprintf(cmd, sizeof(cmd),
             "reg add HKCU\\Software\\Classes\\prismlauncher\\shell\\open\\command /ve /t REG_SZ "
             "/d \"\\\"%s\\\" \\\"%%1\\\"\" /f", exe_path);
    system(cmd);
}

static void create_shortcut(const char *app_dir) {
    char exe_path[MAX_PATH];
    snprintf(exe_path, sizeof(exe_path), "%s\\%s.exe", app_dir, APP_NAME);

    const char *userprofile = getenv("USERPROFILE");
    if (!userprofile) return;
    char desktop[MAX_PATH];
    snprintf(desktop, sizeof(desktop), "%s\\Desktop\\PrismLauncher.lnk", userprofile);

    char ps_cmd[2048];
    snprintf(ps_cmd, sizeof(ps_cmd),
             "powershell -NoProfile -Command "
             "\"$s=(New-Object -ComObject WScript.Shell).CreateShortcut('%s');"
             "$s.TargetPath='%s';$s.WorkingDirectory='%s';$s.IconLocation='%s';$s.Save()\"",
             desktop, exe_path, app_dir, exe_path);
    system(ps_cmd);
}