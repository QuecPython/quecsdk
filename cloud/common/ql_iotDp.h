#ifndef __QUOS_DATA_PACKET_H__
#define __QUOS_DATA_PACKET_H__
#include "Ql_iotApi.h"

enum
{
    QIOT_DPCMD_TSL_REQ = 0X0011,              /* 物模型状态获取 */
    QIOT_DPCMD_TSL_RSP = 0X0012,              /* 物模型状态上报-回复 */
    QIOT_DPCMD_TSL_WRITE = 0X0013,            /* 物模型数据下发 */
    QIOT_DPCMD_TSL_EVENT = 0X0014,            /* 物模型数据上报 */
    QIOT_DPCMD_PASS_WRITE = 0X0023,           /* 透传数据下发 */
    QIOT_DPCMD_PASS_EVENT = 0X0024,           /* 透传数据上发 */
    QIOT_DPCMD_STATUS_REQ = 0X0031,           /* 设备状态获取 */
    QIOT_DPCMD_STATUS_RSP = 0X0032,           /* 设备状态上报-回复 */
    QIOT_DPCMD_STATUS_EVENT = 0X0034,         /* 设备状态上报 */
    QIOT_DPCMD_INFO_REQ = 0X0041,             /* 模组信息获取 */
    QIOT_DPCMD_INFO_RSP = 0X0042,             /* 模组信息上报-回复 */
    QIOT_DPCMD_INFO_EVENT = 0X0044,           /* 模组信息上报 */
    QIOT_DPCMD_EXCE_WRITE = 0X00A3,           /* 异常通知 */
    QIOT_DPCMD_EXCE_EVENT = 0X00A4,           /* 异常通知 */
    QIOT_DPCMD_DEV_MANAGER_WRITE = 0X00B3,    /* 设备管理命令 */
    QIOT_DPCMD_DEV_CONFIG_WRITE = 0X00B4,     /* 设备配置命令 */
    QIOT_DPCMD_DEV_BINDCODE_WRITE = 0X00B5,   /* 设备上报绑定信息，仅在有局域网通信时有效 */
    QIOT_DPCMD_OTA_NOTIFY = 0X0111,           /* OTA升级任务通知 */
    QIOT_DPCMD_OTA_COMFIRM = 0X0112,          /* OTA确认-OTA升级任务通知确认是否要升级 */
    QIOT_DPCMD_OTA_FW_INFO = 0X0113,          /* 固件信息下发 */
    QIOT_DPCMD_OTA_EVENT = 0X0114,            /* OTA过程状态上报 */
    QIOT_DPCMD_OTA_REQUEST = 0X115,           /* OTA请求 */
    QIOT_DPCMD_LOC_CFG_WRITE = 0X0121,        /* 设置定位信息上报内容,不再支持 */
    QIOT_DPCMD_LOC_REPORT = 0X0122,           /* 定位信息上报 */
    QIOT_DPCMD_LOC_REQ = 0X0123,              /* 获取实时定位信息 */
    QIOT_DPCMD_LOC_RSP = 0X0124,              /* 获取实时定位信息-回复 */
    QIOT_DPCMD_LAN_SEND_ACK = 0X7000,         /* 应答局域网通信的请求包，相当于mqtt puback */
    QIOT_DPCMD_LAN_DISCOVER_REQ = 0X7001,     /* 发现局域网下设备 */
    QIOT_DPCMD_LAN_DISCOVER_RSP = 0X7002,     /* 应答发现局域网下设备 */
    QIOT_DPCMD_LAN_AUTH_COND_REQ = 0X7003,    /* 局域网认证前置条件请求 */
    QIOT_DPCMD_LAN_AUTH_COND_RSP = 0X7004,    /* 局域网认证前置条件应答 */
    QIOT_DPCMD_LAN_AUTH_SIGN_REQ = 0X7005,    /* 局域网认证签名请求 */
    QIOT_DPCMD_LAN_AUTH_SIGN_RSP = 0X7006,    /* 局域网认证签名应答 */
    QIOT_DPCMD_SUB_AUTH = 0x00C0,             /* 子设备认证指令 */
    QIOT_DPCMD_SUB_AUTH_RSP = 0x00C1,         /* 子设备认证回复 */
    QIOT_DPCMD_SUB_LOGIN = 0x00C2,            /* 子设备登陆指令 */
    QIOT_DPCMD_SUB_LOGIN_RSP = 0x00C3,        /* 子设备登陆返回 */
    QIOT_DPCMD_SUB_LOGOUT = 0x00C4,           /* 子设备登出 */
    QIOT_DPCMD_SUB_LOGOUT_RSP = 0x00C5,       /* 子设备登出回复 */
    QIOT_DPCMD_SUB_UNAUTH_EVENT = 0x00C6,     /* 子设备注销 */
    QIOT_DPCMD_SUB_UNAUTH_EVENT_RSP = 0x00C7, /* 子设备注销返回 */
    QIOT_DPCMD_SUB_OFFLINE_EVENT = 0x00C8,    /* 子设备下线通知 */
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
