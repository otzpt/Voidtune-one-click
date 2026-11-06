/**
 * linux_common.h - helpers shared by the Linux backend files
 */

#ifndef LINUX_COMMON_H
#define LINUX_COMMON_H

/* 1 when VOIDTUNE_DRY_RUN is set: print writes instead of doing them. */
int lx_dry_run(void);

/* State directory (created if missing). */
const char *lx_state_dir(void);

#endif /* LINUX_COMMON_H */
