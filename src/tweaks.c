/**
 * tweaks.c - Tweak engine (platform independent)
 *
 * The tweak table itself comes from the platform backend
 * (win/tweaks_win.c or linux/tweaks_linux.c) via platform_tweaks().
 */

#include "tweaks.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Tweak *g_tweaks;
static int    g_count;

/* -- Public API ------------------------------------------------ */

void tweaks_init(void)
{
    g_tweaks = platform_tweaks(&g_count);
    tweaks_load_state();
}

int tweaks_count(void)
{
    return g_count;
}

Tweak *tweaks_get(int index)
{
    if (index < 0 || index >= g_count)
        return NULL;
    return &g_tweaks[index];
}

int tweaks_apply(int index)
{
    Tweak *t = tweaks_get(index);
    if (!t) return -1;

    int res = t->apply();
    if (res == 0) {
        t->state = STATE_ON;
        tweaks_save_state();
    }
    return res;
}

int tweaks_revert(int index)
{
    Tweak *t = tweaks_get(index);
    if (!t) return -1;

    int res = t->revert();
    if (res == 0) {
        t->state = STATE_OFF;
        tweaks_save_state();
    }
    return res;
}

void tweaks_save_state(void)
{
    const char *path = platform_state_file();
    if (!path) return;
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "voidtune: cannot write state file %s\n", path);
        return;
    }

    for (int i = 0; i < g_count; i++)
        fprintf(f, "%s=%d\n", g_tweaks[i].name, (int)g_tweaks[i].state);

    fclose(f);
}

void tweaks_load_state(void)
{
    const char *path = platform_state_file();
    FILE *f = path ? fopen(path, "r") : NULL;
    if (!f) return;  /* no file = everything OFF, that's fine */

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        /* format: "Game Mode=1" */
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';  /* splits the string into name + value */
        int val = atoi(eq + 1);

        /* looks up the tweak by name */
        for (int i = 0; i < g_count; i++) {
            if (strcmp(g_tweaks[i].name, line) == 0) {
                g_tweaks[i].state = val ? STATE_ON : STATE_OFF;
                break;
            }
        }
    }

    fclose(f);
}

const char *tweak_category_str(TweakCategory c)
{
    switch (c) {
        case CAT_PERFORMANCE: return "PERFORMANCE";
        case CAT_PRIVACY:     return "PRIVACY";
        case CAT_NETWORK:     return "NETWORK";
        case CAT_SERVICES:    return "SERVICES";
        case CAT_VISUALS:     return "VISUALS";
        default:              return "UNKNOWN";
    }
}

const char *tweak_level_str(TweakLevel l)
{
    return (l == SAFE) ? "SAFE" : "EXTREME";
}

const char *tweak_state_str(TweakState s)
{
    return (s == STATE_ON) ? "ON" : "OFF";
}

