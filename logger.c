#include "common.h"

#include <proto/exec.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static FILE* file;
static APTR mutex;
static APTR logBuffer;

static const size_t LOG_BUFFER_SIZE = 10 * 1024 * 1024;
static size_t offset = 0;

// If filename is passed, let's try to use RAM buffer + file. In other cases log to serial port.

void logInit(const char * filename)
{
    if (filename) {
        printf("Opening '%s'\n", filename);

        file = fopen(filename, "w");

        mutex = IExec->AllocSysObjectTags(ASOT_MUTEX, TAG_DONE);

        logBuffer = IExec->AllocVecTags(LOG_BUFFER_SIZE, AVT_ClearWithValue, 0, TAG_DONE);
    }
}

static void warn(void)
{
    static BOOL warned = FALSE;

    if (!warned) {
        IExec->DebugPrintF("glSnoop filebuffer became full!\n");
        warned = TRUE;
    }
}

void logLine(const char * fmt, ...)
{
    char buffer[1024];

	va_list ap;
	va_start(ap, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, ap);

	va_end(ap);

    if (mutex && logBuffer) {
        // Write log buffer one task at time
        IExec->MutexObtain(mutex);

        const size_t capacity = LOG_BUFFER_SIZE - offset;

        if ((strlen(buffer) + 1) < capacity) {
            snprintf(logBuffer + offset, capacity, "%s\n", buffer);
            offset += strlen(buffer) + 1;
        } else {
            warn();
        }

        IExec->MutexRelease(mutex);
    } else {
        IExec->DebugPrintF("%s\n", buffer);
    }
}

void logExit(void)
{
    if (file) {
        const size_t CHUNK_SIZE = 64 * 1024;
        size_t left = offset;
        char* source = logBuffer;

        while (left > 0) {
            const size_t writeSize = left < CHUNK_SIZE ? left : CHUNK_SIZE;
            const size_t objs = fwrite(source, writeSize, 1, file);

            if (!objs) {
                break;
            }

            left -= writeSize;
            source += writeSize;
        }

        fclose(file);
        file = NULL;
    }

    if (mutex) {
        IExec->FreeSysObject(ASOT_MUTEX, mutex);
        mutex = NULL;
    }

    if (logBuffer) {
        IExec->FreeVec(logBuffer);
        logBuffer = NULL;
    }
}
