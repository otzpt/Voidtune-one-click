#!/bin/sh
# Drives the real binary against fake /proc/sys, /sys and /etc/sysctl.d
# trees. Checks apply, persistence file, saved values, and revert.
set -eu
BIN=$(realpath "${1:-./voidtune}")
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

mkdir -p "$T/proc/vm" "$T/proc/net/core" "$T/proc/net/ipv4" "$T/proc/kernel" \
         "$T/sysctl.d" "$T/state" "$T/cpu/cpu0/cpufreq" "$T/cpu/cpu1/cpufreq"
echo 60     > "$T/proc/vm/swappiness"
echo 100    > "$T/proc/vm/vfs_cache_pressure"
echo 65530  > "$T/proc/vm/max_map_count"
echo pfifo_fast > "$T/proc/net/core/default_qdisc"
echo cubic  > "$T/proc/net/ipv4/tcp_congestion_control"
echo 1      > "$T/proc/net/ipv4/tcp_fastopen"
echo 212992 > "$T/proc/net/core/rmem_max"
echo 212992 > "$T/proc/net/core/wmem_max"
echo schedutil > "$T/cpu/cpu0/cpufreq/scaling_governor"
echo schedutil > "$T/cpu/cpu1/cpufreq/scaling_governor"
# kernel.nmi_watchdog absent on purpose: must report n/a, not fail.

export VOIDTUNE_PROC_DIR="$T/proc" VOIDTUNE_SYSCTL_DIR="$T/sysctl.d" \
       VOIDTUNE_STATE_DIR="$T/state" VOIDTUNE_CPU_DIR="$T/cpu"

fail() { echo "FAIL: $1"; exit 1; }
val()  { cat "$T/$1"; }

# Option 1 = one-click (SAFE only), y = confirm, blank = back, 0 = exit.
printf '1\ny\n\n0\n' | "$BIN" >"$T/out1.txt"
[ "$(val proc/vm/swappiness)" = 10 ]                  || fail "swappiness not applied"
[ "$(val proc/net/ipv4/tcp_congestion_control)" = bbr ] || fail "bbr not applied"
[ "$(val proc/net/core/rmem_max)" = 16777216 ]        || fail "rmem_max not applied"
grep -q 'vm.swappiness = 10' "$T/sysctl.d/99-voidtune-swappiness.conf" || fail "no sysctl.d drop-in"
[ "$(val state/prev/swappiness.vm.swappiness)" = 60 ] || fail "previous value not saved"
[ "$(val cpu/cpu0/cpufreq/scaling_governor)" = schedutil ] || fail "EXTREME governor applied by one-click"

# Second run must keep the first saved original, not the applied value.
printf '1\ny\n\n0\n' | "$BIN" >"$T/out2.txt"
[ "$(val state/prev/swappiness.vm.swappiness)" = 60 ] || fail "original overwritten on re-apply"

# Option 4 = revert all.
printf '4\ny\n\n0\n' | "$BIN" >"$T/out3.txt"
[ "$(val proc/vm/swappiness)" = 60 ]                  || fail "swappiness not restored"
[ "$(val proc/net/ipv4/tcp_congestion_control)" = cubic ] || fail "congestion control not restored"
[ "$(val proc/net/core/rmem_max)" = 212992 ]          || fail "rmem_max not restored"
[ ! -e "$T/sysctl.d/99-voidtune-swappiness.conf" ]    || fail "drop-in not removed"

echo "linux tweaks: ok"
