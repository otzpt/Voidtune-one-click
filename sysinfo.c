/**
 * sysinfo.c — System info collection implementation
 */

#include "sysinfo.h"
#include <stdio.h>
#include <string.h>

void sysinfo_get(SysInfo *out)
{
    if (!out) return;
    memset(out, 0, sizeof(SysInfo));

    /* ── OS name ─────────────────────────────────────────────── */
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD size = sizeof(out->os_name);
        RegQueryValueExA(hKey, "ProductName", NULL, NULL,
                         (BYTE *)out->os_name, &size);
        RegCloseKey(hKey);
    }

    /* ── CPU name (via registry) ─────────────────────────────── */
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD size = sizeof(out->cpu_name);
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL,
                         (BYTE *)out->cpu_name, &size);
        RegCloseKey(hKey);
    }

    /* ── Cores and threads ───────────────────────────────────── */
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    out->cpu_threads = (int)si.dwNumberOfProcessors;
    /* physical cores would require WMI — use thread count as an approximation for now */
    out->cpu_cores = out->cpu_threads;

    /* ── RAM ────────────────────────────────────────────────── */
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        out->ram_total_mb = mem.ullTotalPhys / (1024 * 1024);
        out->ram_free_mb  = mem.ullAvailPhys / (1024 * 1024);
    }

    /* ── Admin check ────────────────────────────────────────── */
    BOOL is_admin = FALSE;
    HANDLE token = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elev;
        DWORD size = sizeof(elev);
        if (GetTokenInformation(token, TokenElevation, &elev, size, &size))
            is_admin = elev.TokenIsElevated;
        CloseHandle(token);
    }
    out->is_admin = (int)is_admin;
}

void sysinfo_print(const SysInfo *info)
{
    printf("  OS      : %s\n", info->os_name);
    printf("  CPU     : %s\n", info->cpu_name);
    printf("  Threads : %d\n", info->cpu_threads);
    printf("  RAM     : %llu MB total / %llu MB free\n",
           info->ram_total_mb, info->ram_free_mb);
    printf("  Admin   : %s\n", info->is_admin ? "YES" : "NO (running as Administrator is recommended)");
}
