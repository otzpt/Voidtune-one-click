/**
 * tweaks.h — Motor de tweaks do VOIDTUNE C
 *
 * Cada tweak é uma struct com nome, descrição, categoria,
 * e dois ponteiros de função: apply() e revert().
 * Para adicionar um tweak novo: define as duas funções em tweaks.c
 * e adiciona a entrada no array g_tweaks[].
 */

#ifndef TWEAKS_H
#define TWEAKS_H

/* Categorias de tweak */
typedef enum {
    CAT_PERFORMANCE = 0,
    CAT_PRIVACY,
    CAT_NETWORK,
    CAT_SERVICES,
    CAT_VISUALS
} TweakCategory;

/* Nível de risco */
typedef enum {
    SAFE = 0,
    EXTREME
} TweakLevel;

/* Estado actual do tweak */
typedef enum {
    STATE_OFF = 0,
    STATE_ON
} TweakState;

/*
 * Tweak — representa um tweak individual
 *
 * apply()  : aplica o tweak, retorna 0 em sucesso
 * revert() : reverte o tweak, retorna 0 em sucesso
 */
typedef struct {
    int           id;
    const char   *name;
    const char   *desc;
    TweakCategory category;
    TweakLevel    level;
    TweakState    state;        /* carregado do ficheiro de estado */
    int         (*apply)(void);
    int         (*revert)(void);
} Tweak;

/* ── API pública ─────────────────────────────────────────────── */

/* Inicializa o motor — carrega estados do disco */
void tweaks_init(void);

/* Número total de tweaks disponíveis */
int tweaks_count(void);

/* Devolve ponteiro para o tweak pelo índice (0-based) */
Tweak *tweaks_get(int index);

/* Aplica um tweak pelo índice — atualiza state e guarda em disco */
int tweaks_apply(int index);

/* Reverte um tweak pelo índice — atualiza state e guarda em disco */
int tweaks_revert(int index);

/* Guarda todos os estados em voidtune_state.txt */
void tweaks_save_state(void);

/* Carrega estados de voidtune_state.txt */
void tweaks_load_state(void);

/* Strings legíveis para os enums */
const char *tweak_category_str(TweakCategory c);
const char *tweak_level_str(TweakLevel l);
const char *tweak_state_str(TweakState s);

#endif /* TWEAKS_H */
