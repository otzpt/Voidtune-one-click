/**
 * sysinfo.h — System information (CPU, RAM, OS)
 */

#ifndef SYSINFO_H
#define SYSINFO_H

#include <windows.h>

typedef struct {
    char os_name[128];       /* e.g. "Windows 11 Pro" */
    char cpu_name[128];      /* e.g. "AMD Ryzen 5 7600X" */
    int  cpu_cores;
    int  cpu_threads;
    unsigned long long ram_total_mb;
    unsigned long long ram_free_mb;
    int  is_admin;           /* 1 if running as Administrator */
} SysInfo;

/* Fills the SysInfo struct with current system data */
void sysinfo_get(SysInfo *out);

/* Prints a SysInfo summary to the console */
void sysinfo_print(const SysInfo *info);

#endif /* SYSINFO_H */
