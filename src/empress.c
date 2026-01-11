#include "empress/empress.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <systemd/sd-bus.h>

#define DBUS_PROPERTY_IMPL(FuncName) static int FuncName(sd_bus* bus, const char* path, const char* interface, const char* property, sd_bus_message* reply, void* userdata, sd_bus_error* ret_error)
#define DBUS_METHOD_IMPL(FuncName) static int FuncName(sd_bus_message *reply, void *userdata, sd_bus_error *ret_error)
#define DBUS_RETURN(Types, ...) return sd_bus_message_append(reply, Types, __VA_ARGS__)

#define DBUS_PROPERTY_CHANGED(Interface, Property) sd_bus_emit_properties_changed(ctx->bus, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2."Interface, Property, NULL);

#define DBUS_KEY_VALUE(Key, Type, Value) {\
    sd_bus_message_open_container(reply, 'e', "sv");\
    sd_bus_message_append(reply, "s", Key);\
    sd_bus_message_append(reply, "v", Type, Value);\
    sd_bus_message_close_container(reply);\
}

#define DBUS_OPEN_ARRAY(Key, Type) {\
    sd_bus_message_open_container(reply, 'e', "sv");\
    sd_bus_message_append(reply, "s", Key);\
    sd_bus_message_open_container(reply, 'v', "a"Type);\
    sd_bus_message_open_container(reply, 'a', Type);\
}

#define DBUS_CLOSE_ARRAY() {\
    sd_bus_message_close_container(reply);\
    sd_bus_message_close_container(reply);\
    sd_bus_message_close_container(reply);\
}

#define DBUS_ARRAY_VALUE(Type, Value) sd_bus_message_append(reply, Type, Value)

typedef struct
{
    EmpApplicationInfo appInfo;
    sd_bus* bus;
    pthread_t thread;
    sd_event* event;
    sd_bus_slot* mprisSlot;
    sd_bus_slot* playerSlot;

    EmpPlayState currentPlayState;
    EmpTrackMetadata trackMetadata;
    bool canPlay;
    bool canPause;
    bool canSeek;
    bool canGoNext;
    bool canGoPrevious;

    void(*focusCallback)(EmpContext*);
    void(*buttonPressedCallback)(EmpContext*, EmpButton);
    void(*seekCallback)(EmpContext*, size_t, long long);
    long long(*positionCallback)(EmpContext*);
} MprisContext;

const char* PlayStateToDBusString(const EmpPlayState state)
{
    switch (state)
    {
        case EMP_PLAY_STATE_STOPPED:
            return "Stopped";
        case EMP_PLAY_STATE_PAUSED:
            return "Paused";
        case EMP_PLAY_STATE_PLAYING:
            return "Playing";
        default:
            return "Stopped";
    }
}

DBUS_PROPERTY_IMPL(GetIdentity)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    const EmpApplicationInfo* appInfo = &ctx->appInfo;

    const char* appName = appInfo->appFriendlyName ? appInfo->appFriendlyName : appInfo->appUniqueName;
    DBUS_RETURN("s", appName);
}

DBUS_PROPERTY_IMPL(GetDesktopEntry)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    const EmpApplicationInfo* appInfo = &ctx->appInfo;

    const char* desktopEntry = appInfo->desktopEntry ? appInfo->desktopEntry : "";
    DBUS_RETURN("s", desktopEntry);
}

DBUS_PROPERTY_IMPL(GetCanRaise)
{
    DBUS_RETURN("b", 1);
}

DBUS_PROPERTY_IMPL(GetHasTrackList)
{
    DBUS_RETURN("b", 0);
}

DBUS_METHOD_IMPL(Raise)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->focusCallback)
        ctx->focusCallback((EmpContext*) ctx);
    DBUS_RETURN("", "");
}

DBUS_PROPERTY_IMPL(GetCanControl)
{
    DBUS_RETURN("b", 1);
}

DBUS_PROPERTY_IMPL(GetCanPlay)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    DBUS_RETURN("b", ctx->canPlay);
}

DBUS_PROPERTY_IMPL(GetCanPause)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    DBUS_RETURN("b", ctx->canPause);
}

DBUS_PROPERTY_IMPL(GetCanSeek)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    DBUS_RETURN("b", ctx->canSeek);
}

DBUS_PROPERTY_IMPL(GetCanGoNext)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    DBUS_RETURN("b", ctx->canGoNext);
}

DBUS_PROPERTY_IMPL(GetCanGoPrevious)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    DBUS_RETURN("b", ctx->canGoPrevious);
}

DBUS_PROPERTY_IMPL(GetPlaybackStatus)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    DBUS_RETURN("s", PlayStateToDBusString(ctx->currentPlayState));
}

DBUS_PROPERTY_IMPL(GetMetadata)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    const EmpTrackMetadata* metadata = &ctx->trackMetadata;

    sd_bus_message_open_container(reply, 'a', "{sv}");
    DBUS_KEY_VALUE("mpris:trackid", "o", "/org/mpris/MediaPlayer2/Track/0");

    if (metadata->trackNumber > 0)
        DBUS_KEY_VALUE("xesam:trackNumber", "x", metadata->trackNumber);

    if (metadata->title)
        DBUS_KEY_VALUE("xesam:title", "s", metadata->title);

    if (metadata->numArtists > 0 && metadata->artists)
    {
        DBUS_OPEN_ARRAY("xesam:artist", "s");

        for (int i = 0; i < metadata->numArtists; i++)
            DBUS_ARRAY_VALUE("s", metadata->artists[i]);

        DBUS_CLOSE_ARRAY();
    }

    if (metadata->album)
        DBUS_KEY_VALUE("xesam:album", "s", metadata->album);

    if (metadata->length > 0)
    {
        unsigned long int lengthInMicroseconds = metadata->length;
        DBUS_KEY_VALUE("mpris:length", "x", lengthInMicroseconds);
    }

    if (metadata->numGenres > 0 && metadata->genres)
    {
        DBUS_OPEN_ARRAY("xesam:genres", "s");

        for (int i = 0; i < metadata->numGenres; i++)
            DBUS_ARRAY_VALUE("s", metadata->genres[i]);

        DBUS_CLOSE_ARRAY();
    }

    if (metadata->imageUri)
        DBUS_KEY_VALUE("mpris:artUrl", "s", metadata->imageUri);

    return sd_bus_message_close_container(reply);
}

DBUS_PROPERTY_IMPL(GetPosition)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->positionCallback == NULL)
        DBUS_RETURN("x", 0);

    const long long value = ctx->positionCallback((EmpContext*) ctx);
    DBUS_RETURN("x", value);
}

DBUS_METHOD_IMPL(Play)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->buttonPressedCallback)
        ctx->buttonPressedCallback((EmpContext*) ctx, EMP_BUTTON_PLAY);
    DBUS_RETURN("", "");
}

DBUS_METHOD_IMPL(Pause)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->buttonPressedCallback)
        ctx->buttonPressedCallback((EmpContext*) ctx, EMP_BUTTON_PAUSE);
    DBUS_RETURN("", "");
}

DBUS_METHOD_IMPL(Stop)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->buttonPressedCallback)
        ctx->buttonPressedCallback((EmpContext*) ctx, EMP_BUTTON_STOP);
    DBUS_RETURN("", "");
}

DBUS_METHOD_IMPL(Next)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->buttonPressedCallback)
        ctx->buttonPressedCallback((EmpContext*) ctx, EMP_BUTTON_NEXT);
    DBUS_RETURN("", "");
}

DBUS_METHOD_IMPL(Previous)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->buttonPressedCallback)
        ctx->buttonPressedCallback((EmpContext*) ctx, EMP_BUTTON_PREVIOUS);
    DBUS_RETURN("", "");
}

DBUS_METHOD_IMPL(SetPosition)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->seekCallback)
    {
        size_t position;
        sd_bus_message_read(reply, "ox", NULL, &position);
        ctx->seekCallback((EmpContext*) ctx, position, 0);
    }
    DBUS_RETURN("", "");
}

DBUS_METHOD_IMPL(Seek)
{
    const MprisContext* ctx = (MprisContext*) userdata;
    if (ctx->seekCallback)
    {
        long int seekAmount;
        sd_bus_message_read(reply, "x", &seekAmount);
        ctx->seekCallback((EmpContext*) ctx, seekAmount, 0);
    }
    DBUS_RETURN("", "");
}

static const sd_bus_vtable MprisVtable[] = {
    SD_BUS_VTABLE_START(0),

    SD_BUS_PROPERTY("Identity", "s", GetIdentity, 0, 0),
    SD_BUS_PROPERTY("DesktopEntry", "s", GetDesktopEntry, 0, 0),
    SD_BUS_PROPERTY("CanRaise", "b", GetCanRaise, 0, 0),
    SD_BUS_PROPERTY("HasTrackList", "b", GetHasTrackList, 0, 0),
    SD_BUS_METHOD("Raise", "", "", Raise, SD_BUS_VTABLE_UNPRIVILEGED),

    SD_BUS_VTABLE_END
};

static const sd_bus_vtable PlayerVtable[] = {
    SD_BUS_VTABLE_START(0),

    SD_BUS_PROPERTY("CanControl", "b", GetCanControl, 0, SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_PROPERTY("CanPlay", "b", GetCanPlay, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanPause", "b", GetCanPause, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanSeek", "b", GetCanSeek, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanGoNext", "b", GetCanGoNext, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanGoPrevious", "b", GetCanGoPrevious, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("PlaybackStatus", "s", GetPlaybackStatus, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Metadata", "a{sv}", GetMetadata, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Position", "x", GetPosition, 0, 0),
    SD_BUS_METHOD("Play", "", "", Play, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Pause", "", "", Pause, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Stop", "", "", Stop, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Next", "", "", Next, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Previous", "", "", Previous, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SetPosition", "ox", "", SetPosition, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Seek", "x", "", Seek, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_SIGNAL("Seeked", "x", 0),

    SD_BUS_VTABLE_END
};

void* EventLoop(void* arg)
{
    sd_event_loop((sd_event*) arg);
    return NULL;
}

EmpResult empCreate(const EmpApplicationInfo *appInfo, EmpContext **context)
{
    MprisContext* ctx = malloc(sizeof(MprisContext));
    if (!ctx)
        return EMP_RESULT_UNKNOWN_ERROR;
    ctx->appInfo = *appInfo;
    ctx->currentPlayState = EMP_PLAY_STATE_STOPPED;
    ctx->canPlay = false;
    ctx->canPause = false;
    ctx->canSeek = false;
    ctx->canGoNext = false;
    ctx->canGoPrevious = false;
    memset(&ctx->trackMetadata, 0, sizeof(EmpTrackMetadata));
    ctx->focusCallback = NULL;
    ctx->buttonPressedCallback = NULL;
    ctx->seekCallback = NULL;
    ctx->positionCallback = NULL;

    if (sd_bus_open_user(&ctx->bus) < 0)
        return EMP_RESULT_UNKNOWN_ERROR;

    const char* baseName = "org.mpris.MediaPlayer2.";
    int baseLength = strlen(baseName);
    int appLength = strlen(appInfo->appUniqueName);

    char* appName = calloc((baseLength + appLength), sizeof(char*));
    strcat(appName, baseName);
    strcat(appName, appInfo->appUniqueName);

    printf("%s\n", appName);

    if (sd_bus_add_object_vtable(ctx->bus, &ctx->mprisSlot, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", MprisVtable, ctx) < 0)
    {
        free(appName);
        return EMP_RESULT_UNKNOWN_ERROR;
    }

    if (sd_bus_add_object_vtable(ctx->bus, &ctx->playerSlot, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", PlayerVtable, ctx) < 0)
    {
        free(appName);
        return EMP_RESULT_UNKNOWN_ERROR;
    }

    if (sd_bus_request_name(ctx->bus, appName, 0) < 0)
    {
        free(appName);
        return EMP_RESULT_INVALID_APP_NAME;
    }

    free(appName);

    if (sd_event_new(&ctx->event) < 0)
        return EMP_RESULT_UNKNOWN_ERROR;

    if (sd_bus_attach_event(ctx->bus, ctx->event, 0) < 0)
        return EMP_RESULT_UNKNOWN_ERROR;

    pthread_create(&ctx->thread, NULL, EventLoop, ctx->event);

    *context = (EmpContext*) ctx;

    return EMP_RESULT_OK;
}

void empDestroy(EmpContext *context)
{
    MprisContext* ctx = (MprisContext*) context;
    sd_event_exit(ctx->event, 0);
    sd_bus_detach_event(ctx->bus);
    sd_event_unref(ctx->event);
    sd_bus_slot_unref(ctx->playerSlot);
    sd_bus_slot_unref(ctx->mprisSlot);
    sd_bus_unref(ctx->bus);
    free(ctx);
}

void empSetFocusCallback(EmpContext* context, void(* callback)(EmpContext*))
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->focusCallback = callback;
}

void empSetButtonPressedCallback(EmpContext* context, void(*callback)(EmpContext*, EmpButton))
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->buttonPressedCallback = callback;
}

void empSetSeekCallback(EmpContext* context, void(*callback)(EmpContext*, size_t, long long))
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->seekCallback = callback;
}

void empSetPositionCallback(EmpContext *context, long long(*callback)(EmpContext*))
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->positionCallback = callback;
}

void empSetPlayPosition(EmpContext *context, const size_t position) {
    const MprisContext* ctx = (MprisContext*) context;
    sd_bus_emit_signal(ctx->bus, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Seeked", "x", position);
}

void empSetPlayState(EmpContext* context, EmpPlayState state)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->currentPlayState = state;
    const char* dbusState = PlayStateToDBusString(state);
    printf("%s\n", dbusState);
    DBUS_PROPERTY_CHANGED("Player", "PlaybackStatus");
}

void empSetTrackMetadata(EmpContext* context, EmpTrackMetadata* metadata)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->trackMetadata = *metadata;
    DBUS_PROPERTY_CHANGED("Player", "Metadata");
}

void empClearTrackMetadata(EmpContext* context)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->trackMetadata = (EmpTrackMetadata) { 0 };
    DBUS_PROPERTY_CHANGED("Player", "Metadata");
}

void empSetCanPlay(EmpContext* context, bool value)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->canPlay = value;
    DBUS_PROPERTY_CHANGED("Player", "CanPlay");
}

void empSetCanPause(EmpContext* context, bool value)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->canPause = value;
    DBUS_PROPERTY_CHANGED("Player", "CanPause");
}

void empSetCanSeek(EmpContext* context, bool value)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->canSeek = value;
    DBUS_PROPERTY_CHANGED("Player", "CanSeek");
}

void empSetCanGoNext(EmpContext* context, bool value)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->canGoNext = value;
    DBUS_PROPERTY_CHANGED("Player", "CanGoNext");
}

void empSetCanGoPrevious(EmpContext* context, bool value)
{
    MprisContext* ctx = (MprisContext*) context;
    ctx->canGoPrevious = value;
    DBUS_PROPERTY_CHANGED("Player", "CanGoPrevious");
}