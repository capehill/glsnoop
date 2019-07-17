#include "profiling.h"

int tickComparison(const void* first, const void* second)
{
    const ProfilingItem* a = first;
    const ProfilingItem* b = second;

    if (a->ticks > b->ticks) return -1;
    if (a->ticks < b->ticks) return 1;

    return 0;
}
