#include "common.h"

#include <proto/exec.h>

#include <stdio.h>
#include <stdarg.h>

static BOOL paused = FALSE;

void logLine(const char * fmt, ...)
{
    if (!paused) {
        char buffer[16 * 1024];
        int len;

        va_list ap;
        va_start(ap, fmt);

        len = vsnprintf(buffer, sizeof(buffer), fmt, ap);

        va_end(ap);

        IExec->DebugPrintF("%s\n", buffer);

        if (len >= (int)sizeof(buffer)) {
            IExec->DebugPrintF("*** Line truncated: %d bytes buffer needed ***\n", len);
        }
    }
}

void pause_log(void)
{
    if (!paused) {
        paused = TRUE;
        IExec->DebugPrintF("glSnoop: tracing paused\n");
    }
}

void resume_log(void)
{
    if (paused) {
        IExec->DebugPrintF("glSnoop: tracing resumed\n");
        paused = FALSE;
    }
}
