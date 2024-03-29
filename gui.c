#include "gui.h"
#include "logger.h"
#include "ogles2_module.h"
#include "warp3dnova_module.h"
#include "timer.h"
#include "version.h"

#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>

#include <classes/requester.h>
#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <libraries/gadtools.h>

#include <stdio.h>

enum EObject {
    OID_Window,
    OID_AboutWindow,
    OID_TracingLayout,
    OID_Trace,
    OID_Pause,
    OID_ProfilingLayout,
    OID_StartProfiling,
    OID_FinishProfiling,
    OID_Ogles2Errors,
    OID_NovaErrors,
    OID_Count // KEEP LAST
};

enum EGadget {
    GID_Trace,
    GID_Pause,
    GID_StartProfiling,
    GID_FinishProfiling
};

typedef enum EMenu {
    MID_Iconify = 1,
    MID_About,
    MID_Quit
} EMenu;

static struct NewMenu menus[] = {
    { NM_TITLE, "glSnoop", NULL, 0, 0, NULL },
    { NM_ITEM, "Iconify", "I", 0, 0, (APTR)MID_Iconify },
    { NM_ITEM, "About...", "?", 0, 0, (APTR)MID_About },
    { NM_ITEM, "Quit", "Q", 0, 0, (APTR)MID_Quit },
    { NM_END, NULL, NULL, 0, 0, NULL }
};

static Object* objects[OID_Count];
static struct Window* window;
static struct MsgPort* port;

static const ULONG seconds = 1;
static const ULONG micros = 0;

TimerContext timer;

static struct ClassLibrary* WindowBase;
static struct ClassLibrary* RequesterBase;
static struct ClassLibrary* ButtonBase;
static struct ClassLibrary* LayoutBase;

static Class* WindowClass;
static Class* RequesterClass;
static Class* ButtonClass;
static Class* LayoutClass;

static void OpenClasses()
{
    const int version = 53;

    WindowBase = IIntuition->OpenClass("window.class", version, &WindowClass);
    if (!WindowBase) {
        logAlways("Failed to open window.class");
    }

    RequesterBase = IIntuition->OpenClass("requester.class", version, &RequesterClass);
    if (!RequesterBase) {
        logAlways("Failed to open requester.class");
    }

    ButtonBase = IIntuition->OpenClass("gadgets/button.gadget", version, &ButtonClass);
    if (!ButtonBase) {
        logAlways("Failed to open button.gadget");
    }

    LayoutBase = IIntuition->OpenClass("gadgets/layout.gadget", version, &LayoutClass);
    if (!LayoutBase) {
        logAlways("Failed to open layout.gadget");
    }
}

static void CloseClasses()
{
    IIntuition->CloseClass(WindowBase);
    IIntuition->CloseClass(RequesterBase);
    IIntuition->CloseClass(ButtonBase);
    IIntuition->CloseClass(LayoutBase);
}

static char* getApplicationName()
{
    #define maxPathLen 255

    static char pathBuffer[maxPathLen];

    if (IDOS->GetCliProgramName(pathBuffer, maxPathLen - 1)) {
        logLine("GetCliProgramName: '%s'", pathBuffer);
    } else {
        logLine("Failed to get CLI program name, checking task node");

        struct Task* me = IExec->FindTask(NULL);
        snprintf(pathBuffer, maxPathLen, "%s", ((struct Node *)me)->ln_Name);
    }

    logLine("Application name: '%s'", pathBuffer);

    return pathBuffer;
}

static struct DiskObject* getDiskObject()
{
    struct DiskObject *diskObject = NULL;

    BPTR oldDir = IDOS->SetCurrentDir(IDOS->GetProgramDir());
    diskObject = IIcon->GetDiskObject(getApplicationName());
    IDOS->SetCurrentDir(oldDir);

    return diskObject;
}

static void show_about_window()
{
    objects[OID_AboutWindow] = IIntuition->NewObject(RequesterClass, NULL,
        REQ_TitleText, "About glSnoop",
        REQ_BodyText, VERSION_STRING DATE_STRING,
        REQ_GadgetText, "_Ok",
        REQ_Image, REQIMAGE_INFO,
        TAG_DONE);

    if (objects[OID_AboutWindow]) {
        IIntuition->SetAttrs(objects[OID_Window], WA_BusyPointer, TRUE, TAG_DONE);
        IIntuition->IDoMethod(objects[OID_AboutWindow], RM_OPENREQ, NULL, window, NULL, TAG_DONE);
        IIntuition->SetAttrs(objects[OID_Window], WA_BusyPointer, FALSE, TAG_DONE);
        IIntuition->DisposeObject(objects[OID_AboutWindow]);
        objects[OID_AboutWindow] = NULL;
    }
}

static Object* create_gui(LONG profiling)
{
    return IIntuition->NewObject(WindowClass, NULL,
        WA_ScreenTitle, VERSION_STRING DATE_STRING,
        WA_Title, VERSION_STRING,
        WA_Activate, TRUE,
        WA_DragBar, TRUE,
        WA_CloseGadget, TRUE,
        WA_DepthGadget, TRUE,
        WA_SizeGadget, TRUE,
        WA_Width, 200,
        WA_Height, 50,
        WINDOW_Position, WPOS_CENTERMOUSE,
        WINDOW_IconifyGadget, TRUE,
        WINDOW_Icon, getDiskObject(),
        WINDOW_AppPort, port, // Iconification needs it
        WINDOW_GadgetHelp, TRUE,
        WINDOW_NewMenu, menus,
        WINDOW_Layout, IIntuition->NewObject(LayoutClass, NULL,
            LAYOUT_Orientation, LAYOUT_ORIENT_VERT,

            LAYOUT_AddChild, objects[OID_TracingLayout] = IIntuition->NewObject(LayoutClass, NULL,
                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                LAYOUT_Label, "Tracing",
                LAYOUT_BevelStyle, BVS_GROUP,
                LAYOUT_AddChild, objects[OID_Trace] = IIntuition->NewObject(ButtonClass, NULL,
                    GA_Text, "_Trace",
                    GA_ID, GID_Trace,
                    GA_RelVerify, TRUE, // TODO: required or not?
                    GA_Disabled, TRUE,
                    GA_HintInfo, "Enable function tracing to serial port",
                    TAG_DONE),
                LAYOUT_AddChild, objects[OID_Pause] = IIntuition->NewObject(ButtonClass, NULL,
                    GA_Text, "_Pause",
                    GA_ID, GID_Pause,
                    GA_RelVerify, TRUE, // TODO: required or not?
                    GA_Disabled, profiling ? TRUE : FALSE,
                    GA_HintInfo, "Disable function tracing to serial port",
                    TAG_DONE),
                TAG_DONE), // horizontal layout.gadget

            LAYOUT_AddChild, objects[OID_ProfilingLayout] = IIntuition->NewObject(LayoutClass, NULL,
                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                LAYOUT_Label, "Profiling",
                LAYOUT_BevelStyle, BVS_GROUP,
                LAYOUT_AddChild, objects[OID_StartProfiling] = IIntuition->NewObject(ButtonClass, NULL,
                    GA_Text, "_Start",
                    GA_ID, GID_StartProfiling,
                    GA_RelVerify, TRUE,
                    GA_Disabled, TRUE, // When glSnoop starts, it's "ready to profile", hence we will disable Start
                    GA_HintInfo, "Initialize profiling statistics",
                    TAG_DONE),
                LAYOUT_AddChild, objects[OID_FinishProfiling] = IIntuition->NewObject(ButtonClass, NULL,
                    GA_Text, "_Finish",
                    GA_ID, GID_FinishProfiling,
                    GA_RelVerify, TRUE,
                    GA_Disabled, FALSE,
                    GA_HintInfo, "Send profiling statistics to serial port",
                    TAG_DONE),
                TAG_DONE), // horizontal layout.gadget

            LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL,
                LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                LAYOUT_Label, "Information",
                LAYOUT_BevelStyle, BVS_GROUP,
                LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL,
                    GA_ReadOnly, TRUE,
                    GA_Text, ogles2_version_string(),
                    BUTTON_BevelStyle, BVS_NONE,
                    BUTTON_Transparent, TRUE,
                    TAG_DONE),
                LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL,
                    GA_ReadOnly, TRUE,
                    GA_Text, warp3dnova_version_string(),
                    BUTTON_BevelStyle, BVS_NONE,
                    BUTTON_Transparent, TRUE,
                    TAG_DONE),
                LAYOUT_AddChild, objects[OID_Ogles2Errors] = IIntuition->NewObject(ButtonClass, NULL,
                    GA_ReadOnly, TRUE,
                    GA_Text, ogles2_errors_string(),
                    BUTTON_BevelStyle, BVS_NONE,
                    BUTTON_Transparent, TRUE,
                    TAG_DONE),
                LAYOUT_AddChild, objects[OID_NovaErrors] = IIntuition->NewObject(ButtonClass, NULL,
                    GA_ReadOnly, TRUE,
                    GA_Text, warp3dnova_errors_string(),
                    BUTTON_BevelStyle, BVS_NONE,
                    BUTTON_Transparent, TRUE,
                    TAG_DONE),
                TAG_DONE), // vertical layout.gadget

            TAG_DONE), // vertical layout.gadget
        TAG_DONE); // window.class
}

static void refresh_object(Object * object)
{
    IIntuition->RefreshGList((struct Gadget *)object, window, NULL, -1);
}

static void refresh_tracing_buttons(void)
{
    refresh_object(objects[OID_TracingLayout]);
}

static void refresh_profiling_buttons(void)
{
    refresh_object(objects[OID_ProfilingLayout]);
}

static void refresh_errors(void)
{
    IIntuition->SetAttrs(objects[OID_Ogles2Errors], GA_Text, ogles2_errors_string(), TAG_DONE);
    IIntuition->SetAttrs(objects[OID_NovaErrors], GA_Text, warp3dnova_errors_string(), TAG_DONE);

    refresh_object(objects[OID_Ogles2Errors]);
    refresh_object(objects[OID_NovaErrors]);
}

static void trace(void)
{
    IIntuition->SetAttrs(objects[OID_Trace], GA_Disabled, TRUE, TAG_DONE);
    IIntuition->SetAttrs(objects[OID_Pause], GA_Disabled, FALSE, TAG_DONE);

    refresh_tracing_buttons();
    resume_log();
}

static void pause(void)
{
    IIntuition->SetAttrs(objects[OID_Trace], GA_Disabled, FALSE, TAG_DONE);
    IIntuition->SetAttrs(objects[OID_Pause], GA_Disabled, TRUE, TAG_DONE);

    refresh_tracing_buttons();
    pause_log();
}

static void start_profiling(void)
{
    static unsigned counter = 0;

    IIntuition->SetAttrs(objects[OID_StartProfiling], GA_Disabled, TRUE, TAG_DONE);
    IIntuition->SetAttrs(objects[OID_FinishProfiling], GA_Disabled, FALSE, TAG_DONE);

    refresh_profiling_buttons();

    logAlways("Profiling started by user (#%u) - clearing statistics", ++counter);

    ogles2_start_profiling();
    warp3dnova_start_profiling();
}

static void finish_profiling(void)
{
    static unsigned counter = 0;

    IIntuition->SetAttrs(objects[OID_StartProfiling], GA_Disabled, FALSE, TAG_DONE);
    IIntuition->SetAttrs(objects[OID_FinishProfiling], GA_Disabled, TRUE, TAG_DONE);

    refresh_profiling_buttons();

    logAlways("Profiling finished by user (#%u) - displaying statistics", ++counter);

    warp3dnova_finish_profiling();
    ogles2_finish_profiling();
}

static void handle_gadgets(int id)
{
    //printf("Gadget %d\n", id);

    switch (id) {
        case GID_Trace:
            trace();
            break;
        case GID_Pause:
            pause();
            break;
        case GID_StartProfiling:
            start_profiling();
            break;
        case GID_FinishProfiling:
            finish_profiling();
            break;
    }
}

static void handle_iconify(void)
{
    window = NULL;
    IIntuition->IDoMethod(objects[OID_Window], WM_ICONIFY);
}

static void handle_uniconify(void)
{
    window = (struct Window *)IIntuition->IDoMethod(objects[OID_Window], WM_OPEN);
}

static BOOL handle_menupick(uint16 menuNumber)
{
    struct MenuItem* item = IIntuition->ItemAddress(window->MenuStrip, menuNumber);

    if (item) {
        const EMenu id = (EMenu)GTMENUITEM_USERDATA(item);
        //printf("menu %x, menu num %d, item num %d, userdata %d\n", menuNumber, MENUNUM(menuNumber), ITEMNUM(menuNumber), (EMenu)GTMENUITEM_USERDATA(item));
        switch (id) {
            case MID_Iconify: handle_iconify(); break;
            case MID_About: show_about_window(); break;
            case MID_Quit: return FALSE;
        }
    }

    return TRUE;
}

static void handle_events(void)
{
    uint32 signal = 0;
    IIntuition->GetAttr(WINDOW_SigMask, objects[OID_Window], &signal);

    const uint32 timerSignal = timer_signal(&timer);

    BOOL running = TRUE;

    while (running) {
        uint32 wait = IExec->Wait(signal | SIGBREAKF_CTRL_C | timerSignal);

        if (wait & SIGBREAKF_CTRL_C) {
            puts("*** Break ***");
            running = FALSE;
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
                    case WMHI_ICONIFY:
                        handle_iconify();
                        break;
                    case WMHI_UNICONIFY:
                        handle_uniconify();
                        break;
                    case WMHI_MENUPICK:
                        running = handle_menupick(result & WMHI_MENUMASK);
                        break;
                }
            }
        }

        if (wait & timerSignal) {
            timer_handle_events(&timer);
            if (window) {
                refresh_errors();
            }
            timer_start(&timer, seconds, micros);
        }
    }
}

// When profiling, Pause/Resume buttons are disabled
void run_gui(LONG profiling)
{
    OpenClasses();

	port = IExec->AllocSysObjectTags(ASOT_PORT,
		ASOPORT_Name, "app_port",
		TAG_DONE);

    objects[OID_Window] = create_gui(profiling);

    if (objects[OID_Window]) {
        if ((window = (struct Window *)IIntuition->IDoMethod(objects[OID_Window], WM_OPEN))) {
            timer_start(&timer, seconds, micros);
            handle_events();
        } else {
            puts("Failed to open window");
        }

        timer_stop(&timer);

        IIntuition->DisposeObject(objects[OID_Window]);
    } else {
        puts("Failed to create window");
    }

    if (port) {
        IExec->FreeSysObject(ASOT_PORT, port);
    }

    CloseClasses();
}
