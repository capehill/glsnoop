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
    logLine("Primitive statistics (in vertices):");

    const uint64 total = counter->triangles + counter->triangleStrips + counter->triangleFans +
        counter->lines + counter->lineStrips + counter->lineLoops + counter->points;

    logLine("  Total vertices %llu, per second %.6f, per draw call %.6f", total, total / seconds, total / drawcalls);

    if (counter->triangles) {
        logLine("  Triangles %llu, per second %.6f, per draw call %.6f", counter->triangles,
            counter->triangles / seconds, counter->triangles / drawcalls);
    }

    if (counter->triangleStrips) {
        logLine("  Triangle strips %llu, per second %.6f, per draw call %.6f", counter->triangleStrips,
            counter->triangleStrips / seconds, counter->triangleStrips / drawcalls);
    }

    if (counter->triangleFans) {
        logLine("  Triangle fans %llu, per second %.6f, per draw call %.6f", counter->triangleFans,
            counter->triangleFans / seconds, counter->triangleFans / drawcalls);
    }

    if (counter->lines) {
        logLine("  Lines %llu, per second %.6f, per draw call %.6f", counter->lines,
            counter->lines / seconds, counter->lines / drawcalls);
    }

    if (counter->lineStrips) {
        logLine("  Line strips %llu, per second %.6f, per draw call %.6f", counter->lineStrips,
            counter->lineStrips / seconds, counter->lineStrips / drawcalls);
    }

    if (counter->lineLoops) {
        logLine("  Line loops %llu, per second %.6f, per draw call %.6f", counter->lineLoops,
            counter->lineLoops / seconds, counter->lineLoops / drawcalls);
    }

    if (counter->points) {
        logLine("  Points %llu, per second %.6f, per draw call %.6f", counter->points,
            counter->points / seconds, counter->points / drawcalls);
    }
}
