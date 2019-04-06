#ifndef COMMON_H
#define COMMON_H

#include <proto/exec.h>

static STRPTR task_name()
{
    return (((struct Node *)IExec->FindTask(NULL))->ln_Name);
}

#define PATCH_INTERFACE(type,func) \
static void patch_##func(BOOL patching, struct Interface* interface) \
{ \
    IExec->Forbid(); \
    if (patching) { \
        old_##func = IExec->SetMethod(interface, offsetof(struct type, func), my_##func); \
        if (old_##func) { \
            IExec->DebugPrintF("Patched " #func " %p with %p\n", old_##func, my_##func); \
        } \
    } else { \
        if (old_##func) { \
            IExec->SetMethod(interface, offsetof(struct type, func), old_##func); \
            IExec->DebugPrintF("Restored " #func " %p\n", old_##func); \
            old_##func = NULL; \
        } \
    } \
    IExec->Permit(); \
} \


#endif

