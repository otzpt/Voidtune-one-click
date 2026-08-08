# VOIDTUNE C — Development Guide

## Build
```bash
cd "/c/path/to/VOIDTUNE C edition"
mingw32-make
./voidtune.exe
```

---

## Adding a new tweak

### 1. Declare the functions at the top of `tweaks.c`
```c
static int tweak_superfetch_apply(void);
static int tweak_superfetch_revert(void);
```

### 2. Add the entry to the `g_tweaks[]` array
```c
{
    6, "Disable SuperFetch",
    "Disables SysMain to reduce background disk usage",
    CAT_PERFORMANCE, SAFE, STATE_OFF,
    tweak_superfetch_apply, tweak_superfetch_revert
},
```

### 3. Implement the functions
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

## Patterns already used in the project

### Registry-based tweak
```c
static int tweak_xxx_apply(void)
{
    RegResult r = reg_set_dword(
        HKEY_LOCAL_MACHINE,   // or HKEY_CURRENT_USER
        "path\\to\\key",
        "ValueName", 1
    );
    return (r == REG_OK) ? 0 : -1;
}
```

### External-command tweak
```c
static int tweak_xxx_apply(void)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcessA(NULL, "command here",
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return -1;

    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
```

### Multiple commands in the same tweak
```c
// Reset si and pi between each CreateProcessA call
si = (STARTUPINFOA){ sizeof(si) };
memset(&pi, 0, sizeof(pi));
CreateProcessA(NULL, "second command", ...);
```

---

## Tweaks still to port from PowerShell

| Tweak | Method | Command / Key |
|-------|--------|-----------------|
| Disable SuperFetch | sc.exe | `sc stop SysMain` + `sc config SysMain start= disabled` |
| Power Plan Ultra | powercfg | `powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c` |
| Disable Xbox DVR | Registry HKCU | `SOFTWARE\Microsoft\GameBar` → `AppCaptureEnabled=0` |
| Visual FX Clean | Registry HKCU | `SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects` → `VisualFXSetting=2` |
| Disable Cortana | Registry HKLM | `SOFTWARE\Policies\Microsoft\Windows\Windows Search` → `AllowCortana=0` |
| MSI Mode | Registry HKLM | `SYSTEM\CurrentControlSet\Enum\...\Device Parameters\Interrupt Management\MessageSignaledInterruptProperties` → `MSISupported=1` |

---

## File layout

```
voidtune/
├── main.c       ← CLI menu — don't touch much for now
├── tweaks.h     ← Tweak struct and enums — read before editing
├── tweaks.c     ← MOST of your work happens here
├── registry.h   ← registry API — shouldn't need to edit
├── registry.c   ← implementation — shouldn't need to edit
├── sysinfo.h    ← SysInfo struct
├── sysinfo.c    ← OS/CPU/RAM readout
└── Makefile     ← build rules
```

---

## Common errors

| Error | Cause | Fix |
|------|-------|-----|
| `HKLM undeclared` | PowerShell abbreviation, doesn't exist in C | Use `HKEY_LOCAL_MACHINE` instead |
| `tweaks_apply` returns -1 | No permission | Run as Administrator |
| `make: command not found` | MSYS2 without make | Use `mingw32-make` |
| Tweak doesn't save state | — | Already automatic in `tweaks_apply/revert` |

---

## Next steps, in order

1. Port 3-4 tweaks from PowerShell to `tweaks.c` using the patterns above
2. Add a new category to the `TweakCategory` enum in `tweaks.h` if needed
3. Add GPU info to `sysinfo.c` via the registry
4. Category filtering in the CLI menu
5. GUI — only once 15-20 tweaks are ported and the logic is solid
