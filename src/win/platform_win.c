/**
 * platform_win.c - Windows console and path helpers
 */

#include "../platform.h"
#include <windows.h>
#include <stdlib.h>

void plat_init(void)
{
    /* Enables ANSI color support on the Windows console */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(hOut, &mode))
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void plat_clear(void)       { system("cls"); }
void plat_sleep_ms(int ms)  { Sleep((DWORD)ms); }

const char *platform_state_file(void) { return "voidtune_state.txt"; }

const char *plat_admin_hint(void)
{
    return "Error. Check that you're running as Administrator.";
}
