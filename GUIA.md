# VOIDTUNE C — Guia de Desenvolvimento

## Compilar
```bash
cd "/c/Users/mrjs1/OneDrive/Documentos/my optimizer/VOIDTUNE C edition"
mingw32-make
./voidtune.exe
```

---

## Adicionar um tweak novo

### 1. Declara as funções no topo de `tweaks.c`
```c
static int tweak_superfetch_apply(void);
static int tweak_superfetch_revert(void);
```

### 2. Adiciona a entrada no array `g_tweaks[]`
```c
{
    6, "Disable SuperFetch",
    "Desativa SysMain para reduzir uso de disco em background",
    CAT_PERFORMANCE, SAFE, STATE_OFF,
    tweak_superfetch_apply, tweak_superfetch_revert
},
```

### 3. Implementa as funções
```c
static int tweak_superfetch_apply(void)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};

    CreateProcessA(NULL, "sc stop SysMain",
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);

    si = (STARTUPINFOA){ sizeof(si) }; memset(&pi, 0, sizeof(pi));

    CreateProcessA(NULL, "sc config SysMain start= disabled",
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    return 0;
}

static int tweak_superfetch_revert(void)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};

    CreateProcessA(NULL, "sc config SysMain start= auto",
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);

    si = (STARTUPINFOA){ sizeof(si) }; memset(&pi, 0, sizeof(pi));

    CreateProcessA(NULL, "sc start SysMain",
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    return 0;
}
```

---

## Padrões que já existem no projeto

### Tweak via Registry
```c
static int tweak_xxx_apply(void)
{
    RegResult r = reg_set_dword(
        HKEY_LOCAL_MACHINE,   // ou HKEY_CURRENT_USER
        "caminho\\da\\chave",
        "NomeDoValor", 1
    );
    return (r == REG_OK) ? 0 : -1;
}
```

### Tweak via comando externo
```c
static int tweak_xxx_apply(void)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcessA(NULL, "comando aqui",
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return -1;

    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
```

### Múltiplos comandos no mesmo tweak
```c
// Reinicializa si e pi entre cada CreateProcessA
si = (STARTUPINFOA){ sizeof(si) };
memset(&pi, 0, sizeof(pi));
CreateProcessA(NULL, "segundo comando", ...);
```

---

## Tweaks a portar do PowerShell

| Tweak | Método | Comando / Chave |
|-------|--------|-----------------|
| Disable SuperFetch | sc.exe | `sc stop SysMain` + `sc config SysMain start= disabled` |
| Power Plan Ultra | powercfg | `powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c` |
| Disable Xbox DVR | Registry HKCU | `SOFTWARE\Microsoft\GameBar` → `AppCaptureEnabled=0` |
| Visual FX Clean | Registry HKCU | `SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects` → `VisualFXSetting=2` |
| Disable Cortana | Registry HKLM | `SOFTWARE\Policies\Microsoft\Windows\Windows Search` → `AllowCortana=0` |
| MSI Mode | Registry HKLM | `SYSTEM\CurrentControlSet\Enum\...\Device Parameters\Interrupt Management\MessageSignaledInterruptProperties` → `MSISupported=1` |

---

## Estrutura de ficheiros

```
voidtune/
├── main.c       ← menu CLI — não tocar muito por agora
├── tweaks.h     ← struct Tweak e enums — lê antes de editar
├── tweaks.c     ← AQUI vais trabalhar maioritariamente
├── registry.h   ← API registry — não precisas de editar
├── registry.c   ← implementação — não precisas de editar
├── sysinfo.h    ← struct SysInfo
├── sysinfo.c    ← leitura OS/CPU/RAM
└── Makefile     ← compilação
```

---

## Erros comuns

| Erro | Causa | Fix |
|------|-------|-----|
| `HKLM undeclared` | Abreviatura PS, não existe em C | Substituir por `HKEY_LOCAL_MACHINE` |
| `tweaks_apply` retorna -1 | Sem permissão | Correr como Administrador |
| `make: command not found` | MSYS2 sem make | Usar `mingw32-make` |
| Tweak não guarda estado | — | Já é automático em `tweaks_apply/revert` |

---

## Próximos passos por ordem

1. Porta 3-4 tweaks do PS para `tweaks.c` usando os padrões acima
2. Adiciona categoria nova ao enum `TweakCategory` em `tweaks.h` se precisares
3. Adiciona info de GPU ao `sysinfo.c` via registry
4. Filtragem por categoria no menu CLI
5. GUI — só depois de 15-20 tweaks portados e lógica sólida

---

## Contexto para conversa nova com o Claude

> "Estou a desenvolver o VOIDTUNE C Edition, reescrita em C do meu otimizador Windows.
> Estrutura: main.c (CLI), tweaks.c/.h (motor com apply/revert por ponteiros de função),
> registry.c/.h (abstração Win32), sysinfo.c/.h (CPU/RAM/OS).
> Compila com mingw32-make no MSYS2 UCRT64.
> Tenho X tweaks e quero agora [o que precisas]."
