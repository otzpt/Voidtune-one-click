/**
 * platform_linux.c - terminal and path helpers for Linux
 *
 * State directory: /var/lib/voidtune (root). Overridable with
 * VOIDTUNE_STATE_DIR. In dry-run mode (VOIDTUNE_DRY_RUN set) the state
 * lives under /tmp so a non-root dry run never touches system paths.
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
    else if (lx_dry_run())  snprintf(dir, sizeof(dir), "/tmp/voidtune-dryrun");
    else                    snprintf(dir, sizeof(dir), "/var/lib/voidtune");
    mkdir(dir, 0755);   /* parent must exist; failure surfaces when the file is written */
    return dir;
}

const char *platform_state_file(void)
{
    static char path[300];
    snprintf(path, sizeof(path), "%s/voidtune_state.txt", lx_state_dir());
    return path;
}

const char *plat_admin_hint(void)
{
    return "Error. Check that you're running as root (sudo).";
}
