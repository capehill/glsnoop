#ifndef COMMON_H
#define COMMON_H

#include <proto/exec.h>

#define NAME_LEN 64
#define MAX_CLIENTS 5

void find_process_name2(struct Node * node, char * destination);

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
            logDebug("Patched " #func " %p with %p", ctx->old_##func, prefix##_##func); \
        } \
    } else { \
        logDebug("Restored " #func); \
    } \
} \

#define GENERATE_FILTERED_PATCH(type,func,prefix,ctxtype) \
static void patch_##func(BOOL patching, struct ctxtype * ctx) \
{ \
    IExec->Forbid(); \
    if (patching) { \
        if (match(#func)) { \
            ctx->old_##func = (const void *)IExec->SetMethod((struct Interface *)ctx->interface, offsetof(struct type, func), prefix##_##func); \
        } \
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
            logDebug("Patched " #func " %p with %p", ctx->old_##func, prefix##_##func); \
        } \
    } else { \
        logDebug("Restored " #func); \
    } \
} \


#endif

