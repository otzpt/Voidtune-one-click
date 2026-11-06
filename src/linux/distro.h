/**
 * distro.h - Linux distro detection from os-release
 *
 * Detection reads ID and ID_LIKE. CachyOS and Zorin are matched by ID as
 * well, because ID_LIKE has been missing or inconsistent on some installs.
 */

#ifndef DISTRO_H
#define DISTRO_H

typedef enum {
    FAM_UNKNOWN = 0,
    FAM_VOID,       /* Void Linux (runit, xbps) */
    FAM_ARCH,       /* Arch, CachyOS (pacman) */
    FAM_DEBIAN      /* Ubuntu, Linux Mint, Zorin OS (apt) */
} DistroFamily;

typedef struct {
    char         id[64];
    char         pretty[128];
    DistroFamily family;
} Distro;

/* Parses the text of an os-release file. */
void distro_parse(const char *text, Distro *out);

/* Reads /etc/os-release (or /usr/lib/os-release) and parses it. */
void distro_detect(Distro *out);

const char *distro_family_str(DistroFamily f);

#endif /* DISTRO_H */
