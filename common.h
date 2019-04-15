#ifndef COMMON_H
#define COMMON_H

#include <proto/exec.h>

void logLine(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));
void pause_log(void);
void resume_log(void);

#define PATCH_INTERFACE(type,func,prefix) \
static void patch_##func(BOOL patching, struct Interface* interface) \
{ \
    IExec->Forbid(); \
    if (patching) { \
        old_##func = IExec->SetMethod(interface, offsetof(struct type, func), prefix##_##func); \
    } else { \
        if (old_##func) { \
            IExec->SetMethod(interface, offsetof(struct type, func), old_##func); \
            old_##func = NULL; \
        } \
    } \
    IExec->Permit(); \
    \
    if (patching) { \
        if (old_##func) { \
            logLine("Patched " #func " %p with %p", old_##func, prefix##_##func); \
        } \
    } else { \
        logLine("Restored " #func); \
    } \
} \

#endif

