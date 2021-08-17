#ifndef __QUOS_BASE64_H__
#define __QUOS_BASE64_H__
#include "quos_config.h"

#define QUOS_BASE64_DSTDATA_LEN(SRCLEN) (((SRCLEN+2)/3 << 2) + 1)
#if (SDK_ENABLE_BASE64 == 1)
quint16_t Quos_base64Encrypt(quint8_t *srcData, quint16_t srcLen, quint8_t *dstData);
quint16_t Quos_base64Decrypt(quint8_t *srcData, quint16_t srcLen, quint8_t *dstData);
#endif
#endif
