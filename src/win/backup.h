/**
 * backup.h - Windows Registry backup
 * Ported from MakeBackup() in core/helpers.ps1 of VOIDTUNE v0.8V
 * Copyright (C) 2026 @OTZPT - GPL v3
 *
 * Exports the registry keys touched by the tweaks to a timestamped
 * .reg file, which can be imported at any time to restore the prior
 * state.
 */

#ifndef BACKUP_H
#define BACKUP_H

/*
 * backup_create - creates a .reg file in the current folder.
 *
 * @note     : descriptive suffix, e.g. "manual", "pre-apply"
 * @out_path : buffer where the path of the created file is written
 * @out_size : size of the out_path buffer
 *
 * Returns 1 on success, 0 on error.
 */
int backup_create(const char *note, char *out_path, int out_size);

#endif /* BACKUP_H */
