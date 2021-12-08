#ifndef __QL_IOT_LOCATOR_H__
#define __QL_IOT_LOCATOR_H__
#include "Ql_iotApi.h"

char *Ql_iotLocatorTtlv2String(const void *ttlv);
qbool Ql_iotLocatoTitleClashCheck(const void *ttlvHead);

void FUNCTION_ATTR_ROM Ql_iotCmdLocDataReqRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
#endif