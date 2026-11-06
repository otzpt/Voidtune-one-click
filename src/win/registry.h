/**
 * registry.h - Windows Registry abstraction
 * All registry operations go through here.
 * Never call RegSetValueEx directly anywhere else in the codebase.
 */

#ifndef REGISTRY_H
#define REGISTRY_H

#include <windows.h>

/* Result of a registry operation */
typedef enum {
    REG_OK = 0,
    REG_ERR_OPEN,
    REG_ERR_SET,
    REG_ERR_DELETE,
    REG_ERR_NOT_FOUND
} RegResult;

/*
 * reg_set_dword - Writes a DWORD value to a registry key
 * @hive:    HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, etc.
 * @path:    key path, e.g. "SYSTEM\\CurrentControlSet\\..."
 * @name:    value name
 * @value:   DWORD value to write
 *
 * Returns REG_OK on success.
 */
RegResult reg_set_dword(HKEY hive, const char *path,
                        const char *name, DWORD value);

/*
 * reg_delete_value - Removes a value from the registry
 * Returns REG_OK, or REG_ERR_NOT_FOUND if it didn't exist.
 */
RegResult reg_delete_value(HKEY hive, const char *path, const char *name);

/*
 * reg_get_dword - Reads a DWORD value from the registry
 * @out: pointer where the read value is stored
 * Returns REG_OK or REG_ERR_NOT_FOUND.
 */
RegResult reg_get_dword(HKEY hive, const char *path,
                        const char *name, DWORD *out);

/* Translates a RegResult to a human-readable string */
const char *reg_result_str(RegResult r);

#endif /* REGISTRY_H */
