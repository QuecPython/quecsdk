#ifndef __QHAL_FLASHOPT_H__
#define __QHAL_FLASHOPT_H__
#include "Qhal_types.h"

#if defined (PLAT_Unisoc)
#define QIOT_FILE_CONFIG "UFS:qiot_config.ini"
#define QIOT_FILE_OTA "UFS:fota.pack"
#elif defined (PLAT_ASR)
#define QIOT_FILE_CONFIG "U:/qiot_config.ini"
#define QIOT_FILE_OTA "U:/qiot_ota.bin"
#else
#define QIOT_FILE_CONFIG "qiot_config.ini"
#define QIOT_FILE_OTA "qiot_ota.bin"
#endif	


pointer_t Qhal_fileOpen(const char *filename, qbool onlyRead);
qbool Qhal_fileWrite(pointer_t sockFd,quint32_t offset,void *buf,quint16_t bufLen);
quint32_t Qhal_fileRead(pointer_t sockFd,quint32_t offset,void *buf,quint16_t bufLen);
void Qhal_fileClose(pointer_t sockFd);
qbool Qhal_fileErase(const char *filename);
#endif
