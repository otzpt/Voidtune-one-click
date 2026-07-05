/**
 * main.c — VOIDTUNE Lite (native C edition)
 * The tiny, zero-dependency CLI edition of VOIDTUNE. ~160 KB, no runtime,
 * runs anywhere Windows runs (WinPE, servers, stripped installs).
 * The full GUI flagship is the C# / WinUI 3 edition.
 *
 * Compilar (MSYS2 UCRT64):
 *   gcc -Wall -Wextra -o voidtune.exe main.c tweaks.c registry.c sysinfo.c -ladvapi32
 *
 * Ou simplesmente: make
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "tweaks.h"
#include "sysinfo.h"

#define VT_VERSION "0.1-c"

/* ── Protótipos internos ─────────────────────────────────────── */
static void clear_screen(void);
static void print_header(void);
static void print_separator(void);
static void menu_main(void);
static void menu_oneclick(void);
static void menu_tweaks(void);
static void menu_sysinfo(void);
static int  read_int(const char *prompt);

/* ── Entry point ─────────────────────────────────────────────── */
int main(void)
{
    /* Activa suporte a UTF-8 e cores ANSI na consola do Windows */
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    /* Inicializa o motor de tweaks (carrega estados do disco) */
    tweaks_init();

    menu_main();
    return EXIT_SUCCESS;
}

/* ── Utilitários de UI ───────────────────────────────────────── */

static void clear_screen(void)
{
    system("cls");
}

static void print_separator(void)
{
    printf("\033[38;5;240m");  /* cinzento escuro */
    printf("  ────────────────────────────────────────────────────\n");
    printf("\033[0m");
}

static void print_header(void)
{
    printf("\n");
    printf("\033[35m");  /* roxo */
    printf("  ██╗   ██╗ ██████╗ ██╗██████╗ ████████╗██╗   ██╗███╗   ██╗███████╗\n");
    printf("  ██║   ██║██╔═══██╗██║██╔══██╗╚══██╔══╝██║   ██║████╗  ██║██╔════╝\n");
    printf("  ██║   ██║██║   ██║██║██║  ██║   ██║   ██║   ██║██╔██╗ ██║█████╗  \n");
    printf("  ╚██╗ ██╔╝██║   ██║██║██║  ██║   ██║   ██║   ██║██║╚██╗██║██╔══╝  \n");
    printf("   ╚████╔╝ ╚██████╔╝██║██████╔╝   ██║   ╚██████╔╝██║ ╚████║███████╗\n");
    printf("    ╚═══╝   ╚═════╝ ╚═╝╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═══╝╚══════╝\n");
    printf("\033[0m");
    printf("\033[38;5;240m  ONE-CLICK OPTIMIZER · v%s · @OTZPT_DEV · GPL v3\033[0m\n\n", VT_VERSION);
}

/* ── Menu principal ──────────────────────────────────────────── */
static void menu_main(void)
{
    int choice;

    while (1) {
        clear_screen();
        print_header();
        print_separator();

        printf("  \033[35m[1] OTIMIZAR AGORA\033[0m  \033[38;5;240m(one-click - todos os SAFE)\033[0m\n");
        printf("  \033[37m[2]\033[0m Tweaks (manual)\n");
        printf("  \033[37m[3]\033[0m Informacao do Sistema\n");
        printf("  \033[37m[0]\033[0m Sair\n\n");

        print_separator();
        choice = read_int("  > ");

        switch (choice) {
            case 1: menu_oneclick(); break;
            case 2: menu_tweaks();  break;
            case 3: menu_sysinfo(); break;
            case 0:
                printf("\n  VOIDTUNE encerrado.\n\n");
                return;
            default:
                printf("\n  \033[33mOpcao invalida.\033[0m\n");
                Sleep(800);
        }
    }
}

/* ── One-click: aplica todos os tweaks SAFE de uma vez ───────── */
static void menu_oneclick(void)
{
    clear_screen();
    print_header();

    printf("  \033[35m// OTIMIZACAO AUTOMATICA\033[0m\n\n");
    print_separator();
    printf("  Aplica todos os tweaks \033[32mSAFE\033[0m de uma vez.\n");
    printf("  Tudo reversivel no menu Tweaks. Corre como Administrador.\n\n");
    printf("  Continuar? [s/N] ");

    char resp[8];
    if (!fgets(resp, sizeof(resp), stdin)) return;
    if (resp[0] != 's' && resp[0] != 'S') return;

    int total = tweaks_count(), applied = 0, failed = 0, already = 0;
    printf("\n");
    for (int i = 0; i < total; i++) {
        Tweak *t = tweaks_get(i);
        if (t->level != SAFE) continue;            /* so os seguros no one-click */
        if (t->state == STATE_ON) { already++; continue; }

        printf("  \033[38;5;240m>\033[0m %-24s ", t->name);
        if (tweaks_apply(i) == 0) { printf("\033[32mOK\033[0m\n");    applied++; }
        else                      { printf("\033[31mfalhou\033[0m\n"); failed++;  }
    }

    printf("\n");
    print_separator();
    printf("  \033[32m%d aplicados\033[0m", applied);
    if (already) printf(" \033[38;5;240m- %d ja activos\033[0m", already);
    if (failed)  printf(" - \033[31m%d falharam\033[0m", failed);
    printf("\n  Reinicia o PC para efeito completo.\n\n");
    print_separator();

    read_int("  [Enter para voltar] > ");
}

/* ── Menu de tweaks ──────────────────────────────────────────── */
static void menu_tweaks(void)
{
    int choice;
    int total = tweaks_count();

    while (1) {
        clear_screen();
        print_header();

        printf("  \033[35m// TWEAKS\033[0m\n\n");
        print_separator();

        /* Lista todos os tweaks */
        for (int i = 0; i < total; i++) {
            Tweak *t = tweaks_get(i);

            /* Cor do estado: verde = ON, cinzento = OFF */
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
        printf("  Numero do tweak para toggle | \033[37m[0]\033[0m Voltar\n\n");

        choice = read_int("  > ");

        if (choice == 0) return;

        int idx = choice - 1;
        if (idx < 0 || idx >= total) {
            printf("\n  \033[33mOpcao invalida.\033[0m\n");
            Sleep(800);
            continue;
        }

        Tweak *t = tweaks_get(idx);

        /* Warning para tweaks EXTREME */
        if (t->level == EXTREME && t->state == STATE_OFF) {
            printf("\n  \033[31m[EXTREME]\033[0m Este tweak e agressivo. Tens a certeza? [s/N] ");
            char resp[8];
            fgets(resp, sizeof(resp), stdin);
            if (resp[0] != 's' && resp[0] != 'S') continue;
        }

        int res;
        if (t->state == STATE_OFF) {
            printf("\n  A aplicar \033[35m%s\033[0m...\n", t->name);
            res = tweaks_apply(idx);
        } else {
            printf("\n  A reverter \033[35m%s\033[0m...\n", t->name);
            res = tweaks_revert(idx);
        }

        if (res == 0)
            printf("  \033[32mOK\033[0m\n");
        else
            printf("  \033[31mErro. Verifica se correste como Administrador.\033[0m\n");

        Sleep(1000);
    }
}

/* ── Menu de info do sistema ─────────────────────────────────── */
static void menu_sysinfo(void)
{
    clear_screen();
    print_header();

    printf("  \033[35m// SISTEMA\033[0m\n\n");
    print_separator();

    SysInfo info;
    sysinfo_get(&info);
    sysinfo_print(&info);

    print_separator();

    /* Mostra tweaks activos */
    int active = 0;
    for (int i = 0; i < tweaks_count(); i++)
        if (tweaks_get(i)->state == STATE_ON) active++;

    printf("  Tweaks activos : \033[35m%d\033[0m / %d\n\n", active, tweaks_count());
    print_separator();

    read_int("  [Enter para voltar] > ");
}

/* ── Lê um inteiro da consola de forma segura ────────────────── */
static int read_int(const char *prompt)
{
    char buf[32];
    printf("%s", prompt);
    if (!fgets(buf, sizeof(buf), stdin)) return -1;
    return atoi(buf);
}
