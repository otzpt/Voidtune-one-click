/**
 * tweaks_linux.c - Linux tweak table and implementations
 *
 * Works on any distro with /proc/sys. Persistence uses /etc/sysctl.d, which
 * systemd-sysctl (Arch, CachyOS, Ubuntu, Mint, Zorin) and Void's runit
 * stage 1 (/etc/runit/core-services/08-sysctl.sh) both read at boot.
 *
 * Before changing a value the previous one is saved under
 * <state dir>/prev/, so revert restores what was really there.
 *
 * Environment (for testing):
 *   VOIDTUNE_DRY_RUN=1       print writes and commands, change nothing
 *   VOIDTUNE_STATE_DIR=dir   where state and saved values live
 *   VOIDTUNE_SYSCTL_DIR=dir  replaces /etc/sysctl.d
 *   VOIDTUNE_PROC_DIR=dir    replaces /proc/sys
 *   VOIDTUNE_CPU_DIR=dir     replaces /sys/devices/system/cpu
 */

#include "../tweaks.h"
#include "linux_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <unistd.h>
#include <sys/stat.h>

/* -- File helpers ---------------------------------------------- */

static const char *env_or(const char *name, const char *dflt)
{
    const char *v = getenv(name);
    return (v && *v) ? v : dflt;
}

/* Reads a one-line file into buf, trailing newline removed. 0 on success. */
static int read_line(const char *path, char *buf, size_t n)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    if (!fgets(buf, (int)n, f)) { fclose(f); return -1; }
    fclose(f);
    buf[strcspn(buf, "\n")] = '\0';
    return 0;
}

/* Writes value + newline to path. In dry-run mode only prints. 0 on success. */
static int write_line(const char *path, const char *value)
{
    if (lx_dry_run()) {
        printf("    [dry-run] write '%s' to %s\n", value, path);
        return 0;
    }
    FILE *f = fopen(path, "w");
    if (!f) { fprintf(stderr, "voidtune: cannot open %s\n", path); return -1; }
    int bad = (fprintf(f, "%s\n", value) < 0);
    if (fclose(f) != 0) bad = 1;   /* /proc and /sys report errors on flush */
    if (bad) fprintf(stderr, "voidtune: write to %s rejected\n", path);
    return bad ? -1 : 0;
}

static int remove_file(const char *path)
{
    if (lx_dry_run()) { printf("    [dry-run] remove %s\n", path); return 0; }
    return unlink(path);
}

/* Runs a fixed command line (never built from user input). 0 on success. */
static int run(const char *cmd)
{
    if (lx_dry_run()) { printf("    [dry-run] run: %s\n", cmd); return 0; }
    return system(cmd) == 0 ? 0 : -1;
}

/* Quiet check: returns 1 when the command exits 0. */
static int check(const char *cmd)
{
    return system(cmd) == 0;
}

/* <state dir>/prev/<name>; creates the prev directory. */
static void prev_path(char *out, size_t n, const char *name)
{
    char dir[300];
    snprintf(dir, sizeof(dir), "%s/prev", lx_state_dir());
    if (!lx_dry_run()) mkdir(dir, 0755);
    snprintf(out, n, "%s/%s", dir, name);
}

/* -- Sysctl tweaks --------------------------------------------- */

typedef struct { const char *key; const char *value; } SysctlItem;

static void sysctl_proc_path(char *out, size_t n, const char *key)
{
    snprintf(out, n, "%s/", env_or("VOIDTUNE_PROC_DIR", "/proc/sys"));
    size_t len = strlen(out);
    for (const char *p = key; *p && len + 1 < n; p++)
        out[len++] = (*p == '.') ? '/' : *p;
    out[len] = '\0';
}

static int sysctl_apply(const char *id, const SysctlItem *items, int count)
{
    char path[400], old[128], pv[400], name[200];

    /* Pass 1: every key must exist on this kernel, else not applicable. */
    for (int i = 0; i < count; i++) {
        sysctl_proc_path(path, sizeof(path), items[i].key);
        if (read_line(path, old, sizeof(old)) != 0) return TWEAK_NA;
    }

    int errors = 0;
    for (int i = 0; i < count; i++) {
        sysctl_proc_path(path, sizeof(path), items[i].key);
        read_line(path, old, sizeof(old));

        snprintf(name, sizeof(name), "%s.%s", id, items[i].key);
        prev_path(pv, sizeof(pv), name);
        if (access(pv, F_OK) != 0)               /* keep the first original */
            errors += (write_line(pv, old) != 0);
        errors += (write_line(path, items[i].value) != 0);
    }

    /* Persist across reboots. */
    char conf[400], line[1024] = "";
    snprintf(conf, sizeof(conf), "%s/99-voidtune-%s.conf",
             env_or("VOIDTUNE_SYSCTL_DIR", "/etc/sysctl.d"), id);
    for (int i = 0; i < count; i++) {
        size_t len = strlen(line);
        snprintf(line + len, sizeof(line) - len, "%s = %s%s",
                 items[i].key, items[i].value, i + 1 < count ? "\n" : "");
    }
    errors += (write_line(conf, line) != 0);

    return errors ? -1 : 0;
}

static int sysctl_revert(const char *id, const SysctlItem *items, int count)
{
    char path[400], old[128], pv[400], name[200], conf[400];
    int errors = 0;

    for (int i = 0; i < count; i++) {
        sysctl_proc_path(path, sizeof(path), items[i].key);
        snprintf(name, sizeof(name), "%s.%s", id, items[i].key);
        prev_path(pv, sizeof(pv), name);

        if (read_line(pv, old, sizeof(old)) != 0) {
            fprintf(stderr, "voidtune: no saved value for %s, cannot restore\n",
                    items[i].key);
            errors++;
            continue;
        }
        errors += (write_line(path, old) != 0);
        remove_file(pv);
    }

    snprintf(conf, sizeof(conf), "%s/99-voidtune-%s.conf",
             env_or("VOIDTUNE_SYSCTL_DIR", "/etc/sysctl.d"), id);
    if (access(conf, F_OK) == 0) remove_file(conf);

    return errors ? -1 : 0;
}

/* Declares <id>_apply / <id>_revert for a list of sysctl items. */
#define SYSCTL_TWEAK(id, ...)                                              \
    static const SysctlItem id##_items[] = { __VA_ARGS__ };                \
    static int id##_apply(void) {                                          \
        return sysctl_apply(#id, id##_items,                               \
            (int)(sizeof(id##_items) / sizeof(id##_items[0])));            \
    }                                                                      \
    static int id##_revert(void) {                                         \
        return sysctl_revert(#id, id##_items,                              \
            (int)(sizeof(id##_items) / sizeof(id##_items[0])));            \
    }

SYSCTL_TWEAK(swappiness,    { "vm.swappiness", "10" })
SYSCTL_TWEAK(vfs_cache,     { "vm.vfs_cache_pressure", "50" })
SYSCTL_TWEAK(max_map_count, { "vm.max_map_count", "2147483642" })
SYSCTL_TWEAK(tcp_bbr,       { "net.core.default_qdisc", "fq" },
                            { "net.ipv4.tcp_congestion_control", "bbr" })
SYSCTL_TWEAK(tcp_fastopen,  { "net.ipv4.tcp_fastopen", "3" })
SYSCTL_TWEAK(tcp_buffers,   { "net.core.rmem_max", "16777216" },
                            { "net.core.wmem_max", "16777216" })
SYSCTL_TWEAK(nmi_watchdog,  { "kernel.nmi_watchdog", "0" })

/* -- CPU governor ---------------------------------------------- */

/*
 * Sets every core to the "performance" governor. Not persistent: the
 * kernel or power daemon resets it on reboot. Revert restores each
 * core's saved governor.
 */
static int governor_glob(glob_t *g)
{
    char pat[400];
    snprintf(pat, sizeof(pat), "%s/cpu[0-9]*/cpufreq/scaling_governor",
             env_or("VOIDTUNE_CPU_DIR", "/sys/devices/system/cpu"));
    return glob(pat, 0, NULL, g);
}

static int governor_apply(void)
{
    glob_t g;
    if (governor_glob(&g) != 0) return TWEAK_NA;

    int errors = 0;
    for (size_t i = 0; i < g.gl_pathc; i++) {
        char old[64], pv[400], name[64];
        if (read_line(g.gl_pathv[i], old, sizeof(old)) != 0) { errors++; continue; }
        snprintf(name, sizeof(name), "governor.%zu", i);
        prev_path(pv, sizeof(pv), name);
        if (access(pv, F_OK) != 0) errors += (write_line(pv, old) != 0);
        errors += (write_line(g.gl_pathv[i], "performance") != 0);
    }
    globfree(&g);
    return errors ? -1 : 0;
}

static int governor_revert(void)
{
    glob_t g;
    if (governor_glob(&g) != 0) return TWEAK_NA;

    int errors = 0;
    for (size_t i = 0; i < g.gl_pathc; i++) {
        char old[64], pv[400], name[64];
        snprintf(name, sizeof(name), "governor.%zu", i);
        prev_path(pv, sizeof(pv), name);
        if (read_line(pv, old, sizeof(old)) != 0) { errors++; continue; }
        errors += (write_line(g.gl_pathv[i], old) != 0);
        remove_file(pv);
    }
    globfree(&g);
    return errors ? -1 : 0;
}

/* -- systemd unit tweaks (not applicable on Void / runit) ------ */

static int have_systemd(void)
{
    return access("/run/systemd/system", F_OK) == 0;
}

/* Ubuntu family: stops the whoopsie crash-report uploader. */
static int whoopsie_apply(void)
{
    if (!have_systemd() || !check("systemctl is-enabled whoopsie.service >/dev/null 2>&1"))
        return TWEAK_NA;
    return run("systemctl disable --now whoopsie.service");
}

static int whoopsie_revert(void)
{
    if (!have_systemd()) return TWEAK_NA;
    return run("systemctl enable --now whoopsie.service");
}

/* Weekly SSD TRIM. Skipped when the timer is already enabled. */
static int fstrim_apply(void)
{
    if (!have_systemd() || !check("systemctl cat fstrim.timer >/dev/null 2>&1") ||
        check("systemctl is-enabled fstrim.timer >/dev/null 2>&1"))
        return TWEAK_NA;
    return run("systemctl enable --now fstrim.timer");
}

static int fstrim_revert(void)
{
    if (!have_systemd()) return TWEAK_NA;
    return run("systemctl disable --now fstrim.timer");
}

/* -- Tweak table ----------------------------------------------- */

static Tweak g_tweaks[] = {
    { 1, "Low Swappiness",
      "vm.swappiness=10: keep application memory in RAM longer",
      CAT_PERFORMANCE, SAFE, STATE_OFF, swappiness_apply, swappiness_revert },
    { 2, "VFS Cache Pressure",
      "vm.vfs_cache_pressure=50: keep directory and inode caches longer",
      CAT_PERFORMANCE, SAFE, STATE_OFF, vfs_cache_apply, vfs_cache_revert },
    { 3, "Max Map Count",
      "vm.max_map_count raised for games that map many memory regions",
      CAT_PERFORMANCE, SAFE, STATE_OFF, max_map_count_apply, max_map_count_revert },
    { 4, "TCP BBR",
      "BBR congestion control with the fq queue discipline (needs tcp_bbr)",
      CAT_NETWORK, SAFE, STATE_OFF, tcp_bbr_apply, tcp_bbr_revert },
    { 5, "TCP Fast Open",
      "Enables TCP Fast Open for client and server connections",
      CAT_NETWORK, SAFE, STATE_OFF, tcp_fastopen_apply, tcp_fastopen_revert },
    { 6, "TCP Buffers",
      "Raises the maximum socket buffer sizes to 16 MB",
      CAT_NETWORK, SAFE, STATE_OFF, tcp_buffers_apply, tcp_buffers_revert },
    { 7, "Disable Error Reports",
      "Stops whoopsie, the Ubuntu crash-report uploader (Ubuntu family)",
      CAT_PRIVACY, SAFE, STATE_OFF, whoopsie_apply, whoopsie_revert },
    { 8, "Weekly SSD TRIM",
      "Enables fstrim.timer where it is available and not already on",
      CAT_SERVICES, SAFE, STATE_OFF, fstrim_apply, fstrim_revert },
    { 9, "CPU Performance Governor",
      "All cores use the performance governor until reboot",
      CAT_PERFORMANCE, EXTREME, STATE_OFF, governor_apply, governor_revert },
    { 10, "Disable NMI Watchdog",
      "kernel.nmi_watchdog=0: fewer wakeups, no hard-lockup detection",
      CAT_PERFORMANCE, EXTREME, STATE_OFF, nmi_watchdog_apply, nmi_watchdog_revert },
};

Tweak *platform_tweaks(int *count)
{
    *count = (int)(sizeof(g_tweaks) / sizeof(g_tweaks[0]));
    return g_tweaks;
}
