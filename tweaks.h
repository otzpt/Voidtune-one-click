/**
 * tweaks.h — VOIDTUNE C tweak engine
 *
 * Each tweak is a struct with a name, description, category,
 * and two function pointers: apply() and revert().
 * To add a new tweak: define the two functions in tweaks.c
 * and add the entry to the g_tweaks[] array.
 */

#ifndef TWEAKS_H
#define TWEAKS_H

/* Tweak categories */
typedef enum {
    CAT_PERFORMANCE = 0,
    CAT_PRIVACY,
    CAT_NETWORK,
    CAT_SERVICES,
    CAT_VISUALS
} TweakCategory;

/* Risk level */
typedef enum {
    SAFE = 0,
    EXTREME
} TweakLevel;

/* Current tweak state */
typedef enum {
    STATE_OFF = 0,
    STATE_ON
} TweakState;

/*
 * Tweak — represents a single tweak
 *
 * apply()  : applies the tweak, returns 0 on success
 * revert() : reverts the tweak, returns 0 on success
 */
typedef struct {
    int           id;
    const char   *name;
    const char   *desc;
    TweakCategory category;
    TweakLevel    level;
    TweakState    state;        /* loaded from the state file */
    int         (*apply)(void);
    int         (*revert)(void);
} Tweak;

/* ── Public API ──────────────────────────────────────────────── */

/* Initializes the engine — loads state from disk */
void tweaks_init(void);

/* Total number of available tweaks */
int tweaks_count(void);

/* Returns a pointer to the tweak at this index (0-based) */
Tweak *tweaks_get(int index);

/* Applies a tweak by index — updates state and saves to disk */
int tweaks_apply(int index);

/* Reverts a tweak by index — updates state and saves to disk */
int tweaks_revert(int index);

/* Saves all states to voidtune_state.txt */
void tweaks_save_state(void);

/* Loads states from voidtune_state.txt */
void tweaks_load_state(void);

/* Human-readable strings for the enums */
const char *tweak_category_str(TweakCategory c);
const char *tweak_level_str(TweakLevel l);
const char *tweak_state_str(TweakState s);

#endif /* TWEAKS_H */
