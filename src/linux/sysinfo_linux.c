/**
 * sysinfo_linux.c - system info from /proc and os-release
 */

#include "../sysinfo.h"
#include "distro.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

void sysinfo_get(SysInfo *out)
{
    if (!out) return;
    memset(out, 0, sizeof(SysInfo));

    Distro d;
    distro_detect(&d);
    snprintf(out->os_name, sizeof(out->os_name), "%s", d.pretty);

    struct stat st;
    snprintf(out->extra, sizeof(out->extra), "family %s, %s%s",
             distro_family_str(d.family),
             stat("/run/systemd/system", &st) == 0 ? "systemd" : "no systemd (runit or other)",
             d.family == FAM_UNKNOWN ? ", untested distro" : "");

    out->cpu_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    out->cpu_cores = out->cpu_threads;

    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char *colon = strchr(line, ':');
            if (!colon) continue;
            if (!out->cpu_name[0] && strncmp(line, "model name", 10) == 0) {
                snprintf(out->cpu_name, sizeof(out->cpu_name), "%s", colon + 2);
                out->cpu_name[strcspn(out->cpu_name, "\n")] = '\0';
            }
            /* SIMPLIFICATION: assumes one socket. Wrong on multi-socket
             * boards; sum "cpu cores" per "physical id" to fix. */
            if (strncmp(line, "cpu cores", 9) == 0 && atoi(colon + 1) > 0)
                out->cpu_cores = atoi(colon + 1);
        }
        fclose(f);
    }

    f = fopen("/proc/meminfo", "r");
    if (f) {
        char line[128];
        unsigned long long kb;
        while (fgets(line, sizeof(line), f)) {
            if (sscanf(line, "MemTotal: %llu kB", &kb) == 1)     out->ram_total_mb = kb / 1024;
            if (sscanf(line, "MemAvailable: %llu kB", &kb) == 1) out->ram_free_mb  = kb / 1024;
        }
        fclose(f);
    }

    out->is_admin = (geteuid() == 0);
}

void sysinfo_print(const SysInfo *info)
{
    printf("  OS      : %s\n", info->os_name);
    if (info->extra[0])
        printf("  Distro  : %s\n", info->extra);
    printf("  CPU     : %s\n", info->cpu_name);
    printf("  Cores   : %d (%d threads)\n", info->cpu_cores, info->cpu_threads);
    printf("  RAM     : %llu MB total / %llu MB free\n",
           info->ram_total_mb, info->ram_free_mb);
    printf("  Root    : %s\n", info->is_admin ? "YES" : "NO (run with sudo to apply tweaks)");
}
