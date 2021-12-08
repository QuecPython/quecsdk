#ifndef __QL_IOTTTLV_H__
#define __QL_IOTTTLV_H__
#include "Ql_iotApi.h"

void *Ql_iotTtlvUnformat(const quint8_t *buffer, quint32_t len);
quint32_t Ql_iotTtlvFormatLen(const void *ttlvHead);
quint32_t Ql_iotTtlvFormat(const void *ttlvHead, quint8_t *retBuf);

#endif