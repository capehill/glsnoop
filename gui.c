#include "gui.h"
#include "common.h"

#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>

#include <stdio.h>

enum EObject {
    OID_Window,
    OID_Layout,
    OID_Trace,
    OID_Pause,
    OID_Count
};

enum EGadget {
    GID_Trace,
    GID_Pause
};

static Object* objects[OID_Count];
static struct Window* window;

static Object* create_gui(void)
{
    return IIntuition->NewObject(NULL, "window.class",
        WA_ScreenTitle, "glSnoop",
        WA_Title, "glSnoop",
        WA_Activate, TRUE,
        WA_DragBar, TRUE,
        WA_CloseGadget, TRUE,
        WA_DepthGadget, TRUE,
        WA_SizeGadget, TRUE,
        WA_Width, 200,
        WA_Height, 50,
        WINDOW_Position, WPOS_CENTERMOUSE,
        WINDOW_Layout, IIntuition->NewObject(NULL, "layout.gadget",
            LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
            LAYOUT_AddChild, objects[OID_Layout] = IIntuition->NewObject(NULL, "layout.gadget",
                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                LAYOUT_AddChild, objects[OID_Trace] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_Text, "Trace",
                    GA_ID, GID_Trace,
                    GA_RelVerify, TRUE, // TODO: required or not?
                    GA_Disabled, TRUE,
                    TAG_DONE),
                LAYOUT_AddChild, objects[OID_Pause] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_Text, "Pause",
                    GA_ID, GID_Pause,
                    GA_RelVerify, TRUE, // TODO: required or not?
                    TAG_DONE),
                TAG_DONE),
            TAG_DONE),
        TAG_DONE);
}

static void refresh(void)
{
    IIntuition->RefreshGList((struct Gadget *)objects[OID_Layout], window, NULL, -1);
}

static void on_trace(void)
{
    IIntuition->SetAttrs(objects[OID_Trace], GA_Disabled, TRUE, TAG_DONE);
    IIntuition->SetAttrs(objects[OID_Pause], GA_Disabled, FALSE, TAG_DONE);

    refresh();
    unpause_log();
}

static void on_pause()
{
    IIntuition->SetAttrs(objects[OID_Trace], GA_Disabled, FALSE, TAG_DONE);
    IIntuition->SetAttrs(objects[OID_Pause], GA_Disabled, TRUE, TAG_DONE);

    refresh();
    pause_log();
}

static void handle_gadgets(int id)
{
    //printf("Gadget %d\n", id);

    switch (id) {
        case GID_Trace:
            on_trace();
            break;
        case GID_Pause:
            on_pause();
            break;
    }
}

static void handle_events(void)
{
    uint32 signal = 0;
    IIntuition->GetAttr(WINDOW_SigMask, objects[OID_Window], &signal);

    BOOL running = TRUE;

    while (running) {
        uint32 wait = IExec->Wait(signal | SIGBREAKF_CTRL_C);

        if (wait & SIGBREAKF_CTRL_C) {
            puts("*** Break ***");
            running = FALSE;
            break;
        }

        if (wait & signal) {
            uint32 result;
            int16 code = 0;

            while ((result = IIntuition->IDoMethod(objects[OID_Window], WM_HANDLEINPUT, &code)) != WMHI_LASTMSG) {
                switch (result & WMHI_CLASSMASK) {
                    case WMHI_CLOSEWINDOW:
                        running = FALSE;
                        break;
                    case WMHI_GADGETUP:
                        handle_gadgets(result & WMHI_GADGETMASK);
                        break;
                }
            }
        }
    }
}

void run_gui(void)
{
    objects[OID_Window] = create_gui();

    if (objects[OID_Window]) {
        if ((window = (struct Window *)IIntuition->IDoMethod(objects[OID_Window], WM_OPEN))) {
            handle_events();
        } else {
            puts("Failed to open window");
        }

        IIntuition->DisposeObject(objects[OID_Window]);
    } else {
        puts("Failed to create window");
    }
}
