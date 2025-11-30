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

typedef struct
{
    const char *appUniqueName;
    const char *appFriendlyName;
} EmpApplicationInfo;

typedef struct EmpContext EmpContext;

EMP_EXPORT EmpResult empCreate(const EmpApplicationInfo *appInfo, EmpContext **context);
EMP_EXPORT void empDestroy(EmpContext *context);

#ifdef __cplusplus
}
#endif
#endif