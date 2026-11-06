/**
 * backup.c - Windows Registry backup
 * Ported from MakeBackup() in core/helpers.ps1 of VOIDTUNE v0.8V
 * Copyright (C) 2026 @OTZPT - GPL v3
 */

#include "backup.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

/* Registry keys covered by the tweaks - mirrors helpers.ps1 */
static const char *BACKUP_KEYS[] = {
    "HKLM\\SYSTEM\\CurrentControlSet\\Control\\PriorityControl",
    "HKLM\\SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers",
    "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
    "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Kernel",
    "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Power",
    "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Processor",
    "HKLM\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
    "HKLM\\SYSTEM\\CurrentControlSet\\Services\\AFD\\Parameters",
    "HKLM\\SYSTEM\\CurrentControlSet\\Services\\USB",
    "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
    "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks\\Games",
    "HKLM\\SOFTWARE\\Microsoft\\Windows\\Dwm",
    "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Psched",
    "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Windows Search",
    "HKLM\\SOFTWARE\\Policies\\Microsoft\\Power\\PowerThrottling",
    "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System",
    "HKCU\\System\\GameConfigStore",
    "HKCU\\Software\\Microsoft\\GameBar",
    "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager",
    "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects",
    "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
    "HKCU\\Software\\Microsoft\\Windows\\DWM",
    "HKCU\\Control Panel\\Desktop",
    "HKCU\\Control Panel\\Mouse",
    "HKCU\\Control Panel\\Accessibility\\StickyKeys",
    "HKCU\\Control Panel\\Accessibility\\ToggleKeys",
    "HKCU\\Control Panel\\Accessibility\\Keyboard Response",
    NULL  /* sentinel */
};

/*
 * Runs  reg.exe export <key> <tmpfile> /y
 * and returns 1 if the file was created.
 */
static int export_key(const char *regkey, const char *tmpfile)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "reg export \"%s\" \"%s\" /y >nul 2>&1",
             regkey, tmpfile);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char full[1100];
    snprintf(full, sizeof(full), "cmd.exe /c %s", cmd);

    if (!CreateProcessA(NULL, full, NULL, NULL, FALSE,
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return 0;

    WaitForSingleObject(pi.hProcess, 8000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    /* Checks the file exists and has content */
    DWORD attr = GetFileAttributesA(tmpfile);
    return (attr != INVALID_FILE_ATTRIBUTES);
}

/*
 * Reads a temporary .reg file (UTF-16 LE, as produced by reg.exe),
 * converts it to UTF-8 and writes it to the destination file starting
 * at line 3 (skipping the "Windows Registry Editor Version 5.00"
 * header and the blank line that follows it).
 */
static void append_key_data(FILE *dest, const char *tmpfile)
{
    /* reg.exe writes UTF-16 LE with a BOM */
    FILE *f = fopen(tmpfile, "rb");
    if (!f) return;

    /* Reads everything as wide chars */
    fseek(f, 0, SEEK_END);
    long fsz = ftell(f);
    rewind(f);

    if (fsz < 4) { fclose(f); return; }

    /* Skips the FF FE BOM if present */
    unsigned char bom[2];
    fread(bom, 1, 2, f);
    if (!(bom[0] == 0xFF && bom[1] == 0xFE)) rewind(f);

    /* Reads as wchar_t */
    long wcount = (fsz / 2) + 1;
    wchar_t *wbuf = (wchar_t *)malloc(wcount * sizeof(wchar_t));
    if (!wbuf) { fclose(f); return; }
    int wread = (int)fread(wbuf, sizeof(wchar_t), wcount - 1, f);
    wbuf[wread] = L'\0';
    fclose(f);

    /* Converts to UTF-8 */
    int needed = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, NULL, 0, NULL, NULL);
    char *utf8 = (char *)malloc(needed + 1);
    if (!utf8) { free(wbuf); return; }
    WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, utf8, needed, NULL, NULL);
    free(wbuf);

    /*
     * Skips the first two lines:
     *   line 0: "Windows Registry Editor Version 5.00"
     *   line 1: blank line
     */
    char *p = utf8;
    int skipped = 0;
    while (*p && skipped < 2) {
        if (*p == '\n') skipped++;
        if (*p == '\r' && *(p+1) == '\n') { p++; }
        p++;
    }

    if (*p) fputs(p, dest);
    fputs("\n", dest);

    free(utf8);
}

/* -- Public API ------------------------------------------------ */

int backup_create(const char *note, char *out_path, int out_size)
{
    /* Timestamp */
    SYSTEMTIME st;
    GetLocalTime(&st);

    char fname[512];
    snprintf(fname, sizeof(fname),
             "backup_%04d%02d%02d_%02d%02d%02d_%s.reg",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond,
             note ? note : "voidtune");

    /* Opens the destination file in UTF-8 text mode */
    FILE *dest = fopen(fname, "w");
    if (!dest) return 0;

    /* .reg header - written exactly once */
    fprintf(dest, "Windows Registry Editor Version 5.00\r\n\r\n");

    /* Exports each key */
    int i = 0;
    char tmpfile[512];
    while (BACKUP_KEYS[i]) {
        snprintf(tmpfile, sizeof(tmpfile), "vt_bk_tmp_%d.reg", i);

        if (export_key(BACKUP_KEYS[i], tmpfile)) {
            append_key_data(dest, tmpfile);
            DeleteFileA(tmpfile);
        }
        i++;
    }

    fclose(dest);

    if (out_path && out_size > 0)
        strncpy(out_path, fname, out_size - 1);

    return 1;
}
