/**
 * tweaks.c — Implementação do motor de tweaks
 *
 * Para adicionar um tweak novo:
 *   1. Declara as funções apply/revert aqui em cima (static int tweak_xxx_apply, etc.)
 *   2. Implementa-as
 *   3. Adiciona a entrada em g_tweaks[]
 */

#include "tweaks.h"
#include "registry.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define STATE_FILE "voidtune_state.txt"
#define MAX_TWEAKS 64

/* ── Protótipos das funções de tweak ─────────────────────────── */
static int tweak_game_mode_apply(void);
static int tweak_game_mode_revert(void);
static int tweak_nagle_apply(void);
static int tweak_nagle_revert(void);
static int tweak_telemetry_apply(void);
static int tweak_telemetry_revert(void);
static int tweak_timer_apply(void);
static int tweak_timer_revert(void);
static int tweak_hpet_apply(void);
static int tweak_hpet_revert(void);
static int tweak_superfetch_apply(void);
static int tweak_superfetch_revert(void);
static int tweak_powerplan_apply(void);
static int tweak_powerplan_revert(void);
static int tweak_xboxdvr_apply(void);
static int tweak_xboxdvr_revert(void);
static int tweak_visualfx_apply(void);
static int tweak_visualfx_revert(void);
static int tweak_cortana_apply(void);
static int tweak_cortana_revert(void);
static int tweak_gpu_priority_apply(void);
static int tweak_gpu_priority_revert(void);
static int tweak_network_throttling_apply(void);
static int tweak_network_throttling_revert(void);
static int tweak_windows_update_apply(void);
static int tweak_windows_update_revert(void);
static int tweak_startup_delay_apply(void);
static int tweak_startup_delay_revert(void);
static int tweak_menushowdelay_apply(void);
static int tweak_menushowdelay_revert(void);
static int tweak_dns_cache_apply(void);
static int tweak_dns_cache_revert(void);
static int tweak_large_system_cache_apply(void);
static int tweak_large_system_cache_revert(void);
static int tweak_tcp_autotuning_apply(void);
static int tweak_tcp_autotuning_revert(void);

/* ── Array de tweaks — adiciona aqui ─────────────────────────── */
static Tweak g_tweaks[] = {
    {
        1, "Game Mode",
        "Ativa o Game Mode do Windows para priorizar processos de jogo",
        CAT_PERFORMANCE, SAFE, STATE_OFF,
        tweak_game_mode_apply, tweak_game_mode_revert
    },
    {
        2, "Disable Nagle",
        "Desativa o algoritmo de Nagle para menor latencia de rede",
        CAT_NETWORK, SAFE, STATE_OFF,
        tweak_nagle_apply, tweak_nagle_revert
    },
    {
        3, "Disable Telemetry",
        "Remove servicos de telemetria e diagnostico da Microsoft",
        CAT_PRIVACY, SAFE, STATE_OFF,
        tweak_telemetry_apply, tweak_telemetry_revert
    },
    {
        4, "Timer Resolution",
        "Define resolucao do timer para minimo disponivel (menor latencia)",
        CAT_PERFORMANCE, EXTREME, STATE_OFF,
        tweak_timer_apply, tweak_timer_revert
    },
    {
        5, "Disable HPET",
        "Desativa High Precision Event Timer para menor DPC latency",
        CAT_PERFORMANCE, EXTREME, STATE_OFF,
        tweak_hpet_apply, tweak_hpet_revert
    },
    {
        6, "Disable Superfetch",
        "Desativa o Superfetch para melhorar o desempenho do sistema",
        CAT_PERFORMANCE, SAFE, STATE_OFF,
        tweak_superfetch_apply, tweak_superfetch_revert
    },
    {
        7, "Power Plan Ultra",
        "Ativa plano de energia de alta performance",
        CAT_PERFORMANCE, SAFE, STATE_OFF,
        tweak_powerplan_apply, tweak_powerplan_revert
    },
    {
        8, "Disable Xbox DVR",
        "Desativa captura de jogo em background (Game Bar)",
        CAT_PERFORMANCE, SAFE, STATE_OFF,
        tweak_xboxdvr_apply, tweak_xboxdvr_revert
    },
    {
        9, "Visual FX Clean",
        "Remove animacoes visuais desnecessarias do Explorer",
        CAT_PERFORMANCE, SAFE, STATE_OFF,
        tweak_visualfx_apply, tweak_visualfx_revert
    },
    {
        10, "Disable Cortana",
        "Desativa Cortana via politica de grupo",
        CAT_PRIVACY, SAFE, STATE_OFF,
        tweak_cortana_apply, tweak_cortana_revert
    },
    {
        11, "GPU Priority",
        "Define prioridade da GPU para foco em desempenho",
        CAT_PERFORMANCE, SAFE, STATE_OFF,
        tweak_gpu_priority_apply, tweak_gpu_priority_revert
    },
    {
        12, "Network Throttling",
        "Desativa limitacao de largura de banda do sistema",
        CAT_NETWORK, SAFE, STATE_OFF,
        tweak_network_throttling_apply, tweak_network_throttling_revert
    },
    {
        13, "Windows Update Pause",
        "Pausa atualizacoes do Windows por 365 dias",
        CAT_SERVICES, SAFE, STATE_OFF,
        tweak_windows_update_apply, tweak_windows_update_revert
    },
    {
        14, "Disable Startup Delay",
        "Remove delay de inicializacao de aplicativos no boot",
        CAT_PERFORMANCE, SAFE, STATE_OFF,
        tweak_startup_delay_apply, tweak_startup_delay_revert
    },
    {
        15, "Fast Menu Show",
        "Reduz delay de exibicao dos menus do Windows",
        CAT_VISUALS, SAFE, STATE_OFF,
        tweak_menushowdelay_apply, tweak_menushowdelay_revert
    },
    {
        16, "DNS Cache",
        "Aumenta cache DNS para respostas mais rapidas",
        CAT_NETWORK, SAFE, STATE_OFF,
        tweak_dns_cache_apply, tweak_dns_cache_revert
    },
    {
        17, "Large System Cache",
        "Aumenta cache do sistema para melhor desempenho",
        CAT_PERFORMANCE, EXTREME, STATE_OFF,
        tweak_large_system_cache_apply, tweak_large_system_cache_revert
    },
    {
        18, "TCP AutoTuning",
        "Otimiza parametros TCP para maior throughput",
        CAT_NETWORK, SAFE, STATE_OFF,
        tweak_tcp_autotuning_apply, tweak_tcp_autotuning_revert
    },
};

static int g_count = sizeof(g_tweaks) / sizeof(g_tweaks[0]);

/* ── API pública ─────────────────────────────────────────────── */

void tweaks_init(void)
{
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
    FILE *f = fopen(STATE_FILE, "w");
    if (!f) return;

    for (int i = 0; i < g_count; i++)
        fprintf(f, "%s=%d\n", g_tweaks[i].name, (int)g_tweaks[i].state);

    fclose(f);
}

void tweaks_load_state(void)
{
    FILE *f = fopen(STATE_FILE, "r");
    if (!f) return;  /* sem ficheiro = todos OFF, tudo bem */

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        /* formato: "Game Mode=1" */
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';  /* divide a string em nome + valor */
        int val = atoi(eq + 1);

        /* procura o tweak pelo nome */
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

/* ── Função auxiliar para executar comandos ──────────────────── */
static int run_command(const char *cmd)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return -1;

    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

/* ── Implementação dos tweaks ────────────────────────────────── */

static int tweak_game_mode_apply(void)
{
    RegResult r = reg_set_dword(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\GameBar",
        "AutoGameModeEnabled", 1
    );
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_game_mode_revert(void)
{
    RegResult r = reg_set_dword(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\GameBar",
        "AutoGameModeEnabled", 0);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_nagle_apply(void)
{
    const char *base = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces";
    HKEY hBase;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, base, 0, KEY_READ, &hBase) != ERROR_SUCCESS)
        return -1;

    char subkey[256];
    DWORD idx = 0;
    int errors = 0;

    while (RegEnumKeyA(hBase, idx++, subkey, sizeof(subkey)) == ERROR_SUCCESS) {
        char full[512];
        snprintf(full, sizeof(full), "%s\\%s", base, subkey);

        if (reg_set_dword(HKEY_LOCAL_MACHINE, full, "TcpAckFrequency", 1) != REG_OK)
            errors++;
        if (reg_set_dword(HKEY_LOCAL_MACHINE, full, "TCPNoDelay", 1) != REG_OK)
            errors++;
    }

    RegCloseKey(hBase);
    return errors == 0 ? 0 : -1;
}

static int tweak_nagle_revert(void)
{
    const char *base = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces";
    HKEY hBase;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, base, 0, KEY_READ, &hBase) != ERROR_SUCCESS)
        return -1;

    char subkey[256];
    DWORD idx = 0;

    while (RegEnumKeyA(hBase, idx++, subkey, sizeof(subkey)) == ERROR_SUCCESS) {
        char full[512];
        snprintf(full, sizeof(full), "%s\\%s", base, subkey);
        reg_delete_value(HKEY_LOCAL_MACHINE, full, "TcpAckFrequency");
        reg_delete_value(HKEY_LOCAL_MACHINE, full, "TCPNoDelay");
    }

    RegCloseKey(hBase);
    return 0;
}

static int tweak_telemetry_apply(void)
{
    int errors = 0;

    errors += (reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection",
        "AllowTelemetry", 0) != REG_OK);

    errors += (reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection",
        "AllowTelemetry", 0) != REG_OK);

    return errors == 0 ? 0 : -1;
}

static int tweak_telemetry_revert(void)
{
    reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection",
        "AllowTelemetry", 1);

    reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection",
        "AllowTelemetry", 1);

    return 0;
}

static int tweak_timer_apply(void)
{
    return run_command("bcdedit /set useplatformtick yes");
}

static int tweak_timer_revert(void)
{
    return run_command("bcdedit /deletevalue useplatformtick");
}

static int tweak_hpet_apply(void)
{
    /* Desativa HPET e Dynamic Tick */
    run_command("bcdedit /set useplatformclock false");
    return run_command("bcdedit /set disabledynamictick yes");
}

static int tweak_hpet_revert(void)
{
    run_command("bcdedit /deletevalue useplatformclock");
    return run_command("bcdedit /set disabledynamictick no");
}

static int tweak_superfetch_apply(void)
{
    run_command("sc stop SysMain");
    Sleep(1000);
    return run_command("sc config SysMain start= disabled");
}

static int tweak_superfetch_revert(void)
{
    run_command("sc config SysMain start= auto");
    Sleep(500);
    return run_command("sc start SysMain");
}

static int tweak_powerplan_apply(void)
{
    /* High Performance GUID */
    return run_command("powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
}

static int tweak_powerplan_revert(void)
{
    /* Balanced GUID */
    return run_command("powercfg /setactive 381b4222-f694-41f0-9685-ff5bb260df2e");
}

static int tweak_xboxdvr_apply(void)
{
    RegResult r = reg_set_dword(HKEY_CURRENT_USER,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameDVR",
        "AppCaptureEnabled", 0);
    
    if (r != REG_OK) return -1;
    
    r = reg_set_dword(HKEY_CURRENT_USER,
        "System\\GameConfigStore",
        "GameDVR_Enabled", 0);
    
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_xboxdvr_revert(void)
{
    reg_set_dword(HKEY_CURRENT_USER,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameDVR",
        "AppCaptureEnabled", 1);
    
    reg_set_dword(HKEY_CURRENT_USER,
        "System\\GameConfigStore",
        "GameDVR_Enabled", 1);
    
    return 0;
}

static int tweak_visualfx_apply(void)
{
    /* 2 = Adjust for best performance */
    RegResult r = reg_set_dword(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects",
        "VisualFXSetting", 2);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_visualfx_revert(void)
{
    /* 1 = Let Windows choose what's best */
    RegResult r = reg_set_dword(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects",
        "VisualFXSetting", 1);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_cortana_apply(void)
{
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Policies\\Microsoft\\Windows\\Windows Search",
        "AllowCortana", 0);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_cortana_revert(void)
{
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Policies\\Microsoft\\Windows\\Windows Search",
        "AllowCortana", 1);
    return (r == REG_OK) ? 0 : -1;
}

/* ── Novos tweaks ────────────────────────────────────────────── */

static int tweak_gpu_priority_apply(void)
{
    /* Define GPU scheduling para alta prioridade */
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
        "SystemResponsiveness", 0);
    
    if (r != REG_OK) return -1;
    
    r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks\\Games",
        "GPU Priority", 8);
    
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_gpu_priority_revert(void)
{
    reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
        "SystemResponsiveness", 20);
    
    reg_delete_value(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks\\Games",
        "GPU Priority");
    
    return 0;
}

static int tweak_network_throttling_apply(void)
{
    /* Desativa throttling de rede */
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
        "NetworkThrottlingIndex", 0xFFFFFFFF);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_network_throttling_revert(void)
{
    /* Valor padrão: 10 */
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
        "NetworkThrottlingIndex", 10);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_windows_update_apply(void)
{
    /* Pausa updates até 2035 */
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\WindowsUpdate\\UX\\Settings",
        "PauseUpdatesExpiryTime", 0x7FFFFFFF);
    
    if (r != REG_OK) return -1;
    
    /* Desativa updates automáticos */
    r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU",
        "NoAutoUpdate", 1);
    
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_windows_update_revert(void)
{
    reg_delete_value(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\WindowsUpdate\\UX\\Settings",
        "PauseUpdatesExpiryTime");
    
    reg_set_dword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU",
        "NoAutoUpdate", 0);
    
    return 0;
}

static int tweak_startup_delay_apply(void)
{
    /* Remove delay de inicialização */
    RegResult r = reg_set_dword(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Serialize",
        "StartupDelayInMSec", 0);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_startup_delay_revert(void)
{
    reg_delete_value(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Serialize",
        "StartupDelayInMSec");
    return 0;
}

static int tweak_menushowdelay_apply(void)
{
    /* Reduz delay do menu para 0ms */
    RegResult r = reg_set_dword(HKEY_CURRENT_USER,
        "Control Panel\\Desktop",
        "MenuShowDelay", 0);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_menushowdelay_revert(void)
{
    /* Valor padrão: 400ms */
    RegResult r = reg_set_dword(HKEY_CURRENT_USER,
        "Control Panel\\Desktop",
        "MenuShowDelay", 400);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_dns_cache_apply(void)
{
    int errors = 0;
    
    errors += (reg_set_dword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\Dnscache\\Parameters",
        "MaxCacheTtl", 86400) != REG_OK);
    
    errors += (reg_set_dword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\Dnscache\\Parameters",
        "MaxNegativeCacheTtl", 0) != REG_OK);
    
    return errors == 0 ? 0 : -1;
}

static int tweak_dns_cache_revert(void)
{
    reg_delete_value(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\Dnscache\\Parameters",
        "MaxCacheTtl");
    
    reg_delete_value(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\Dnscache\\Parameters",
        "MaxNegativeCacheTtl");
    
    return 0;
}

static int tweak_large_system_cache_apply(void)
{
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        "LargeSystemCache", 1);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_large_system_cache_revert(void)
{
    RegResult r = reg_set_dword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        "LargeSystemCache", 0);
    return (r == REG_OK) ? 0 : -1;
}

static int tweak_tcp_autotuning_apply(void)
{
    return run_command("netsh int tcp set global autotuninglevel=normal");
}

static int tweak_tcp_autotuning_revert(void)
{
    return run_command("netsh int tcp set global autotuninglevel=normal");
}