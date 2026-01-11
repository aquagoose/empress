#ifndef EMPRESS_H
#define EMPRESS_H
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EMP_EXPORT

typedef enum
{
    EMP_RESULT_OK,
    EMP_RESULT_UNKNOWN_ERROR,
    EMP_RESULT_INVALID_APP_NAME
} EmpResult;

typedef enum
{
    EMP_PLAY_STATE_STOPPED,
    EMP_PLAY_STATE_PAUSED,
    EMP_PLAY_STATE_PLAYING
} EmpPlayState;

typedef enum
{
    EMP_BUTTON_PLAY,
    EMP_BUTTON_PAUSE,
    EMP_BUTTON_STOP,
    EMP_BUTTON_NEXT,
    EMP_BUTTON_PREVIOUS
} EmpButton;

typedef struct
{
    const char* appUniqueName;
    const char* appFriendlyName;
    const char* desktopEntry;
} EmpApplicationInfo;

typedef struct
{
    int trackNumber;
    const char* title;
    int numArtists;
    const char** artists;
    const char* album;
    size_t length;
    int numGenres;
    const char** genres;
    const char* imageUri;

} EmpTrackMetadata;

typedef struct EmpContext EmpContext;

EMP_EXPORT EmpResult empCreate(const EmpApplicationInfo *appInfo, EmpContext **context);
EMP_EXPORT void empDestroy(EmpContext *context);

EMP_EXPORT void empSetFocusCallback(EmpContext *context, void(*callback)(EmpContext*));
EMP_EXPORT void empSetButtonPressedCallback(EmpContext *context, void(*callback)(EmpContext*, EmpButton));
EMP_EXPORT void empSetSeekCallback(EmpContext *context, void(*callback)(EmpContext*, size_t, long long));
EMP_EXPORT void empSetPositionCallback(EmpContext *context, long long(*callback)(EmpContext*));

EMP_EXPORT void empSetPlayPosition(EmpContext *context, size_t position);
EMP_EXPORT void empSetPlayState(EmpContext *context, EmpPlayState state);
EMP_EXPORT void empSetTrackMetadata(EmpContext *context, EmpTrackMetadata *metadata);
EMP_EXPORT void empClearTrackMetadata(EmpContext *context);

EMP_EXPORT void empSetCanPlay(EmpContext *context, bool value);
EMP_EXPORT void empSetCanPause(EmpContext *context, bool value);
EMP_EXPORT void empSetCanSeek(EmpContext *context, bool value);
EMP_EXPORT void empSetCanGoNext(EmpContext *context, bool value);
EMP_EXPORT void empSetCanGoPrevious(EmpContext *context, bool value);

#ifdef __cplusplus
}
#endif
#endif