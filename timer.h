#ifndef TIMER_H
#define TIMER_H

#include <exec/types.h>

BOOL timer_init(void);
void timer_quit(void);

uint32 timer_signal(void);

void timer_start(void);
void timer_stop(void);
void timer_handle_events(void);

ULONG timer_frequency_ms(void);
ULONG timer_frequency(void);

#endif
