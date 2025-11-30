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

    for (;;)
    {
        sleep(1);
    }

    empDestroy(context);

    return 0;
}