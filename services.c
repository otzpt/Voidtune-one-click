/**
 * services.c — Gestão de serviços do Windows
 * Porta de modules/data.ps1 ($SVCS_DATA) do VOIDTUNE v0.8V
 * Copyright (C) 2026 @OTZPT — GPL v3
 */

#include "services.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

/* ── Lista de serviços — espelha $SVCS_DATA do PowerShell ─────── */
static const struct {
    const char *name;
    const char *desc;
    int         careful;   /* 1 = mostrar aviso extra */
} g_svcs[] = {
    /* Telemetria */
    { "DiagTrack",          "Windows Telemetry — safe to disable",              0 },
    { "dmwappushservice",   "WAP Push Routing — telemetry relay",               0 },
    /* Prefetch / cache */
    { "SysMain",            "Superfetch — useless on SSD",                      0 },
    /* Indexação */
    { "WSearch",            "Windows Search indexing — kills HDD I/O",          0 },
    /* Impressão */
    { "Spooler",            "Print Spooler — disable if no printer",            0 },
    /* Segurança */
    { "RemoteRegistry",     "Remote Registry — security risk",                  0 },
    /* Interface */
    { "TabletInputService", "Tablet PC Input — irrelevant on desktop",          0 },
    /* Legacy */
    { "Fax",                "Fax service — nobody uses this",                   0 },
    { "MapsBroker",         "Offline Maps — unused on most PCs",                0 },
    { "RetailDemo",         "Store Display Mode — only for retail PCs",         0 },
    { "WMPNetworkSvc",      "Windows Media Player sharing",                     0 },
    /* Tempo */
    { "W32Time",            "Time sync — safe to disable offline",              0 },
    /* Bluetooth */
    { "bthserv",            "Bluetooth — disable if no BT devices",             0 },
    /* Windows Update relay */
    { "DoSvc",              "Delivery Optimization — P2P Update relay",         0 },
    { "BITS",               "Background Intelligent Transfer",                  0 },
    /* Biometria */
    { "WbioSrvc",           "Windows Biometric — disable if no fingerprint",    0 },
    /* Sensores */
    { "SensorService",      "Sensor Aggregator — disable on desktop",           0 },
    { "SensrSvc",           "Sensor Monitoring — disable on desktop",           0 },
    /* Xbox */
    { "XboxGip",            "Xbox Game Input Protocol",                         0 },
    { "XblAuthManager",     "Xbox Live Authentication",                         0 },
    { "XblGameSave",        "Xbox Game Save sync",                              0 },
    { "XboxNetApiSvc",      "Xbox Network API",                                 0 },
    /* Cuidado */
    { "wuauserv",           "Windows Update — disable with caution",            1 },
};

static int g_count = (int)(sizeof(g_svcs) / sizeof(g_svcs[0]));

/* ── Auxiliar: corre cmd.exe /c e retorna exit code ─────────── */
static int run_cmd(const char *cmd)
{
    char full[512];
    snprintf(full, sizeof(full), "cmd.exe /c %s", cmd);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, full, NULL, NULL, FALSE,
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return -1;

    DWORD exit_code = 1;
    WaitForSingleObject(pi.hProcess, 8000);
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)exit_code;
}

/* ── Consulta SCM ────────────────────────────────────────────── */
static ServiceState query_state(const char *name)
{
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return SVC_UNKNOWN;

    SC_HANDLE svc = OpenServiceA(scm, name,
                                 SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (!svc) {
        CloseServiceHandle(scm);
        return SVC_UNKNOWN;
    }

    /* Estado de execução */
    SERVICE_STATUS ss;
    BOOL running = FALSE;
    if (QueryServiceStatus(svc, &ss))
        running = (ss.dwCurrentState == SERVICE_RUNNING);

    /* Tipo de arranque */
    BYTE cfgbuf[4096];
    DWORD needed = 0;
    QUERY_SERVICE_CONFIGA *cfg = (QUERY_SERVICE_CONFIGA *)cfgbuf;
    QueryServiceConfigA(svc, cfg, sizeof(cfgbuf), &needed);

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);

    if (cfg->dwStartType == SERVICE_DISABLED)
        return SVC_DISABLED;
    if (running)
        return SVC_RUNNING;
    if (cfg->dwStartType == SERVICE_AUTO_START ||
        cfg->dwStartType == SERVICE_BOOT_START)
        return SVC_AUTO;
    if (cfg->dwStartType == SERVICE_DEMAND_START)
        return SVC_MANUAL;

    return SVC_STOPPED;
}

/* ── API pública ─────────────────────────────────────────────── */

int services_count(void) { return g_count; }

const char *services_get_name(int i)
{
    if (i < 0 || i >= g_count) return "";
    return g_svcs[i].name;
}

const char *services_get_desc(int i)
{
    if (i < 0 || i >= g_count) return "";
    return g_svcs[i].desc;
}

ServiceState services_get_state(int i)
{
    if (i < 0 || i >= g_count) return SVC_UNKNOWN;
    return query_state(g_svcs[i].name);
}

int services_disable(int i)
{
    if (i < 0 || i >= g_count) return -1;
    const char *n = g_svcs[i].name;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sc config %s start= disabled & sc stop %s", n, n);
    return run_cmd(cmd);
}

int services_enable(int i)
{
    if (i < 0 || i >= g_count) return -1;
    const char *n = g_svcs[i].name;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sc config %s start= auto & sc start %s", n, n);
    return run_cmd(cmd);
}

const char *service_state_str(ServiceState s)
{
    switch (s) {
        case SVC_RUNNING:  return "RUNNING ";
        case SVC_STOPPED:  return "STOPPED ";
        case SVC_AUTO:     return "AUTO    ";
        case SVC_DISABLED: return "DISABLED";
        case SVC_MANUAL:   return "MANUAL  ";
        default:           return "UNKNOWN ";
    }
}

void services_print_all(void)
{
    printf("  \033[38;5;240m%-4s %-22s %-10s %s\033[0m\n",
           "#", "SERVICO", "ESTADO", "DESCRICAO");
    printf("  \033[38;5;240m");
    for (int i = 0; i < 60; i++) printf("-");
    printf("\033[0m\n");

    for (int i = 0; i < g_count; i++) {
        ServiceState st = query_state(g_svcs[i].name);

        /* Cor do estado */
        const char *col;
        switch (st) {
            case SVC_RUNNING:  col = "\033[32m";         break;  /* verde */
            case SVC_DISABLED: col = "\033[38;5;240m";   break;  /* cinzento */
            case SVC_AUTO:     col = "\033[33m";          break;  /* amarelo */
            default:           col = "\033[37m";          break;  /* branco */
        }

        /* Aviso "careful" a vermelho */
        const char *nc = g_svcs[i].careful ? "\033[31m" : "\033[37m";

        printf("  %s[%2d]\033[0m %-22s %s%-10s\033[0m %s\n",
               nc, i + 1,
               g_svcs[i].name,
               col, service_state_str(st),
               g_svcs[i].desc);
    }
}
