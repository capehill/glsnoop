#ifndef COMMON_H
#define COMMON_H

#include <proto/exec.h>

#define NAME_LEN 32
#define MAX_CLIENTS 5

void logLine(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));
void pause_log(void);
void resume_log(void);

#define GENERATE_PATCH(type,func,prefix,ctxtype) \
static void patch_##func(BOOL patching, struct ctxtype * ctx) \
{ \
    IExec->Forbid(); \
    if (patching) { \
        ctx->old_##func = (const void *)IExec->SetMethod((struct Interface *)ctx->interface, offsetof(struct type, func), prefix##_##func); \
    } else { \
        if (ctx->old_##func) { \
            IExec->SetMethod((struct Interface *)ctx->interface, offsetof(struct type, func), ctx->old_##func); \
            ctx->old_##func = NULL; \
        } \
    } \
    IExec->Permit(); \
    \
    if (patching) { \
        if (ctx->old_##func) { \
            logLine("Patched " #func " %p with %p", ctx->old_##func, prefix##_##func); \
        } \
    } else { \
        logLine("Restored " #func); \
    } \
} \

#endif

