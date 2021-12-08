#ifndef __QIOT_CMDSYS_H__
#define __QIOT_CMDSYS_H__
#include "Ql_iotApi.h"

enum
{
    QIOT_DPID_EXCE_ERRCODE = 1, /* �쳣������ */
    QIOT_DPID_EXCE_PKGID = 2,   /* �쳣��ID */
    QIOT_DPID_EXCE_PK = 3,      /* �쳣�豸pk */
    QIOT_DPID_EXCE_DK = 4,      /* �쳣�豸dk */
};

enum
{
    QIOT_DPID_DEV_DNS = 1,         /* DNS��Ϣ */
    QIOT_DPID_DEV_DOMAIN = 2,      /* DNS���� */
    QIOT_DPID_DEV_IP = 3,          /* DNS��ip */
    QIOT_DPID_DEV_PORT = 4,        /* DNS�Ķ˿� */
    QIOT_DPID_DEV_SECRET = 5,      /* �豸����Կ */
    QIOT_DPID_DEV_SESSIONKEY = 6,  /* �ỰsessionKey */
    QIOT_DPID_DEV_PK = 7,          /* product key */
    QIOT_DPID_DEV_PS = 8,          /* product Secret */
    QIOT_DPID_DEV_REAUTH = 9,      /* ��֤��ʶ */
    QIOT_DPID_DEV_NEW_SECRET = 10, /* �µ��豸��Կ */
    QIOT_DPID_DEV_BINDCODE = 11,   /* �豸����Ϣbindcode�ϱ��������о�����ͨ��ʱ��Ч */
};

enum
{
    QIOT_SERVER_ERRCODE_PROTOCOL = 1,
    QIOT_SERVER_ERRCODE_LEN = 2,
    QIOT_SERVER_ERRCODE_CRC = 3,
    QIOT_SERVER_ERRCODE_CMD = 4,
    QIOT_SERVER_ERRCODE_UNFORMAT_FAIL = 5,
};

#define QIOT_SERVER_ERRCODE_STRING(X)                                                                                                           \
    (                                                                                                                                           \
        (X == QIOT_SERVER_ERRCODE_PROTOCOL) ? "EXCE_ERRCODE_PROTOCOL" : (X == QIOT_SERVER_ERRCODE_LEN)          ? "EXCE_ERRCODE_LEN"            \
                                                                    : (X == QIOT_SERVER_ERRCODE_CRC)            ? "EXCE_ERRCODE_CRC"            \
                                                                    : (X == QIOT_SERVER_ERRCODE_CMD)            ? "EXCE_ERRCODE_CMD"            \
                                                                    : (X == QIOT_SERVER_ERRCODE_UNFORMAT_FAIL)  ? "EXCE_ERRCODE_UNFORMAT_FAIL"  \
                                                                    : (X == QIOT_SERVER_ERRCODE_RATE_LIMIT)     ? "EXCE_ERRCODE_RATE_LIMIT"     \
                                                                    : (X == QIOT_SERVER_ERRCODE_QUANTITY_LIMIT) ? "EXCE_ERRCODE_QUANTITY_LIMIT" \
                                                                                                                : "Unknown")

void Ql_iotCmdSysInit(void);
void Ql_iotCmdSysExceReport(QIot_dpAppType_e app, const char *endPoint, qint32_t errcode, quint16_t pkgId);
void Ql_iotCmdSysStatusRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
void Ql_iotCmdSysDevInfoRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
void Ql_iotCmdSysTerManage(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
void Ql_iotCmdSysExceWrite(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
#endif