/**
 * services.h — Windows service management
 * Ported from modules/data.ps1 ($SVCS_DATA) of VOIDTUNE v0.8V
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

/* Total number of services in the list */
int services_count(void);

/* Name and description of the service at this index */
const char *services_get_name(int index);
const char *services_get_desc(int index);

/* Current state of the service (queries the SCM) */
ServiceState services_get_state(int index);

/* Stops and disables the service; returns 0 on success */
int services_disable(int index);

/* Re-enables and starts the service; returns 0 on success */
int services_enable(int index);

/* Prints the full list to the console with colored state */
void services_print_all(void);

/* Human-readable string for a ServiceState */
const char *service_state_str(ServiceState s);

#endif /* SERVICES_H */
