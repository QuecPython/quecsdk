#ifndef __QUOS_DATA_STORE_H__
#define __QUOS_DATA_STORE_H__
#include "quos_config.h"

#if (SDK_ENABLE_DATASAFE == 1)
qbool Quos_dsWrite(const char *filename,void *buf,quint16_t bufLen);
quint32_t Quos_dsRead(const char *filename, void **buf);
#endif
typedef struct
{
    quint8_t id;
    qbool isString;
    void *dat;
    quint16_t maxLen;
}dsKeyValue_t;
qbool Quos_dsKvRead(const char *filename,const dsKeyValue_t keyValueNode[]);
qbool Quos_dsKvWrite(const char *filename,const dsKeyValue_t keyValueNode[]);
#endif