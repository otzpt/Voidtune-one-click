# VOIDTUNE — One-Click Optimizer (native C edition) — Makefile
# Copyright (C) 2026 @OTZPT — GPL v3
#
# Build:   make
# Clean:   make clean
# Run:     make run
#
# Requires MSYS2 UCRT64:
#   pacman -S mingw-w64-ucrt-x86_64-gcc

CC      = gcc
CFLAGS  = -Wall -Wextra -Wno-unused-parameter -O2 -std=c11
LDFLAGS = -ladvapi32 -lole32
TARGET  = voidtune.exe
SRCS    = main.c tweaks.c registry.c sysinfo.c backup.c services.c
OBJS    = $(SRCS:.c=.o)

# ── Rules ────────────────────────────────────────────────────────

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "  Built: $(TARGET)"
	@echo ""

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o $(TARGET) backup_*.reg voidtune_state.txt 2>nul || \
	rm -f *.o $(TARGET) backup_*.reg voidtune_state.txt

run: $(TARGET)
	./$(TARGET)

# Strips the final executable (saves ~500KB)
release: $(TARGET)
	strip $(TARGET)
	@echo "  Release: $(TARGET) (stripped)"

.PHONY: all clean run release
