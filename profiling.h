#ifndef PROFILING_H
#define PROFILING_H

#include <proto/timer.h>

typedef struct ProfilingItem
{
    uint64 ticks;
    uint64 callCount;
    int index; // Only for qsorting
} ProfilingItem;

typedef struct MyClock {
    union {
        uint64 ticks;
        struct EClockVal clockVal;
    };
} MyClock;

typedef struct PrimitiveCounter {
    uint64 triangles;
    uint64 triangleStrips;
    uint64 triangleFans;
    uint64 lines;
    uint64 lineStrips;
    uint64 lineLoops;
    uint64 points;
} PrimitiveCounter;

#define PROF_INIT(context, LAST_INDEX) \
    ITimer->ReadEClock(&context->start.clockVal); \
    for (int i = 0; i < LAST_INDEX; i++) { \
        context->profiling[i].index = i; \
    }

#define PROF_START \
    struct MyClock start, finish; \
    ITimer->ReadEClock(&start.clockVal);

#define PROF_FINISH(func) \
    ITimer->ReadEClock(&finish.clockVal); \
    const uint64 duration = finish.ticks - start.ticks; \
    context->ticks += duration; \
    context->profiling[func].ticks += duration; \
    context->profiling[func].callCount++;

#define PROF_FINISH_CONTEXT \
    MyClock finish; \
    ITimer->ReadEClock(&finish.clockVal); \
    const uint64 totalTicks = finish.ticks - context->start.ticks; \
    const double seconds = (double)totalTicks / timer_frequency();

#define PROF_PRINT_TOTAL \
    logLine("Total recorded duration %.6f ms, %.2f %% of total context life-time %.6f ms", \
        (double)context->ticks / timer_frequency_ms(), \
        (double)context->ticks * 100.0 / totalTicks, \
        (double)totalTicks / timer_frequency_ms()); \

int tickComparison(const void* first, const void* second);
void primitiveStats(const PrimitiveCounter* const counter, const double seconds, const double drawcalls);

#endif

