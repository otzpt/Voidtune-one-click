/**
 * distro.c - os-release parsing
 */

#include "distro.h"
#include <stdio.h>
#include <string.h>

static DistroFamily classify(const char *tok)
{
    static const struct { const char *name; DistroFamily fam; } map[] = {
        { "void",      FAM_VOID   },
        { "arch",      FAM_ARCH   },
        { "cachyos",   FAM_ARCH   },
        { "debian",    FAM_DEBIAN },
        { "ubuntu",    FAM_DEBIAN },
        { "linuxmint", FAM_DEBIAN },
        { "zorin",     FAM_DEBIAN },
    };
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (strcmp(tok, map[i].name) == 0) return map[i].fam;
    return FAM_UNKNOWN;
}

/* Returns the family of the first recognised token in a space list. */
static DistroFamily classify_list(const char *list)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", list);
    for (char *tok = strtok(buf, " "); tok; tok = strtok(NULL, " ")) {
        DistroFamily f = classify(tok);
        if (f != FAM_UNKNOWN) return f;
    }
    return FAM_UNKNOWN;
}

/* Copies the value of "KEY=value" (quotes stripped) into dst. */
static void take_value(const char *val, char *dst, size_t n)
{
    if (*val == '"' || *val == '\'') val++;
    snprintf(dst, n, "%s", val);
    size_t len = strlen(dst);
    while (len && (dst[len - 1] == '\n' || dst[len - 1] == '\r' ||
                   dst[len - 1] == '"'  || dst[len - 1] == '\''))
        dst[--len] = '\0';
}

void distro_parse(const char *text, Distro *out)
{
    char id_like[256] = "";
    memset(out, 0, sizeof(*out));

    char line[512];
    const char *p = text;
    while (*p) {
        size_t n = strcspn(p, "\n");
        snprintf(line, sizeof(line), "%.*s", (int)n, p);
        p += n + (p[n] == '\n');

        if      (strncmp(line, "ID=", 3) == 0)          take_value(line + 3, out->id, sizeof(out->id));
        else if (strncmp(line, "ID_LIKE=", 8) == 0)     take_value(line + 8, id_like, sizeof(id_like));
        else if (strncmp(line, "PRETTY_NAME=", 12) == 0) take_value(line + 12, out->pretty, sizeof(out->pretty));
    }

    out->family = classify_list(out->id);
    if (out->family == FAM_UNKNOWN) out->family = classify_list(id_like);
    if (!out->pretty[0]) snprintf(out->pretty, sizeof(out->pretty), "%s", out->id[0] ? out->id : "Linux");
}

void distro_detect(Distro *out)
{
    static const char *paths[] = { "/etc/os-release", "/usr/lib/os-release" };
    char text[4096];

    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        FILE *f = fopen(paths[i], "r");
        if (!f) continue;
        size_t n = fread(text, 1, sizeof(text) - 1, f);
        fclose(f);
        text[n] = '\0';
        distro_parse(text, out);
        return;
    }
    memset(out, 0, sizeof(*out));
    snprintf(out->pretty, sizeof(out->pretty), "Linux");
}

const char *distro_family_str(DistroFamily f)
{
    switch (f) {
        case FAM_VOID:   return "void";
        case FAM_ARCH:   return "arch";
        case FAM_DEBIAN: return "debian";
        default:         return "unknown";
    }
}
