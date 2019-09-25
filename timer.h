#ifndef TIMER_H
#define TIMER_H

#include <exec/types.h>

typedef struct TimerContext
{
    struct MsgPort* port;
    struct TimeRequest* request;
    BYTE device;
} TimerContext;

typedef enum ESignalType {
    ESignalType_Break,
    ESignalType_Timer
} ESignalType;

extern TimerContext triggerTimer;

BOOL timer_init(TimerContext * tc);
void timer_quit(TimerContext * tc);

uint32 timer_signal(TimerContext * tc);

void timer_start(TimerContext * tc, ULONG seconds, ULONG micros);
void timer_stop(TimerContext * tc);
void timer_handle_events(TimerContext * tc);

ESignalType timer_wait_for_signal(uint32 timerSig, const char* const name);
void timer_delay(ULONG seconds);

double timer_ticks_to_s(const uint64 ticks);
double timer_ticks_to_ms(const uint64 ticks);
double timer_ticks_to_us(const uint64 ticks);

#endif
