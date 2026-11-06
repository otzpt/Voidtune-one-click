/**
 * platform.h - the small surface each OS backend must provide
 *
 * Implemented by win/platform_win.c and linux/platform_linux.c.
 * Tweak tables live in win/tweaks_win.c and linux/tweaks_linux.c.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include "tweaks.h"

/* One-time console setup (ANSI colors on Windows). */
void plat_init(void);

/* Clears the terminal. */
void plat_clear(void);

/* Sleeps for the given number of milliseconds. */
void plat_sleep_ms(int ms);

/* Returns the tweak table for this OS and stores its length in *count. */
Tweak *platform_tweaks(int *count);

/* Path of the file that records which tweaks are ON. */
const char *platform_state_file(void);

/* Message shown when apply/revert fails (privilege hint). */
const char *plat_admin_hint(void);

#endif /* PLATFORM_H */
