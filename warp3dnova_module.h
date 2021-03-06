#ifndef WARP3DNOVA_MODULE_H
#define WARP3DNOVA_MODULE_H

#include <exec/types.h>

void warp3dnova_install_patches(ULONG startTimeInSeconds, ULONG durationTimeInSeconds);
void warp3dnova_remove_patches(void);
void warp3dnova_free(void);

const char* warp3dnova_version_string(void);
const char* warp3dnova_errors_string(void);

void warp3dnova_start_profiling(void);
void warp3dnova_finish_profiling(void);

#endif

