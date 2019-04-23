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
    if (!name) {
        logLine("%s: NULL pointer\n", __func__);
        return;
    }

    if (strlen(name) > 0) {
        if (count < MAX_FILTER) {
            if (filters[count] == NULL) {
                filters[count] = strdup(name);
                logLine("Filter[%u] '%s' added", count, filters[count]);
                count++;
            }
        }
    }
}

static void strip(char* const buffer)
{
    if (!buffer) {
        logLine("%s: NULL pointer\n", __func__);
        return;
    }

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
    if (!fileName) {
        // No problem, we don't filter, then
        return FALSE;
    }

    FILE* file = fopen(fileName, "r");

    if (!file) {
        logLine("Failed to open '%s'\n", fileName);
        return FALSE;
    }

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

BOOL match(const char* const name)
{
    if (!name) {
        logLine("%s: NULL pointer\n", __func__);
        return FALSE;
    }

    if (count == 0) {
        // No filters in use
        return TRUE;
    }

    size_t i;
    for (i = 0; i < count; i++) {
        if (filters[i] && (strcmp(name, filters[i]) == 0)) {
            //logLine("Match '%s' vs '%s'", name, filters[i]);
            return TRUE;
        }
    }

    return FALSE;
}

void free_filters(void)
{
    size_t i;
    for (i = 0; i < count; i++) {
        free(filters[i]);
        filters[i] = NULL;
    }
}

