#ifndef EMPRESS_H
#define EMPRESS_H

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
    const char *appUniqueName;
    const char *appFriendlyName;
} EmpApplicationInfo;

typedef struct
{
    const char* title;
    int numArtists;
    const char** artists;
    const char* album;
    unsigned long int length;
    int numGenres;
    const char** genres;

} EmpTrackMetadata;

typedef struct EmpContext EmpContext;

EMP_EXPORT EmpResult empCreate(const EmpApplicationInfo *appInfo, EmpContext **context);
EMP_EXPORT void empDestroy(EmpContext *context);

EMP_EXPORT void empSetButtonPressedCallback(EmpContext *context, void(*callback)(EmpContext*, EmpButton));

EMP_EXPORT void empSetPlayState(EmpContext *context, EmpPlayState state);
EMP_EXPORT void empSetTrackMetadata(EmpContext *context, EmpTrackMetadata *metadata);

#ifdef __cplusplus
}
#endif
#endif