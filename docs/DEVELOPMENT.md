# VOIDTUNE One-Click: development

Source and docs are ASCII only. CI fails on any byte above 0x7F.

## Build and test

```sh
make          # builds for the current OS
make test     # Linux only
```

The Windows build is checked in CI with mingw-w64 (`windows-cross` job). It is not exercised by `make test`.

## Structure

`src/tweaks.c` is the engine. It asks the platform for a table of `Tweak` structs through `platform_tweaks()` and handles apply, revert and the state file. Everything OS-specific is behind `src/platform.h` and `src/sysinfo.h`.

| Concern | Windows | Linux |
|---|---|---|
| Tweak table | `win/tweaks_win.c` | `linux/tweaks_linux.c` |
| Console, state path | `win/platform_win.c` | `linux/platform_linux.c` |
| System info | `win/sysinfo_win.c` | `linux/sysinfo_linux.c`, `linux/distro.c` |
| Backup, services | `win/backup.c`, `win/services.c` | n/a |

`services.c` is compiled but has no menu entry yet.

## Return values

`apply()` and `revert()` return `0` on success, `TWEAK_NA` (1) when the tweak does not apply to this system, and `-1` on failure. The engine only changes state on `0`.

## Adding a Windows tweak

In `win/tweaks_win.c`: declare `tweak_x_apply/revert`, add an entry to `g_tweaks[]`, implement both with `reg_set_dword`, `reg_delete_value` or `run_command`.

## Adding a Linux sysctl tweak

One line in `linux/tweaks_linux.c`, plus a table entry:

```c
SYSCTL_TWEAK(my_tweak, { "vm.some_key", "42" })
```

This generates `my_tweak_apply` and `my_tweak_revert`. The tweak is `n/a` if the key is missing from `/proc/sys`. The first original value is saved under `<state dir>/prev/` and is never overwritten by a re-apply.

## Adding a distro

Add the `ID` to the map in `linux/distro.c` and a case to `tests/test_distro.c`. Match by `ID` as well as `ID_LIKE`: some distros omit `ID_LIKE`.

## Test hooks (Linux)

`VOIDTUNE_DRY_RUN`, `VOIDTUNE_STATE_DIR`, `VOIDTUNE_SYSCTL_DIR`, `VOIDTUNE_PROC_DIR`, `VOIDTUNE_CPU_DIR`. See the header of `linux/tweaks_linux.c`. `tests/test_linux_tweaks.sh` builds fake trees and drives the real binary.

## Known limits

- TCP Auto-Tuning Reset (Windows) always reverts to `restricted`, not the prior level.
- CPU governor (Linux) is not persistent across reboots.
- Core count on Linux assumes one CPU socket.
