#include <stdbool.h>
#include <stdio.h>
#include <empress/empress.h>
#include <unistd.h>

bool isPlaying = false;

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

int main(void)
{
    EmpContext* context;

    EmpApplicationInfo appInfo;
    appInfo.appUniqueName = "grumpse";

    EmpResult result = empCreate(&appInfo, &context);
    if (result != EMP_RESULT_OK)
        return 1;

    empSetButtonPressedCallback(context, ButtonPressedCallback);
    empSetPlayState(context, EMP_PLAY_STATE_PAUSED);

    EmpTrackMetadata metadata = {0};
    metadata.title = "Test Track";
    metadata.album = "Test Album";
    const char* artists[2] = { "Artist 1", "Artist 2" };
    metadata.numArtists = 2;
    metadata.artists = artists;
    metadata.length = 50 * 1000;
    const char* genres[2] = { "Dance", "Pop" };
    metadata.numGenres = 2;
    metadata.genres = genres;

    empSetTrackMetadata(context, &metadata);

    for (;;)
    {
        sleep(1);
    }

    empDestroy(context);

    return 0;
}