/**
 * backup.h — Backup do Windows Registry
 * Porta de MakeBackup() em core/helpers.ps1 do VOIDTUNE v0.8V
 * Copyright (C) 2026 @OTZPT — GPL v3
 *
 * Exporta as chaves de registry afectadas pelos tweaks para um
 * ficheiro .reg com timestamp, que pode ser importado em qualquer
 * altura para restaurar o estado anterior.
 */

#ifndef BACKUP_H
#define BACKUP_H

/*
 * backup_create - cria um ficheiro .reg na pasta corrente.
 *
 * @note     : sufixo descritivo, ex: "manual", "pre-apply"
 * @out_path : buffer onde o caminho do ficheiro criado é escrito
 * @out_size : tamanho do buffer out_path
 *
 * Retorna 1 em sucesso, 0 em erro.
 */
int backup_create(const char *note, char *out_path, int out_size);

#endif /* BACKUP_H */
