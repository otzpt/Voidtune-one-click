/**
 * platform_linux.c - terminal and path helpers for Linux
 *
 * State directory: /var/lib/voidtune (root). Overridable with
 * VOIDTUNE_STATE_DIR. In dry-run mode (VOIDTUNE_DRY_RUN set) nothing is
 * created or written: no state file, no directory, no /tmp path.
 */

#include "../platform.h"
#include "linux_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

void plat_init(void) { }

void plat_clear(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

void plat_sleep_ms(int ms)
{
    struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

int lx_dry_run(void)
{
    const char *v = getenv("VOIDTUNE_DRY_RUN");
    return v && *v && *v != '0';
}

const char *lx_state_dir(void)
{
    static char dir[256];
    const char *env = getenv("VOIDTUNE_STATE_DIR");
    if (env && *env)        snprintf(dir, sizeof(dir), "%s", env);
    else                    snprintf(dir, sizeof(dir), "/var/lib/voidtune");
    /* parent must exist; failure surfaces when the file is written */
    if (!lx_dry_run()) mkdir(dir, 0755);
    return dir;
}

const char *platform_state_file(void)
{
    static char path[300];
    /* Dry run without an explicit state dir: keep state in memory only. */
    if (lx_dry_run() && !getenv("VOIDTUNE_STATE_DIR")) return NULL;
    snprintf(path, sizeof(path), "%s/voidtune_state.txt", lx_state_dir());
    return path;
}

const char *plat_admin_hint(void)
{
    return "Error. Check that you're running as root (sudo).";
}
