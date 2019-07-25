#ifndef TIMER_H
#define TIMER_H

#include <exec/types.h>

BOOL timer_init(void);
void timer_quit(void);

uint32 timer_signal(void);

void timer_start(void);
void timer_stop(void);
void timer_handle_events(void);

double timer_ticks_to_s(const uint64 ticks);
double timer_ticks_to_ms(const uint64 ticks);
double timer_ticks_to_us(const uint64 ticks);

#endif
