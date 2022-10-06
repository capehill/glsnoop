#include "timer.h"
#include "logger.h"
#include "profiling.h"

#include <proto/exec.h>
#include <proto/timer.h>
#include <dos/dos.h>

#include <stdio.h>

static MyClock start;
static ULONG frequency = 0;
static int users = 0;

TimerContext triggerTimer;

static void read_frequency(void)
{
    frequency = ITimer->ReadEClock(&start.clockVal);

    logLine("Timer frequency %lu ticks / second", frequency);
}

BOOL timer_init(TimerContext * tc)
{
    tc->port = NULL;
    tc->request = NULL;
    tc->device = -1;

    tc->port = IExec->AllocSysObjectTags(ASOT_PORT,
        ASOPORT_Name, "timer_port",
        TAG_DONE);

    if (!tc->port) {
        puts("Couldn't create timer port");
        goto out;
    }

    tc->request = IExec->AllocSysObjectTags(ASOT_IOREQUEST,
        ASOIOR_Size, sizeof(struct TimeRequest),
        ASOIOR_ReplyPort, tc->port,
        TAG_DONE);

    if (!tc->request) {
        puts("Couldn't create timer IO request");
        goto out;
    }

    tc->device = IExec->OpenDevice(TIMERNAME, UNIT_WAITUNTIL,
        (struct IORequest *) tc->request, 0);

    if (tc->device) {
        puts("Couldn't open timer.device");
        goto out;
    }

    if (!ITimer) {
        ITimer = (struct TimerIFace *) IExec->GetInterface(
            (struct Library *) tc->request->Request.io_Device, "main", 1, NULL);

        if (!ITimer) {
            puts("Couldn't get Timer interface");
            goto out;
        }
    }

    if (!frequency) {
        read_frequency();
    }

    users++;

    return TRUE;

out:
    timer_quit(tc);

    return FALSE;
}

void timer_quit(TimerContext * tc)
{
    if ((--users <= 0) && ITimer) {
        //logLine("ITimer user count %d, dropping it", users);
        IExec->DropInterface((struct Interface *) ITimer);
        ITimer = NULL;
    }

    if (tc->device == 0 && tc->request) {
        IExec->CloseDevice((struct IORequest *) tc->request);
        tc->device = -1;
    }

    if (tc->request) {
        IExec->FreeSysObject(ASOT_IOREQUEST, tc->request);
        tc->request = NULL;
    }

    if (tc->port) {
        IExec->FreeSysObject(ASOT_PORT, tc->port);
        tc->port = NULL;
    }
}

uint32 timer_signal(TimerContext * tc)
{
    if (!tc->port) {
        logLine("%s: timer port NULL", __func__);
        return 0;
    }

    return 1L << tc->port->mp_SigBit;
}

void timer_start(TimerContext * tc, ULONG seconds, ULONG micros)
{
    struct TimeVal tv;
    struct TimeVal increment;

    if (!tc->request) {
        logLine("%s: timer request NULL", __func__);
        return;
    }

    if (!ITimer) {
        logLine("%s: ITimer NULL", __func__);
        return;
    }

    ITimer->GetSysTime(&tv);

    increment.Seconds = seconds;
    increment.Microseconds = micros;

    ITimer->AddTime(&tv, &increment);

    tc->request->Request.io_Command = TR_ADDREQUEST;
    tc->request->Time.Seconds = tv.Seconds;
    tc->request->Time.Microseconds = tv.Microseconds;

    IExec->SendIO((struct IORequest *) tc->request);
}

void timer_handle_events(TimerContext * tc)
{
    struct Message *msg;

    if (!tc->port) {
        logLine("%s: timer port NULL", __func__);
        return;
    }

    while ((msg = IExec->GetMsg(tc->port))) {
        const int8 error = ((struct IORequest *)msg)->io_Error;

        if (error) {
            printf("Timer message received with code %d\n", error);
        }
    }
}

void timer_stop(TimerContext * tc)
{
    if (!tc->request) {
        logLine("%s: timer request NULL", __func__);
        return;
    }

    if (!IExec->CheckIO((struct IORequest *) tc->request)) {
        logLine("%s: aborting timer IO request %p", __func__, tc->request);
        IExec->AbortIO((struct IORequest *) tc->request);
        IExec->WaitIO((struct IORequest *) tc->request);
    }
}

ESignalType timer_wait_for_signal(uint32 timerSig, const char* const name)
{
    const uint32 wait = IExec->Wait(SIGBREAKF_CTRL_C | timerSig);

    if (wait & SIGBREAKF_CTRL_C) {
        puts("*** Control-C detected ***");
        return ESignalType_Break;
    }

    if (wait & timerSig) {
        printf("*** %s timer triggered ***\n", name);
    }

    return ESignalType_Timer;
}

void timer_delay(ULONG seconds)
{
    const ULONG micros = 0;

    TimerContext delayTimer;

    timer_init(&delayTimer);
    timer_start(&delayTimer, seconds, micros);

    timer_wait_for_signal(timer_signal(&delayTimer), "Delay");

    timer_stop(&delayTimer);
    timer_quit(&delayTimer);
}

double timer_ticks_to_s(const uint64 ticks)
{
    return (double)ticks / (double)frequency;
}

double timer_ticks_to_ms(const uint64 ticks)
{
    return 1000.0 * timer_ticks_to_s(ticks);
}

double timer_ticks_to_us(const uint64 ticks)
{
    return 1000000.0 * timer_ticks_to_s(ticks);
}

double timer_get_elapsed_seconds()
{
    double time = 0.0;

    if (ITimer) {
        struct MyClock now;

        ITimer->ReadEClock(&now.clockVal);

        const uint64 elapsed = now.ticks - start.ticks;
        time = timer_ticks_to_s(elapsed);
    }

    return time;
}

