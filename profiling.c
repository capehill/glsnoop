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

    logAlways("    Total vertices %llu, per second %.6f, per draw call %.6f, consisting of:", total, total / seconds, total / drawcalls);

    if (counter->triangles) {
        logAlways("    - Triangles %llu, per second %.6f, per draw call %.6f", counter->triangles,
            counter->triangles / seconds, counter->triangles / drawcalls);
    }

    if (counter->triangleStrips) {
        logAlways("    - Triangle strips %llu, per second %.6f, per draw call %.6f", counter->triangleStrips,
            counter->triangleStrips / seconds, counter->triangleStrips / drawcalls);
    }

    if (counter->triangleFans) {
        logAlways("    - Triangle fans %llu, per second %.6f, per draw call %.6f", counter->triangleFans,
            counter->triangleFans / seconds, counter->triangleFans / drawcalls);
    }

    if (counter->lines) {
        logAlways("    - Lines %llu, per second %.6f, per draw call %.6f", counter->lines,
            counter->lines / seconds, counter->lines / drawcalls);
    }

    if (counter->lineStrips) {
        logAlways("    - Line strips %llu, per second %.6f, per draw call %.6f", counter->lineStrips,
            counter->lineStrips / seconds, counter->lineStrips / drawcalls);
    }

    if (counter->lineLoops) {
        logAlways("    - Line loops %llu, per second %.6f, per draw call %.6f", counter->lineLoops,
            counter->lineLoops / seconds, counter->lineLoops / drawcalls);
    }

    if (counter->points) {
        logAlways("    - Points %llu, per second %.6f, per draw call %.6f", counter->points,
            counter->points / seconds, counter->points / drawcalls);
    }
}
