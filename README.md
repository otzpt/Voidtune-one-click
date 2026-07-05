# VOIDTUNE — One-Click Optimizer (native C edition)

**The tiny, zero-dependency automatic optimizer.** One `.exe`, ~170 KB, no runtime, no install. Run it, hit **Optimize Now**, done.

| | |
|---|---|
| **Size** | ~170 KB single executable |
| **Dependencies** | none — pure Win32 (`advapi32`, `ole32`) |
| **Runs on** | Windows 7 → 11, WinPE, Server, stripped installs |
| **License** | GPL v3 |
| **Sibling** | the full GUI flagship is the **C# / WinUI 3** edition |

## Why it exists
The flagship VOIDTUNE (C# / WinUI 3) is a rich GUI with DevTools, live monitors and ~150 tweaks — but it's ~60 MB and needs the Windows App SDK. This edition is the opposite end: a **set-and-forget one-click optimizer** that fits on a floppy and runs anywhere. Same philosophy — every tweak reversible — in a native binary.

## Use
1. Run `voidtune.exe` **as Administrator**.
2. **[1] OTIMIZAR AGORA** — applies every **SAFE** tweak in one pass (fully reversible).
3. **[2] Tweaks** — toggle individual tweaks by hand (SAFE + EXTREME).
4. **[3] System Info** — CPU / RAM / OS + how many tweaks are active.
5. Reboot for changes that need it.

State is saved to `voidtune_state.txt`; a registry backup is written before changes.

## Build
Requires MSYS2 UCRT64 (`pacman -S mingw-w64-ucrt-x86_64-gcc`):

```sh
make            # build voidtune.exe
make run        # build + run
make release    # build + strip (smallest binary)
make clean      # remove artifacts
```

Sources: `main.c` (CLI), `tweaks.c` (engine), `registry.c`, `services.c`, `sysinfo.c`, `backup.c`.

---
Part of the **VOIDTUNE** ecosystem · [@OTZPT](https://github.com/otzpt) · Discord: https://discord.gg/vAWqWD2e6R
