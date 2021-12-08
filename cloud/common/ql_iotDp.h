#ifndef __QUOS_DATA_PACKET_H__
#define __QUOS_DATA_PACKET_H__
#include "Ql_iotApi.h"

enum
{
    QIOT_DPCMD_TSL_REQ = 0X0011,              /* ��ģ��״̬��ȡ */
    QIOT_DPCMD_TSL_RSP = 0X0012,              /* ��ģ��״̬�ϱ�-�ظ� */
    QIOT_DPCMD_TSL_WRITE = 0X0013,            /* ��ģ�������·� */
    QIOT_DPCMD_TSL_EVENT = 0X0014,            /* ��ģ�������ϱ� */
    QIOT_DPCMD_PASS_WRITE = 0X0023,           /* ͸�������·� */
    QIOT_DPCMD_PASS_EVENT = 0X0024,           /* ͸�������Ϸ� */
    QIOT_DPCMD_STATUS_REQ = 0X0031,           /* �豸״̬��ȡ */
    QIOT_DPCMD_STATUS_RSP = 0X0032,           /* �豸״̬�ϱ�-�ظ� */
    QIOT_DPCMD_STATUS_EVENT = 0X0034,         /* �豸״̬�ϱ� */
    QIOT_DPCMD_INFO_REQ = 0X0041,             /* ģ����Ϣ��ȡ */
    QIOT_DPCMD_INFO_RSP = 0X0042,             /* ģ����Ϣ�ϱ�-�ظ� */
    QIOT_DPCMD_INFO_EVENT = 0X0044,           /* ģ����Ϣ�ϱ� */
    QIOT_DPCMD_EXCE_WRITE = 0X00A3,           /* �쳣֪ͨ */
    QIOT_DPCMD_EXCE_EVENT = 0X00A4,           /* �쳣֪ͨ */
    QIOT_DPCMD_DEV_MANAGER_WRITE = 0X00B3,    /* �豸�������� */
    QIOT_DPCMD_DEV_CONFIG_WRITE = 0X00B4,     /* �豸�������� */
    QIOT_DPCMD_DEV_BINDCODE_WRITE = 0X00B5,   /* �豸�ϱ�����Ϣ�������о�����ͨ��ʱ��Ч */
    QIOT_DPCMD_OTA_NOTIFY = 0X0111,           /* OTA��������֪ͨ */
    QIOT_DPCMD_OTA_COMFIRM = 0X0112,          /* OTAȷ��-OTA��������֪ͨȷ���Ƿ�Ҫ���� */
    QIOT_DPCMD_OTA_FW_INFO = 0X0113,          /* �̼���Ϣ�·� */
    QIOT_DPCMD_OTA_EVENT = 0X0114,            /* OTA����״̬�ϱ� */
    QIOT_DPCMD_OTA_REQUEST = 0X115,           /* OTA���� */
    QIOT_DPCMD_LOC_CFG_WRITE = 0X0121,        /* ���ö�λ��Ϣ�ϱ�����,����֧�� */
    QIOT_DPCMD_LOC_REPORT = 0X0122,           /* ��λ��Ϣ�ϱ� */
    QIOT_DPCMD_LOC_REQ = 0X0123,              /* ��ȡʵʱ��λ��Ϣ */
    QIOT_DPCMD_LOC_RSP = 0X0124,              /* ��ȡʵʱ��λ��Ϣ-�ظ� */
    QIOT_DPCMD_LAN_SEND_ACK = 0X7000,         /* Ӧ�������ͨ�ŵ���������൱��mqtt puback */
    QIOT_DPCMD_LAN_DISCOVER_REQ = 0X7001,     /* ���־��������豸 */
    QIOT_DPCMD_LAN_DISCOVER_RSP = 0X7002,     /* Ӧ���־��������豸 */
    QIOT_DPCMD_LAN_AUTH_COND_REQ = 0X7003,    /* ��������֤ǰ���������� */
    QIOT_DPCMD_LAN_AUTH_COND_RSP = 0X7004,    /* ��������֤ǰ������Ӧ�� */
    QIOT_DPCMD_LAN_AUTH_SIGN_REQ = 0X7005,    /* ��������֤ǩ������ */
    QIOT_DPCMD_LAN_AUTH_SIGN_RSP = 0X7006,    /* ��������֤ǩ��Ӧ�� */
    QIOT_DPCMD_SUB_AUTH = 0x00C0,             /* ���豸��ָ֤�� */
    QIOT_DPCMD_SUB_AUTH_RSP = 0x00C1,         /* ���豸��֤�ظ� */
    QIOT_DPCMD_SUB_LOGIN = 0x00C2,            /* ���豸��½ָ�� */
    QIOT_DPCMD_SUB_LOGIN_RSP = 0x00C3,        /* ���豸��½���� */
    QIOT_DPCMD_SUB_LOGOUT = 0x00C4,           /* ���豸�ǳ� */
    QIOT_DPCMD_SUB_LOGOUT_RSP = 0x00C5,       /* ���豸�ǳ��ظ� */
    QIOT_DPCMD_SUB_UNAUTH_EVENT = 0x00C6,     /* ���豸ע�� */
    QIOT_DPCMD_SUB_UNAUTH_EVENT_RSP = 0x00C7, /* ���豸ע������ */
    QIOT_DPCMD_SUB_OFFLINE_EVENT = 0x00C8,    /* ���豸����֪ͨ */
};

typedef struct
{
    quint16_t head;
    quint8_t sum;
    quint16_t pkgId;
    quint16_t cmd;
    quint16_t payloadLen;
    quint8_t *payload;
} QIot_dpPackage_t;

typedef qbool (*QIot_dpAppSend_ftemp)(const char *endPoint, QIot_dpPackage_t dpPkg, socketRecvNodeCb_f recvCB);
typedef qbool (*QIot_dpAppSend_f)(const char *endPoint, quint16_t mode, quint16_t cmd, quint16_t srcpkgId, quint16_t pkgId, const quint8_t *payload, quint32_t payloadLen, socketRecvNodeCb_f recvCB);
typedef struct
{
    QIot_dpAppType_e app;
    QIot_dpAppSend_f send;
} QIot_dpAppSend_t;

typedef void (*QIot_dpCmdHandle_f)(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
typedef struct
{
    quint16_t cmd;
    QIot_dpCmdHandle_f cmdHandle;
} QIot_cmdTable_t;

quint32_t Ql_iotDpFormat(quint8_t **buf, quint16_t pId, quint16_t cmd, const quint8_t *payload, quint32_t payloadLen);
quint16_t Ql_iotDpPkgIdUpdate(quint16_t *pkgId);
qbool Ql_iotDpRawDataPickup(quint8_t *buf, quint32_t len, QIot_dpPackage_t *pkg);
void Ql_iotDpHandle(QIot_dpAppType_e app, const char *endPoint, quint8_t *payload, quint32_t payloadLen);
qbool Ql_iotDpSendCommonReq(QIot_dpAppType_e app, const char *endPoint, quint16_t srcPkgId, quint16_t mode, quint16_t cmd, const quint8_t *payload, quint32_t payloadLen, socketRecvNodeCb_f recvCB);
qbool Ql_iotDpSendCommonRsp(QIot_dpAppType_e app, const char *endPoint, quint16_t cmd, quint16_t pkgId, const quint8_t *payload, quint32_t payloadLen);
qbool Ql_iotDpSendTtlvReq(QIot_dpAppType_e app, const char *endPoint, quint16_t srcPkgId, quint16_t mode, quint16_t cmd, const void *ttlvHead, socketRecvNodeCb_f recvCB);
qbool Ql_iotDpSendTtlvRsp(QIot_dpAppType_e app, const char *endPoint, quint16_t cmd, quint16_t pkgId, const void *ttlvHead);
#endif
