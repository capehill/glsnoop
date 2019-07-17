#ifndef PROFILING_H
#define PROFILING_H

#include <proto/timer.h>

typedef struct ProfilingItem
{
    uint64 ticks;
    uint64 callCount;
} ProfilingItem;

typedef struct MyClock {
    union {
        uint64 ticks;
        struct EClockVal clockVal;
    };
} MyClock;

#define PROF_INIT(context) ITimer->ReadEClock(&context->start.clockVal);

#define PROF_START \
    struct MyClock start, finish; \
    ITimer->ReadEClock(&start.clockVal);

#define PROF_FINISH(func) \
    ITimer->ReadEClock(&finish.clockVal); \
    const uint64 duration = finish.ticks - start.ticks; \
    context->totalTicks += duration; \
    context->profiling[func].ticks += duration; \
    context->profiling[func].callCount++;

#define PROF_FINISH_CONTEXT \
    MyClock finish; \
    ITimer->ReadEClock(&finish.clockVal); \
    const uint64 totalDuration = finish.ticks - context->start.ticks;

#define PROF_PRINT_TOTAL \
    logLine("Total recorded duration %.6f ms, %.2f %% of total %.6f ms", \
        (double)context->totalTicks / timer_frequency_ms(), \
        (double)context->totalTicks * 100.0 / totalDuration, \
        (double)totalDuration / timer_frequency_ms()); \
    logLine("--------------------------------------------------------");

#endif

