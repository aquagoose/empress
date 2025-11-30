#include <empress/empress.h>
#include <unistd.h>

int main(void)
{
    EmpContext* context;

    EmpApplicationInfo appInfo;
    appInfo.appName = "grumpse";

    EmpResult result = empCreate(&appInfo, &context);
    if (result != EMP_RESULT_OK)
        return 1;

    for (;;)
    {
        sleep(1);
    }

    empDestroy(context);

    return 0;
}