#ifndef PROFILING_H
#define PROFILING_H

#include <proto/timer.h>

typedef struct ProfilingItem
{
    uint64 ticks;
    uint64 callCount;
    uint64 errors;
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
    context->ticks = 0; \
    for (int item = 0; item < LAST_INDEX; item++) { \
        ProfilingItem *pi = &context->profiling[item]; \
        pi->ticks = 0; \
        pi->callCount = 0; \
        pi->errors = 0; \
        pi->index = item; \
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
    const double seconds = timer_ticks_to_s(totalTicks);

#define PROF_PRINT_TOTAL \
    const double timeUsed = timer_ticks_to_ms(context->ticks); \
    char timeUsedBuffer[32]; \
    snprintf(timeUsedBuffer, sizeof(timeUsedBuffer), "%% of %.6f ms", timeUsed); \
    logAlways("  Function calls used %.6f ms, %.2f %% of context life-time %.6f ms", \
        timeUsed, \
        (double)context->ticks * 100.0 / totalTicks, \
        timer_ticks_to_ms(totalTicks));

int tickComparison(const void* first, const void* second);
void sort(ProfilingItem* items, const unsigned count);

void primitiveStats(const PrimitiveCounter* const counter, const double seconds, const double drawcalls);

#endif

