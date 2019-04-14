#include "common.h"

#include <proto/exec.h>

#include <stdio.h>
#include <stdarg.h>

static BOOL paused = FALSE;

void logLine(const char * fmt, ...)
{
    if (!paused) {
        char buffer[1024];

        va_list ap;
        va_start(ap, fmt);

        vsnprintf(buffer, sizeof(buffer), fmt, ap);

        va_end(ap);

        IExec->DebugPrintF("%s\n", buffer);
    }
}

void pause_log(void)
{
    paused = TRUE;
    IExec->DebugPrintF("glSnoop: tracing paused\n");
}

void unpause_log(void)
{
    IExec->DebugPrintF("glSnoop: tracing started\n");
    paused = FALSE;
}
