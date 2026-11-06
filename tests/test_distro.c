/* Checks os-release classification for the supported distros. */
#include "distro.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void expect(const char *text, DistroFamily fam, const char *pretty)
{
    Distro d;
    distro_parse(text, &d);
    assert(d.family == fam);
    assert(strcmp(d.pretty, pretty) == 0);
}

int main(void)
{
    expect("NAME=\"Void\"\nID=\"void\"\nPRETTY_NAME=\"Void Linux\"\n",
           FAM_VOID, "Void Linux");
    expect("NAME=\"Arch Linux\"\nPRETTY_NAME=\"Arch Linux\"\nID=arch\n",
           FAM_ARCH, "Arch Linux");
    /* CachyOS: matched by ID even when ID_LIKE is absent */
    expect("NAME=\"CachyOS Linux\"\nPRETTY_NAME=\"CachyOS Linux\"\nID=cachyos\n",
           FAM_ARCH, "CachyOS Linux");
    expect("PRETTY_NAME=\"Ubuntu 24.04 LTS\"\nID=ubuntu\nID_LIKE=debian\n",
           FAM_DEBIAN, "Ubuntu 24.04 LTS");
    expect("PRETTY_NAME=\"Linux Mint 22\"\nID=linuxmint\nID_LIKE=\"ubuntu debian\"\n",
           FAM_DEBIAN, "Linux Mint 22");
    /* Zorin: matched by ID even when ID_LIKE is absent */
    expect("PRETTY_NAME=\"Zorin OS 17\"\nID=zorin\n",
           FAM_DEBIAN, "Zorin OS 17");
    /* Unknown ID falls back to ID_LIKE */
    expect("PRETTY_NAME=\"Foo\"\nID=foo\nID_LIKE=arch\n", FAM_ARCH, "Foo");
    expect("PRETTY_NAME=\"Gentoo\"\nID=gentoo\n", FAM_UNKNOWN, "Gentoo");

    puts("distro: ok");
    return 0;
}
