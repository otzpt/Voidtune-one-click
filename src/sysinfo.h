/**
 * sysinfo.h - System information (CPU, RAM, OS)
 * Implemented by win/sysinfo_win.c and linux/sysinfo_linux.c.
 */

#ifndef SYSINFO_H
#define SYSINFO_H

typedef struct {
    char os_name[128];       /* e.g. "Windows 11 Pro", "Arch Linux" */
    char cpu_name[128];      /* e.g. "AMD Ryzen 5 7600X" */
    char extra[128];         /* Linux: distro family and init system */
    int  cpu_cores;
    int  cpu_threads;
    unsigned long long ram_total_mb;
    unsigned long long ram_free_mb;
    int  is_admin;           /* 1 if Administrator (Windows) or root (Linux) */
} SysInfo;

/* Fills the SysInfo struct with current system data */
void sysinfo_get(SysInfo *out);

/* Prints a SysInfo summary to the console */
void sysinfo_print(const SysInfo *info);

#endif /* SYSINFO_H */
