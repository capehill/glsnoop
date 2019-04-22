#include "filter.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILTER 100

static char* filters[MAX_FILTER];
static size_t count;

static void add_filter(const char* const name)
{
    if (strlen(name) > 0) {
        size_t i;
        for (i = 0; i < MAX_FILTER; i++) {
            if (filters[i] == NULL) {
                filters[i] = strdup(name);
                logLine("Filter[%u] '%s' added", count, filters[i]);
                count++;
                break;
            }
        }
    }
}

static void strip(char* const buffer)
{
    size_t i;
    for (i = 0; i < strlen(buffer); i++) {
        switch (buffer[i]) {
            case ' ':
            case '\n':
            case '#':
            case ';':
                buffer[i] = 0;
                break;
        }
    }
}

BOOL load_filters(const char* const fileName)
{
    if (fileName) {
        FILE* file = fopen(fileName, "r");

        if (file) {
            char buffer[64];
            while (!feof(file)) {
                if (fgets(buffer, sizeof(buffer), file) != NULL) {
                    strip(buffer);
                    add_filter(buffer);
                }
            }

            fclose(file);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL match(const char* const name)
{
    if (count == 0) {
        // No filters in use
        return TRUE;
    }

    size_t i;
    for (i = 0; i < MAX_FILTER; i++) {
        if (filters[i] && strstr(name, filters[i])) {
            //logLine("Match '%s' vs '%s'", name, filters[i]);
            return TRUE;
        }
    }

    return FALSE;
}

void free_filters(void)
{
    size_t i;
    for (i = 0; i < MAX_FILTER; i++) {
        free(filters[i]);
        filters[i] = NULL;
    }
}

