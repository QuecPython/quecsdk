#ifndef __QIOT_LAN_H__
#define __QIOT_LAN_H__
#include "Ql_iotApi.h"
#ifdef QUEC_ENABLE_LAN

#define QIOT_LAN_PORT 40000

/* LAN TTLV ID */
enum
{
    QIOT_DPID_LAN_MAC = 1, /* MAC */
    QIOT_DPID_LAN_PK = 2,  /* PK */
};

qbool Ql_iotCmdLanInit(void);
void ql_iotLanDevDiscover(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
#endif
#endif
