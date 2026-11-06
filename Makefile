# VOIDTUNE One-Click - Makefile
# Copyright (C) 2026 @OTZPT - GPL v3
#
#   make            build for the current OS (voidtune / voidtune.exe)
#   make test       Linux only: distro parser + tweak engine checks
#   make release    build + strip
#   make clean
#
# Windows: MSYS2 UCRT64 with  pacman -S mingw-w64-ucrt-x86_64-gcc make
# Linux:   gcc and make from your distro (build-essential, base-devel,
#          or xbps-install base-devel)

CC      = gcc
CFLAGS  = -Wall -Wextra -Wno-unused-parameter -O2 -std=c11
COMMON  = src/main.c src/tweaks.c

ifeq ($(OS),Windows_NT)
TARGET  = voidtune.exe
SRCS    = $(COMMON) src/win/platform_win.c src/win/tweaks_win.c \
          src/win/registry.c src/win/sysinfo_win.c src/win/backup.c \
          src/win/services.c
LDFLAGS = -ladvapi32 -lole32
RM      = del /Q
else
TARGET  = voidtune
SRCS    = $(COMMON) src/linux/platform_linux.c src/linux/tweaks_linux.c \
          src/linux/sysinfo_linux.c src/linux/distro.c
LDFLAGS =
RM      = rm -f
CFLAGS += -D_POSIX_C_SOURCE=200809L
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

release: $(TARGET)
	strip $(TARGET)

run: $(TARGET)
	./$(TARGET)

ifneq ($(OS),Windows_NT)
test: $(TARGET) tests/test_distro
	./tests/test_distro
	sh tests/test_linux_tweaks.sh ./$(TARGET)

tests/test_distro: tests/test_distro.c src/linux/distro.c
	$(CC) $(CFLAGS) -Isrc/linux $^ -o $@
endif

clean:
	$(RM) $(TARGET) tests/test_distro backup_*.reg voidtune_state.txt

.PHONY: all release run test clean
