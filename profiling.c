#include "profiling.h"
#include "logger.h"

#include <stdlib.h>

int tickComparison(const void* first, const void* second)
{
    const ProfilingItem* a = first;
    const ProfilingItem* b = second;

    if (a->ticks > b->ticks) return -1;
    if (a->ticks < b->ticks) return 1;

    return 0;
}

void sort(ProfilingItem* items, const unsigned count)
{
    qsort(items, count, sizeof(ProfilingItem), tickComparison);
}

void primitiveStats(const PrimitiveCounter* const counter, const double seconds, const double drawcalls)
{
    logAlways("  Primitive statistics:");

    const uint64 total = counter->triangles + counter->triangleStrips + counter->triangleFans +
        counter->lines + counter->lineStrips + counter->lineLoops + counter->points;

    if (total == 0) {
        logAlways("    Nothing was drawn, vertex count 0");
        return;
    }

    logAlways("    Total vertices %llu. %.1f vertices/s, %.1f vertices/call, consisting of:", total, total / seconds, total / drawcalls);

    if (counter->triangles) {
        logAlways("    - Triangle vertices %llu. %.1f verts/s, %.1f verts/call", counter->triangles,
            counter->triangles / seconds, counter->triangles / drawcalls);
    }

    if (counter->triangleStrips) {
        logAlways("    - Triangle strip vertices %llu. %.1f verts/s, %.1f verts/call", counter->triangleStrips,
            counter->triangleStrips / seconds, counter->triangleStrips / drawcalls);
    }

    if (counter->triangleFans) {
        logAlways("    - Triangle fan vertices %llu. %.1f verts/s, %.1f verts/call", counter->triangleFans,
            counter->triangleFans / seconds, counter->triangleFans / drawcalls);
    }

    if (counter->lines) {
        logAlways("    - Line vertices %llu. %.1f verts/s, %.1f verts/call", counter->lines,
            counter->lines / seconds, counter->lines / drawcalls);
    }

    if (counter->lineStrips) {
        logAlways("    - Line strip vertices %llu. %.1f verts/s, %.1f verts/call", counter->lineStrips,
            counter->lineStrips / seconds, counter->lineStrips / drawcalls);
    }

    if (counter->lineLoops) {
        logAlways("    - Line loop vertices %llu. %.1f verts/s, %.1f verts/call", counter->lineLoops,
            counter->lineLoops / seconds, counter->lineLoops / drawcalls);
    }

    if (counter->points) {
        logAlways("    - Point vertices %llu. %.1f verts/s, %.1f verts/call", counter->points,
            counter->points / seconds, counter->points / drawcalls);
    }
}
