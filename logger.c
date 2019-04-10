#include "common.h"

#include <proto/exec.h>

#include <stdio.h>
#include <stdarg.h>

void logLine(const char * fmt, ...)
{
    char buffer[1024];

    va_list ap;
    va_start(ap, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, ap);

    va_end(ap);

    IExec->DebugPrintF("%s\n", buffer);
}

