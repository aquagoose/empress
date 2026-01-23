#include <stdbool.h>
#include <stdio.h>
#include <empress/empress.h>
#include <unistd.h>

bool isPlaying = false;
bool loopPlaylist = false;
bool loopTrack = false;

void ButtonPressedCallback(EmpContext* context, EmpButton button)
{
    switch (button)
    {
        case EMP_BUTTON_PLAY:
            isPlaying = true;
            break;
        case EMP_BUTTON_PAUSE:
            isPlaying = false;
            break;
    }

    empSetPlayState(context, isPlaying ? EMP_PLAY_STATE_PLAYING : EMP_PLAY_STATE_PAUSED);
}

void loopChangedCallback(EmpContext* context, EmpLoopState loopState) {
    switch (loopState) {
        case EMP_LOOP_NONE:
            loopPlaylist = false;
            loopTrack = false;
        case EMP_LOOP_PLAYLIST:
            loopPlaylist = true;
            loopTrack = false;
        case EMP_LOOP_TRACK:
            loopPlaylist = false;
            loopTrack = true;
    }

    empSetLoopState(context, loopState);
}

void SeekCallback(EmpContext* context, size_t position, long long seek)
{
    printf("%lu\n", position);
    printf("%lu\n", seek);
}

void FocusCallback(EmpContext* context)
{
    printf("Focus!\n");
}

int main(void)
{
    EmpContext* context;

    EmpApplicationInfo appInfo;
    appInfo.appUniqueName = "test";
    appInfo.appFriendlyName = "Test";
    //appInfo.desktopEntry = "/home/aqua/.local/share/applications/Glimpse";

    EmpResult result = empCreate(&appInfo, &context);
    if (result != EMP_RESULT_OK)
        return 1;

    empSetCanPlay(context, true);
    empSetCanPause(context, true);
    empSetCanSeek(context, true);
    empSetCanGoNext(context, true);
    empSetCanGoPrevious(context, true);
    empSetPlayState(context, EMP_PLAY_STATE_PAUSED);
    empSetFocusCallback(context, FocusCallback);
    empSetButtonPressedCallback(context, ButtonPressedCallback);
    empSetLoopChangedCallback(context, loopChangedCallback);
    empSetSeekCallback(context, SeekCallback);

    EmpTrackMetadata metadata = {0};
    metadata.trackNumber = 1;
    metadata.title = "Test Track";
    metadata.album = "Test Album";
    const char* artists[2] = { "Artist 1", "Artist 2" };
    metadata.numArtists = 2;
    metadata.artists = artists;
    metadata.length = 50;
    const char* genres[2] = { "Dance", "Pop" };
    metadata.numGenres = 2;
    metadata.genres = genres;
    metadata.imageUri = "file:///home/aqua/Music/Music/Tame Impala/Currents/Tame Impala - Currents.jpg";

    empSetTrackMetadata(context, &metadata);

    for (;;)
    {
        sleep(1);
    }

    empDestroy(context);

    return 0;
}