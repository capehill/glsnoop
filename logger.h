#ifndef LOGGER_H
#define LOGGER_H

void logLine(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));
void logAlways(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));

void pause_log(void);
void resume_log(void);

#endif

