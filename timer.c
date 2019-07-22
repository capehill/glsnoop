#include "timer.h"
#include "logger.h"

#include <proto/exec.h>
#include <proto/timer.h>

#include <stdio.h>

static struct MsgPort* port = NULL;
static struct TimeRequest* request = NULL;
static BYTE device = -1;
static ULONG frequency = 0;

static void read_frequency(void)
{
    struct EClockVal clockVal;
    frequency = ITimer->ReadEClock(&clockVal);

    logLine("Timer frequency %lu ticks / second", frequency);
}

BOOL timer_init(void)
{
	port = IExec->AllocSysObjectTags(ASOT_PORT,
		ASOPORT_Name, "timer_port",
		TAG_DONE);

	if (!port) {
		puts("Couldn't create timer port");
		goto out;
	}

	request = IExec->AllocSysObjectTags(ASOT_IOREQUEST,
		ASOIOR_Size, sizeof(struct TimeRequest),
		ASOIOR_ReplyPort, port,
		TAG_DONE);

	if (!request) {
		puts("Couldn't create timer IO request");
		goto out;
	}

	device = IExec->OpenDevice(TIMERNAME, UNIT_WAITUNTIL,
		(struct IORequest *) request, 0);

	if (device) {
		puts("Couldn't open timer.device");
		goto out;
	}

	ITimer = (struct TimerIFace *) IExec->GetInterface(
		(struct Library *) request->Request.io_Device, "main", 1, NULL);

	if (!ITimer) {
		puts("Couldn't get Timer interface");
		goto out;
	}

    read_frequency();

    return TRUE;

out:
    timer_quit();

    return FALSE;
}

void timer_quit(void)
{
	if (ITimer) {
		IExec->DropInterface((struct Interface *) ITimer);
        ITimer = NULL;
	}

	if (device == 0 && request) {
		IExec->CloseDevice((struct IORequest *) request);
        device = -1;
	}

	if (request) {
		IExec->FreeSysObject(ASOT_IOREQUEST, request);
        request = NULL;
	}

	if (port) {
		IExec->FreeSysObject(ASOT_PORT, port);
        port = NULL;
	}
}

uint32 timer_signal(void)
{
    return 1L << port->mp_SigBit;
}

void timer_start(void)
{
    struct TimeVal tv;
	struct TimeVal increment;

	ITimer->GetSysTime(&tv);

	increment.Seconds = 1;
	increment.Microseconds = 0;

	ITimer->AddTime(&tv, &increment);

	request->Request.io_Command = TR_ADDREQUEST;
	request->Time.Seconds = tv.Seconds;
	request->Time.Microseconds = tv.Microseconds;

	IExec->SendIO((struct IORequest *) request);
}

void timer_handle_events(void)
{
	struct Message *msg;

	while ((msg = IExec->GetMsg(port))) {
		const int8 error = ((struct IORequest *)msg)->io_Error;

		if (error) {
			printf("Timer message received with code %d\n", error);
		}
	}

	timer_start();
}

void timer_stop(void)
{
	if (!IExec->CheckIO((struct IORequest *) request)) {
		IExec->AbortIO((struct IORequest *) request);
		IExec->WaitIO((struct IORequest *) request);
	}
}

ULONG timer_frequency_ms(void)
{
    return frequency / 1000;
}

ULONG timer_frequency(void)
{
    return frequency;
}

