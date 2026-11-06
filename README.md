# VOIDTUNE One-Click (native C edition)

Tiny, zero-dependency automatic optimizer for **Windows** and **Linux**. Run it, pick **Optimize Now**, done. Every tweak is reversible.

| | |
|---|---|
| **Windows** | one `.exe`, pure Win32 (`advapi32`, `ole32`), Windows 7 to 11 |
| **Linux** | one binary, libc only. Supported distros: Void, Arch, CachyOS, Ubuntu, Linux Mint, Zorin OS (others work, flagged "untested") |
| **License** | GPL v3 |
| **Sibling** | the full GUI flagship is the C# / WinUI 3 edition ([VOIDTUNE](https://github.com/otzpt/VOIDTUNE), Windows only) |

## Use

Windows: run `voidtune.exe` as Administrator.
Linux: `sudo ./voidtune`

1. **[1] OPTIMIZE NOW** applies every SAFE tweak in one pass. Tweaks that do not apply to the machine are reported as `n/a`, not as failures.
2. **[2] Tweaks** toggles individual tweaks (SAFE and EXTREME).
3. **[3] System Information** shows OS, CPU, RAM and, on Linux, the detected distro family and init system.
4. **[4] Revert all tweaks** restores every active tweak.

Before one-click on Windows, a registry backup (`backup_*.reg`) is written to the current folder. State is saved to `voidtune_state.txt` (Windows) or `/var/lib/voidtune/voidtune_state.txt` (Linux).

## Linux tweaks

| Tweak | Level | What it does |
|---|---|---|
| Low Swappiness | SAFE | `vm.swappiness=10` |
| VFS Cache Pressure | SAFE | `vm.vfs_cache_pressure=50` |
| Max Map Count | SAFE | `vm.max_map_count=2147483642` |
| TCP BBR | SAFE | `fq` + `bbr` (n/a if the kernel has no `tcp_bbr`) |
| TCP Fast Open | SAFE | `net.ipv4.tcp_fastopen=3` |
| TCP Buffers | SAFE | `rmem_max`/`wmem_max` = 16 MB |
| Disable Error Reports | SAFE | stops `whoopsie` (Ubuntu family, systemd only) |
| Weekly SSD TRIM | SAFE | enables `fstrim.timer` if present and off (systemd only) |
| CPU Performance Governor | EXTREME | `performance` on all cores, until reboot |
| Disable NMI Watchdog | EXTREME | `kernel.nmi_watchdog=0` |

Sysctl tweaks apply live and write `/etc/sysctl.d/99-voidtune-<name>.conf` so they persist. systemd-sysctl and Void's runit stage 1 both read that directory. The previous value of each setting is saved in `/var/lib/voidtune/prev/` before the first change, and revert restores it.

Not touched: bootloader, kernel parameters, packages, drivers, DNS, display manager.

Verification so far: the tweak engine was run on Void Linux (dry run on the real system, apply/revert against fake `/proc` trees). The other distros are covered by the os-release parser test only, not by a run on each.

Dry run (no changes, works without root):

```sh
VOIDTUNE_DRY_RUN=1 ./voidtune
```

## Build

Windows (MSYS2 UCRT64, `pacman -S mingw-w64-ucrt-x86_64-gcc make`):

```sh
make            # voidtune.exe
make release    # stripped
```

Linux (gcc and make from your distro: `build-essential`, `base-devel`, or `base-devel` on Void):

```sh
make            # ./voidtune
make test       # distro parser + apply/revert against fake /proc and /sys trees
make release    # stripped
```

## Layout

```
src/main.c, tweaks.c       menus and the platform-independent tweak engine
src/tweaks.h, platform.h   tweak struct and the per-OS interface
src/sysinfo.h
src/win/                   registry, services, backup, Windows tweaks
src/linux/                 sysctl/sysfs/systemd tweaks, distro detection
tests/                     Linux checks
docs/DEVELOPMENT.md        how to add a tweak or a distro
```

---
Part of the **VOIDTUNE** ecosystem - [@OTZPT](https://github.com/otzpt) - Discord: https://discord.gg/vAWqWD2e6R
