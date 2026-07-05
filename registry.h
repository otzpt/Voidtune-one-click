/**
 * registry.h — Abstração do Windows Registry
 * Todas as operações de registry passam por aqui.
 * Nunca chamas RegSetValueEx diretamente no resto do código.
 */

#ifndef REGISTRY_H
#define REGISTRY_H

#include <windows.h>

/* Resultado de uma operação de registry */
typedef enum {
    REG_OK = 0,
    REG_ERR_OPEN,
    REG_ERR_SET,
    REG_ERR_DELETE,
    REG_ERR_NOT_FOUND
} RegResult;

/*
 * reg_set_dword - Escreve um valor DWORD numa chave do registry
 * @hive:    HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, etc.
 * @path:    caminho da chave, ex: "SYSTEM\\CurrentControlSet\\..."
 * @name:    nome do valor
 * @value:   valor DWORD a escrever
 *
 * Retorna REG_OK em caso de sucesso.
 */
RegResult reg_set_dword(HKEY hive, const char *path,
                        const char *name, DWORD value);

/*
 * reg_delete_value - Remove um valor do registry
 * Retorna REG_OK ou REG_ERR_NOT_FOUND se já não existia.
 */
RegResult reg_delete_value(HKEY hive, const char *path, const char *name);

/*
 * reg_get_dword - Lê um valor DWORD do registry
 * @out: ponteiro onde o valor lido é guardado
 * Retorna REG_OK ou REG_ERR_NOT_FOUND.
 */
RegResult reg_get_dword(HKEY hive, const char *path,
                        const char *name, DWORD *out);

/* Traduz um RegResult para string legível */
const char *reg_result_str(RegResult r);

#endif /* REGISTRY_H */
