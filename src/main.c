/**
 * main.c - VOIDTUNE One-Click (native C edition)
 * Tiny, zero-dependency CLI optimizer for Windows and Linux.
 * The full GUI flagship is the C# / WinUI 3 edition (Windows only).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tweaks.h"
#include "sysinfo.h"
#include "platform.h"
#ifdef _WIN32
#include "win/backup.h"
#endif

#define VT_VERSION "0.1.0"

/* -- Internal prototypes --------------------------------------- */
static void clear_screen(void);
static void print_header(void);
static void print_separator(void);
static void menu_main(void);
static void menu_oneclick(void);
static void menu_tweaks(void);
static void menu_sysinfo(void);
static void menu_revert_all(void);
static int  read_int(const char *prompt);

/* -- Entry point ----------------------------------------------- */
int main(void)
{
    plat_init();
    tweaks_init();
    menu_main();
    return EXIT_SUCCESS;
}

/* -- UI utilities ----------------------------------------------- */

static void clear_screen(void)
{
    plat_clear();
}

static void print_separator(void)
{
    printf("\033[38;5;240m");  /* dark grey */
    printf("  ----------------------------------------------------\n");
    printf("\033[0m");
}

static void print_header(void)
{
    printf("\n");
    printf("\033[35m");  /* purple */
    printf("  __     ______ ___ ____  _____ _   _ _   _ _____\n");
    printf("  \\ \\   / / __ \\_ _|  _ \\|_   _| | | | \\ | | ____|\n");
    printf("   \\ \\ / / |  | | || | | | | | | | | |  \\| |  _|\n");
    printf("    \\ V /| |__| | || |_| | | | | |_| | |\\  | |___\n");
    printf("     \\_/  \\____/___|____/  |_|  \\___/|_| \\_|_____|\n");
    printf("\033[0m");
    printf("\033[38;5;240m  ONE-CLICK OPTIMIZER - v%s - @OTZPT_DEV - GPL v3\033[0m\n\n", VT_VERSION);
}

/* -- Main menu -------------------------------------------------- */
static void menu_main(void)
{
    int choice;

    while (1) {
        clear_screen();
        print_header();
        print_separator();

        printf("  \033[35m[1] OPTIMIZE NOW\033[0m  \033[38;5;240m(one-click - all SAFE tweaks)\033[0m\n");
        printf("  \033[37m[2]\033[0m Tweaks (manual)\n");
        printf("  \033[37m[3]\033[0m System Information\n");
        printf("  \033[37m[4]\033[0m Revert all tweaks\n");
        printf("  \033[37m[0]\033[0m Exit\n\n");

        print_separator();
        choice = read_int("  > ");

        switch (choice) {
            case 1: menu_oneclick(); break;
            case 2: menu_tweaks();  break;
            case 3: menu_sysinfo(); break;
            case 4: menu_revert_all(); break;
            case 0:
                printf("\n  VOIDTUNE closed.\n\n");
                return;
            default:
                printf("\n  \033[33mInvalid option.\033[0m\n");
                plat_sleep_ms(800);
        }
    }
}

/* -- One-click: applies every SAFE tweak at once --------------- */
static void menu_oneclick(void)
{
    clear_screen();
    print_header();

    printf("  \033[35m// AUTOMATIC OPTIMIZATION\033[0m\n\n");
    print_separator();
    printf("  Applies every \033[32mSAFE\033[0m tweak at once.\n");
    printf("  Everything is reversible from the Tweaks menu.\n");
    SysInfo si;
    sysinfo_get(&si);
    printf("  System: %s\n\n", si.os_name);
    if (!si.is_admin)
        printf("  \033[33mWarning: not running with administrator/root rights;\n  changes will fail.\033[0m\n\n");
    printf("  Continue? [y/N] ");

    char resp[8];
    if (!fgets(resp, sizeof(resp), stdin)) return;
    if (resp[0] != 'y' && resp[0] != 'Y') return;

    int total = tweaks_count(), applied = 0, failed = 0, already = 0, na = 0;
    printf("\n");
#ifdef _WIN32
    char bk[512];
    if (backup_create("pre-oneclick", bk, sizeof(bk)))
        printf("  Registry backup: %s\n\n", bk);
    else
        printf("  \033[33mRegistry backup failed, continuing.\033[0m\n\n");
#endif
    for (int i = 0; i < total; i++) {
        Tweak *t = tweaks_get(i);
        if (t->level != SAFE) continue;            /* only SAFE tweaks in one-click */
        if (t->state == STATE_ON) { already++; continue; }

        printf("  \033[38;5;240m>\033[0m %-24s ", t->name);
        int res = tweaks_apply(i);
        if (res == 0)             { printf("\033[32mOK\033[0m\n");     applied++; }
        else if (res == TWEAK_NA) { printf("\033[38;5;240mn/a\033[0m\n"); na++;   }
        else                      { printf("\033[31mfailed\033[0m\n"); failed++;  }
    }

    printf("\n");
    print_separator();
    printf("  \033[32m%d applied\033[0m", applied);
    if (already) printf(" \033[38;5;240m- %d already active\033[0m", already);
    if (na)      printf(" - \033[38;5;240m%d n/a on this system\033[0m", na);
    if (failed)  printf(" - \033[31m%d failed\033[0m", failed);
    printf("\n  Reboot for the changes to take full effect.\n\n");
    print_separator();

    read_int("  [Enter to go back] > ");
}

/* -- Tweaks menu ------------------------------------------------ */
static void menu_tweaks(void)
{
    int choice;
    int total = tweaks_count();

    while (1) {
        clear_screen();
        print_header();

        printf("  \033[35m// TWEAKS\033[0m\n\n");
        print_separator();

        /* Lists every tweak */
        for (int i = 0; i < total; i++) {
            Tweak *t = tweaks_get(i);

            /* State color: green = ON, grey = OFF */
            const char *state_color = (t->state == STATE_ON)
                ? "\033[32m" : "\033[38;5;240m";
            const char *level_color = (t->level == EXTREME)
                ? "\033[31m" : "\033[32m";

            printf("  \033[37m[%2d]\033[0m %-22s %s%-7s\033[0m  %s%-9s\033[0m  %s\n",
                i + 1,
                t->name,
                state_color,  tweak_state_str(t->state),
                level_color,  tweak_level_str(t->level),
                t->desc
            );
        }

        print_separator();
        printf("  Enter a tweak number to toggle it | \033[37m[0]\033[0m Back\n\n");

        choice = read_int("  > ");

        if (choice == 0) return;

        int idx = choice - 1;
        if (idx < 0 || idx >= total) {
            printf("\n  \033[33mInvalid option.\033[0m\n");
            plat_sleep_ms(800);
            continue;
        }

        Tweak *t = tweaks_get(idx);

        /* Warning for EXTREME tweaks */
        if (t->level == EXTREME && t->state == STATE_OFF) {
            printf("\n  \033[31m[EXTREME]\033[0m This tweak is aggressive. Are you sure? [y/N] ");
            char resp[8];
            if (!fgets(resp, sizeof(resp), stdin)) return;
            if (resp[0] != 'y' && resp[0] != 'Y') continue;
        }

        int res;
        if (t->state == STATE_OFF) {
            printf("\n  Applying \033[35m%s\033[0m...\n", t->name);
            res = tweaks_apply(idx);
        } else {
            printf("\n  Reverting \033[35m%s\033[0m...\n", t->name);
            res = tweaks_revert(idx);
        }

        if (res == 0)
            printf("  \033[32mOK\033[0m\n");
        else if (res == TWEAK_NA)
            printf("  \033[33mNot applicable on this system.\033[0m\n");
        else
            printf("  \033[31m%s\033[0m\n", plat_admin_hint());

        plat_sleep_ms(1000);
    }
}

/* -- System info menu ------------------------------------------- */
static void menu_sysinfo(void)
{
    clear_screen();
    print_header();

    printf("  \033[35m// SYSTEM\033[0m\n\n");
    print_separator();

    SysInfo info;
    sysinfo_get(&info);
    sysinfo_print(&info);

    print_separator();

    /* Shows active tweaks */
    int active = 0;
    for (int i = 0; i < tweaks_count(); i++)
        if (tweaks_get(i)->state == STATE_ON) active++;

    printf("  Active tweaks : \033[35m%d\033[0m / %d\n\n", active, tweaks_count());
    print_separator();

    read_int("  [Enter to go back] > ");
}

/* -- Revert every active tweak -------------------------------- */
static void menu_revert_all(void)
{
    plat_clear();
    print_header();

    printf("  \033[35m// REVERT ALL\033[0m\n\n");
    print_separator();
    printf("  Reverts every active tweak to its previous value.\n\n");
    printf("  Continue? [y/N] ");

    char resp[8];
    if (!fgets(resp, sizeof(resp), stdin)) return;
    if (resp[0] != 'y' && resp[0] != 'Y') return;

    int reverted = 0, failed = 0;
    printf("\n");
    for (int i = 0; i < tweaks_count(); i++) {
        Tweak *t = tweaks_get(i);
        if (t->state != STATE_ON) continue;

        printf("  \033[38;5;240m>\033[0m %-24s ", t->name);
        if (tweaks_revert(i) == 0) { printf("\033[32mOK\033[0m\n");     reverted++; }
        else                       { printf("\033[31mfailed\033[0m\n"); failed++;   }
    }

    printf("\n");
    print_separator();
    printf("  \033[32m%d reverted\033[0m", reverted);
    if (failed) printf(" - \033[31m%d failed\033[0m", failed);
    printf("\n\n");
    print_separator();

    read_int("  [Enter to go back] > ");
}

/* -- Safely reads an integer from the console ------------------ */
static int read_int(const char *prompt)
{
    char buf[32];
    printf("%s", prompt);
    if (!fgets(buf, sizeof(buf), stdin)) return -1;
    return atoi(buf);
}
