/**
 * services.h — Gestão de serviços do Windows
 * Porta de modules/data.ps1 ($SVCS_DATA) do VOIDTUNE v0.8V
 * Copyright (C) 2026 @OTZPT — GPL v3
 */

#ifndef SERVICES_H
#define SERVICES_H

typedef enum {
    SVC_UNKNOWN = 0,
    SVC_RUNNING,
    SVC_STOPPED,
    SVC_AUTO,
    SVC_DISABLED,
    SVC_MANUAL
} ServiceState;

/* Número total de serviços na lista */
int services_count(void);

/* Nome e descrição do serviço pelo índice */
const char *services_get_name(int index);
const char *services_get_desc(int index);

/* Estado actual do serviço (consulta o SCM) */
ServiceState services_get_state(int index);

/* Para e desactiva o serviço; retorna 0 em sucesso */
int services_disable(int index);

/* Reativa e inicia o serviço; retorna 0 em sucesso */
int services_enable(int index);

/* Imprime a lista completa na consola com estado a cores */
void services_print_all(void);

/* String legível para ServiceState */
const char *service_state_str(ServiceState s);

#endif /* SERVICES_H */
