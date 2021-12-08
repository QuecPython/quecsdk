#ifndef __QUOS_DATA_STORE_H__
#define __QUOS_DATA_STORE_H__
#include "quos_config.h"

#if (SDK_ENABLE_DATASAFE == 1)
qbool Quos_dsWrite(const char *filename, const void *buf, quint16_t bufLen, const char *aesKey);
quint32_t Quos_dsRead(const char *filename, void **buf, const char *aesKey);
#endif
typedef struct
{
    quint8_t id;
    qbool isString;
    void *dat;
    quint16_t maxLen;
} dsKeyValue_t;
qbool Quos_dsKvRead(const char *filename, const dsKeyValue_t keyValueNode[], const char *aesKey);
qbool Quos_dsKvWrite(const char *filename, const dsKeyValue_t keyValueNode[], const char *aesKey);
#endif