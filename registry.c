/**
 * registry.c — Windows Registry abstraction implementation
 * Compiled: gcc -Wall -Wextra -o voidtune *.c -ladvapi32
 */

#include "registry.h"
#include <stdio.h>

/* ── Implementation ──────────────────────────────────────────── */

RegResult reg_set_dword(HKEY hive, const char *path,
                        const char *name, DWORD value)
{
    HKEY key;
    LONG res;

    /* Opens or creates the key with write permission */
    res = RegCreateKeyExA(
        hive, path,
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE, NULL,
        &key, NULL
    );

    if (res != ERROR_SUCCESS)
        return REG_ERR_OPEN;

    res = RegSetValueExA(
        key, name, 0, REG_DWORD,
        (const BYTE *)&value, sizeof(DWORD)
    );

    RegCloseKey(key);
    return (res == ERROR_SUCCESS) ? REG_OK : REG_ERR_SET;
}

RegResult reg_delete_value(HKEY hive, const char *path, const char *name)
{
    HKEY key;
    LONG res;

    res = RegOpenKeyExA(hive, path, 0, KEY_SET_VALUE, &key);
    if (res != ERROR_SUCCESS)
        return REG_ERR_OPEN;

    res = RegDeleteValueA(key, name);
    RegCloseKey(key);

    if (res == ERROR_FILE_NOT_FOUND)
        return REG_ERR_NOT_FOUND;

    return (res == ERROR_SUCCESS) ? REG_OK : REG_ERR_DELETE;
}

RegResult reg_get_dword(HKEY hive, const char *path,
                        const char *name, DWORD *out)
{
    HKEY key;
    DWORD size = sizeof(DWORD);
    DWORD type;
    LONG  res;

    res = RegOpenKeyExA(hive, path, 0, KEY_QUERY_VALUE, &key);
    if (res != ERROR_SUCCESS)
        return REG_ERR_OPEN;

    res = RegQueryValueExA(key, name, NULL, &type, (BYTE *)out, &size);
    RegCloseKey(key);

    if (res == ERROR_FILE_NOT_FOUND)
        return REG_ERR_NOT_FOUND;

    return (res == ERROR_SUCCESS) ? REG_OK : REG_ERR_NOT_FOUND;
}

const char *reg_result_str(RegResult r)
{
    switch (r) {
        case REG_OK:            return "OK";
        case REG_ERR_OPEN:      return "Failed to open key";
        case REG_ERR_SET:       return "Failed to write value";
        case REG_ERR_DELETE:    return "Failed to delete value";
        case REG_ERR_NOT_FOUND: return "Value not found";
        default:                return "Unknown error";
    }
}
