#include "empress/empress.h"

#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>

typedef struct
{
    sd_bus* bus;
    sd_event* event;
} MprisContext;

EmpResult empCreate(const EmpApplicationInfo *appInfo, EmpContext **context)
{
    MprisContext* ctx = malloc(sizeof(MprisContext));
    if (!ctx)
        return EMP_RESULT_UNKNOWN_ERROR;

    if (sd_bus_open_user(&ctx->bus) < 0)
        return EMP_RESULT_UNKNOWN_ERROR;

    const char* baseName = "org.mpris.MediaPlayer2.";
    int baseLength = strlen(baseName);
    int appLength = strlen(appInfo->appName);

    char* appName[baseLength + appLength];
    strcat(appName, baseName);
    strcat(appName, appInfo->appName);

    if (sd_bus_request_name(ctx->bus, appName, 0) < 0)
        return EMP_RESULT_INVALID_APP_NAME;

    if (sd_event_new(&ctx->event) < 0)
        return EMP_RESULT_UNKNOWN_ERROR;

    if (sd_bus_attach_event(ctx->bus, ctx->event, 0) < 0)
        return EMP_RESULT_UNKNOWN_ERROR;

    *context = ctx;

    return EMP_RESULT_OK;
}

void empDestroy(EmpContext *context)
{
    MprisContext* ctx = (MprisContext*) context;
    sd_bus_detach_event(ctx->bus);
    sd_event_unref(ctx->event);
    sd_bus_unref(ctx->bus);
    free(ctx);
}
