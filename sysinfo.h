/**
 * sysinfo.h — Informação do sistema (CPU, RAM, OS)
 */

#ifndef SYSINFO_H
#define SYSINFO_H

#include <windows.h>

typedef struct {
    char os_name[128];       /* ex: "Windows 11 Pro" */
    char cpu_name[128];      /* ex: "AMD Ryzen 5 7600X" */
    int  cpu_cores;
    int  cpu_threads;
    unsigned long long ram_total_mb;
    unsigned long long ram_free_mb;
    int  is_admin;           /* 1 se a correr como Administrador */
} SysInfo;

/* Preenche a struct SysInfo com dados actuais do sistema */
void sysinfo_get(SysInfo *out);

/* Imprime um resumo da SysInfo na consola */
void sysinfo_print(const SysInfo *info);

#endif /* SYSINFO_H */
