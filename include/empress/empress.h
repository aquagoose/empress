#ifndef EMPRESS_H
#define EMPRESS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EMP_RESULT_OK
} EmpResult;

typedef struct
{
    const char *appName;
} EmpApplicationInfo;

typedef struct EmpContext EmpContext;

EmpResult empCreate(const EmpApplicationInfo *appInfo, EmpContext *context);
void empDestroy(EmpContext *context);

#ifdef __cplusplus
}
#endif
#endif