#include "profiling.h"
#include "logger.h"

int tickComparison(const void* first, const void* second)
{
    const ProfilingItem* a = first;
    const ProfilingItem* b = second;

    if (a->ticks > b->ticks) return -1;
    if (a->ticks < b->ticks) return 1;

    return 0;
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
        logAlways("    - Triangles %llu. %.1f tris/s, %.1f tris/call", counter->triangles,
            counter->triangles / seconds, counter->triangles / drawcalls);
    }

    if (counter->triangleStrips) {
        logAlways("    - Triangle strips %llu. %.1f strips/s, %.1f strips/call", counter->triangleStrips,
            counter->triangleStrips / seconds, counter->triangleStrips / drawcalls);
    }

    if (counter->triangleFans) {
        logAlways("    - Triangle fans %llu. %.1f fans/s, %.1f fans/call", counter->triangleFans,
            counter->triangleFans / seconds, counter->triangleFans / drawcalls);
    }

    if (counter->lines) {
        logAlways("    - Lines %llu. %.1f lines/s, %.1f lines/call", counter->lines,
            counter->lines / seconds, counter->lines / drawcalls);
    }

    if (counter->lineStrips) {
        logAlways("    - Line strips %llu. %.1f strips/s, %.1f strips/call", counter->lineStrips,
            counter->lineStrips / seconds, counter->lineStrips / drawcalls);
    }

    if (counter->lineLoops) {
        logAlways("    - Line loops %llu. %.1f loops/s, %.1f loops/call", counter->lineLoops,
            counter->lineLoops / seconds, counter->lineLoops / drawcalls);
    }

    if (counter->points) {
        logAlways("    - Points %llu. %.1f points/s, %.1f points/call", counter->points,
            counter->points / seconds, counter->points / drawcalls);
    }
}
