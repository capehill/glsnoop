#ifndef FILTER_H
#define FILTER_H

#include <exec/types.h>

BOOL load_filters(const char* const fileName);
BOOL match(const char* const name);
void free_filters(void);

#endif
