#include "logger.h"

#include <proto/exec.h>

#include <stdio.h>
#include <stdarg.h>

static BOOL paused = FALSE;
static BOOL verbose = FALSE;

static void logLineImpl(const char * fmt, va_list ap)
{
    char buffer[16 * 1024];
    const int len = vsnprintf(buffer, sizeof(buffer), fmt, ap);

    IExec->DebugPrintF("%s\n", buffer);

    if (len >= (int)sizeof(buffer)) {
        IExec->DebugPrintF("*** Line truncated: %d bytes buffer needed ***\n", len);
    }
}

void logLine(const char * fmt, ...)
{
    if (!paused) {
        va_list ap;
        va_start(ap, fmt);

        logLineImpl(fmt, ap);

        va_end(ap);
    }
}

void logAlways(const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    logLineImpl(fmt, ap);

    va_end(ap);
}

void logDebug(const char * fmt, ...)
{
    if (verbose) {
        va_list ap;
        va_start(ap, fmt);

        logLineImpl(fmt, ap);

        va_end(ap);
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
