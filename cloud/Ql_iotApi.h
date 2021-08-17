#ifndef __QIOT_API_H__
#define __QIOT_API_H__
#include "Quos_kernel.h"

enum
{
    QIOT_ATEVENT_TYPE_AUTH = 1,
    QIOT_ATEVENT_TYPE_CONN = 2,
    QIOT_ATEVENT_TYPE_SUBCRIBE = 3,
    QIOT_ATEVENT_TYPE_SEND = 4,
    QIOT_ATEVENT_TYPE_RECV = 5,
    QIOT_ATEVENT_TYPE_LOGOUT = 6,
    QIOT_ATEVENT_TYPE_OTA = 7,
    QIOT_ATEVENT_TYPE_SERVER = 8,
};
enum
{
    QIOT_AUTH_SUCC = 10200,                /* 设备认证成功 */
    QIOT_AUTH_ERR_REQDATA = 10420,         /* 请求数据错误（连接失败）*/
    QIOT_AUTH_ERR_DONE = 10422,            /* 设备已认证（连接失败）*/
    QIOT_AUTH_ERR_PKPS_INVALID = 10423,    /* 没有找到产品信息（连接失败）*/
    QIOT_AUTH_ERR_PAYLOAD_INVALID = 10424, /* PAYLOAD解析失败（连接失败）*/
    QIOT_AUTH_ERR_SIGN_INVALID = 10425,    /* 签名验证未通过（连接失败）*/
    QIOT_AUTH_ERR_VERSION_INVALID = 10426, /* 认证版本错误（连接失败）*/
    QIOT_AUTH_ERR_HASH_INVALID = 10427,    /* 散列信息不合法（连接失败）*/
    QIOT_AUTH_ERR_PK_CHANGE = 10430,       /* PK发生改变 */
    QIOT_AUTH_ERR_DK_ILLEGAL = 10431,      /* DK不合法 */
    QIOT_AUTH_ERR_PK_VER_NOCOR = 10432,    /* PK与认证版本不匹配 */
    QIOT_AUTH_ERR_DEVICE_INSIDE = 10450,   /* 设备内部错误（连接失败）*/
    QIOT_AUTH_ERR_SERVER_NOTFOUND = 10466, /* 引导服务器地址未找到（连接失败）*/
    QIOT_AUTH_ERR_FAIL = 10500,            /* 设备认证失败（系统发生未知异常）*/
    QIOT_AUTH_ERR_UNKNOWN = 10300,         /* 其他错误 */
};
enum
{
    QIOT_CONN_SUCC = 10200,                 /* 接入成功 */
    QIOT_CONN_ERR_DS_INVALID = 10430,       /* 设备密钥不正确（连接失败）*/
    QIOT_CONN_ERR_DEVICE_FORBID = 10431,    /* 设备被禁用（连接失败）*/
    QIOT_CONN_ERR_DEVICE_INSIDE = 10450,    /* 设备内部错误（连接失败）*/
    QIOT_CONN_ERR_VERSION_NOTFOUND = 10471, /* 实现方案版本不支持（连接失败）*/
    QIOT_CONN_ERR_PING = 10473,             /* 接入心跳异常 */
    QIOT_CONN_ERR_NET = 10474,              /* 网络异常 */
    QIOT_CONN_ERR_UNKNOW = 10500,           /* 接入失败（系统发生未知异常）*/
};
enum
{
    QIOT_SUBCRIBE_SUCC = 10200, /* 订阅成功 */
    QIOT_SUBCRIBE_ERR = 10300,  /* 订阅失败 */
};
enum
{
    QIOT_SEND_SUCC_TRANS = 10200,    /* 透传数据发送成功 */
    QIOT_SEND_ERR_TRANS = 10300,     /* 透传数据发送失败 */
    QIOT_SEND_SUCC_PHYMODEL = 10210, /* 物模型数据发送成功 */
    QIOT_SEND_ERR_PHYMODEL = 10310,  /* 物模型数据发送失败 */
    QIOT_SEND_SUCC_LOC = 10220,      /* 定位数据发送成功 */
    QIOT_SEND_ERR_FAIL_LOC = 10320,  /* 定位数据发送失败 */
};
enum
{
    QIOT_RECV_SUCC_TRANS = 10200,         /* 收到透传数据 */
    QIOT_RECV_SUCC_PHYMODEL_RECV = 10210, /* 收到物模型下发数据 */
    QIOT_RECV_SUCC_PHYMODEL_REQ = 10211,  /* 收到物模型请求数据 */
    QIOT_RECV_ERR_BUFFER = 10473,         /* 接收失败,收到数据但长度超过模组buffer限制，AT非缓存模式下有效*/
    QIOT_RECV_ERR_LIMIT = 10428,          /* 数据接收失败，设备被限制消息通信，缓存模式下有效 */
};
enum
{
    QIOT_LOGOUT_SUCC = 10200, /* 断开连接成功 */
};
enum
{
    QIOT_OTA_TASK_NOTIFY = 10700, /* 有升级任务 */
    QIOT_OTA_START = 10701,       /* 模组开始下载 */
    QIOT_OTA_DOWNLOADING = 10702, /* 包下载中 */
    QIOT_OTA_DOWNLOADED = 10703,  /* 包下载完成 */
    QIOT_OTA_UPDATING = 10704,    /* 包更新中 */
    QIOT_OTA_UPDATE_OK = 10705,   /* 包更新完成 */
    QIOT_OTA_UPDATE_FAIL = 10706, /* 包更新失败 */
};
enum
{
    QIOT_SERVER_ERRCODE_RATE_LIMIT = 10428,
    QIOT_SERVER_ERRCODE_QUANTITY_LIMIT = 10429,
};

/* ql_iotDp.h */
typedef enum
{
    QIOT_DPCMD_TYPE_SYS = 0, /* sys类型命令 */
    QIOT_DPCMD_TYPE_BUS,     /* 业务数据类型命令*/
    QIOT_DPCMD_TYPE_OTA,     /* OTA类型命令 */
} QIot_dpCmdType_e;
typedef enum
{
    QIOT_DPDATA_TYPE_BOOL = 0,
    QIOT_DPDATA_TYPE_INT,
    QIOT_DPDATA_TYPE_FLOAT,
    QIOT_DPDATA_TYPE_BYTE,
    QIOT_DPDATA_TYPE_STRUCT,
} QIot_dpDataType_e;

void Ql_iotTtlvFree(void **ttlvHead);

quint32_t Ql_iotTtlvCountGet(const void *ttlvHead);
void *Ql_iotTtlvNodeGet(const void *ttlvHead, quint16_t index, quint16_t *id, QIot_dpDataType_e *type);

qbool Ql_iotTtlvNodeGetBool(const void *ttlvNode, qbool *value);
qbool Ql_iotTtlvNodeGetInt(const void *ttlvNode, qint64_t *value);
qbool Ql_iotTtlvNodeGetFloat(const void *ttlvNode, double *value);
char *Ql_iotTtlvNodeGetString(const void *ttlvNode);
quint32_t Ql_iotTtlvNodeGetByte(const void *ttlvNode, quint8_t **value);
void *Ql_iotTtlvNodeGetStruct(const void *ttlvNode);

qbool Ql_iotTtlvIdGetBool(const void *ttlvHead, quint16_t id, qbool *value);
qbool Ql_iotTtlvIdGetInt(const void *ttlvHead, quint16_t id, qint64_t *value);
qbool Ql_iotTtlvIdGetFloat(const void *ttlvHead, quint16_t id, double *value);
char *Ql_iotTtlvIdGetString(const void *ttlvHead, quint16_t id);
quint32_t Ql_iotTtlvIdGetByte(const void *ttlvHead, quint16_t id, quint8_t **value);
void *Ql_iotTtlvIdGetStruct(const void *ttlvHead, quint16_t id);

/* id为0时即为添加数组节点 */
qbool Ql_iotTtlvIdAddBool(void **ttlvHead, quint16_t id, qbool value);
qbool Ql_iotTtlvIdAddInt(void **ttlvHead, quint16_t id, qint64_t num);
qbool Ql_iotTtlvIdAddFloat(void **ttlvHead, quint16_t id, double num);
qbool Ql_iotTtlvIdAddByte(void **ttlvHead, quint16_t id, quint8_t *data, quint32_t len);
qbool Ql_iotTtlvIdAddStruct(void **ttlvHead, quint16_t id, void *vStruct);
#define Ql_iotTtlvIdAddString(ttlvHead, id, data) Ql_iotTtlvIdAddByte(ttlvHead, id, (quint8_t *)data, HAL_STRLEN(data))

/* ql_iotCmdBus.h */
qbool Ql_iotCmdBusPassTransSend(quint16_t mode, quint8_t *payload, quint32_t len);
qbool Ql_iotCmdBusPhymodelReport(quint16_t mode, const void *ttlvHead);
qbool Ql_iotCmdBusPhymodelAck(quint16_t mode, quint16_t pkgId, const void *ttlvHead);
qbool Ql_iotCmdBusLocReport(void);

/* ql_iotCmdOTA.h */
qbool Ql_iotCmdOtaAction(quint8_t action);
quint32_t Ql_iotCmdOtaMcuFWDataRead(quint32_t startAddr, quint8_t data[], quint32_t maxLen);

/* ql_iotCmdSys.h */
/* 设备状态 */
enum
{
    QIOT_DPID_STATUS_BATTERY = 1,   /* 电量 */
    QIOT_DPID_STATUS_VOLTAGE = 2,   /* 电压 */
    QIOT_DPID_STATUS_SIGNAL = 3,    /* 信号强度 */
    QIOT_DPID_STATUS_FLASHFREE = 4, /* 剩余空间 */
    QIOT_DPID_STATUS_RSRP = 5,      /* 参考信号接收功率 */
    QIOT_DPID_STATUS_RSRQ = 6,      /* LTE参考信号接收质量 */
    QIOT_DPID_STATUS_SNR = 7,       /* 信号与干扰加噪声比 */
    QIOT_DPID_STATUS_MAX,
};
/* 模组信息 */
enum
{
    QIOT_DPID_INFO_MODEL_TYPE = 1,   /* 模组型号 */
    QIOT_DPID_INFO_MODEL_VER = 2,    /* 模组版本 */
    QIOT_DPID_INFO_MCU_VER = 3,      /* MCU版本 */
    QIOT_DPID_INFO_CELLID = 4,       /* 基站id */
    QIOT_DPID_INFO_ICCID = 5,        /* SIM卡号 */
    QIOT_DPID_INFO_MCC = 6,          /* 移动国家代码 */
    QIOT_DPID_INFO_MNC = 7,          /* 移动网络代码 */
    QIOT_DPID_INFO_LAC = 8,          /* 位置区代码 */
    QIOT_DPID_INFO_PHONE_NUM = 9,    /* phone号 */
    QIOT_DPID_INFO_SIM_NUM = 10,     /* SIM号 */
    QIOT_DPID_INFO_SDK_VER = 11,     /* IOT SDK版本号*/
    QIOT_DPID_INFO_LOC_SUPLIST = 12, /* 定位功能支持列表 */
    QIOT_DPID_INFO_MAX,
};

qbool Ql_iotCmdSysStatusReport(quint16_t ids[], quint32_t size);
qbool Ql_iotCmdSysDevInfoReport(quint16_t ids[], quint32_t size);

/* ql_iotConn.h */

/* ql_iotConfig.h */
typedef enum
{
    QIOT_CONNMODE_IDLE, /* 不连接IOT */
    QIOT_CONNMODE_REQ,  /* 手动发起连接IOT */
    QIOT_CONNMODE_AUTO, /* 联网后主动连接IOT */
} QIot_connMode_e;
typedef enum
{
    QIOT_PPROTOCOL_COAP = 0,
    QIOT_PPROTOCOL_MQTT,
} QIot_protocolType_t;
typedef enum
{
    QIOT_STATE_UNINITIALIZE = 0,
    QIOT_STATE_INITIALIZED,
    QIOT_STATE_AUTHENTICATING,
    QIOT_STATE_AUTHENTICATED,
    QIOT_STATE_AUTHENTICATE_FAILED,
    QIOT_STATE_CONNECTING,
    QIOT_STATE_CONNECTED,
    QIOT_STATE_CONNECT_FAIL,
    QIOT_STATE_SUBSCRIBED,
    QIOT_STATE_SUBSCRIBE_FAIL,
    QIOT_STATE_DISCONNECTING,
    QIOT_STATE_DISCONNECTED,
    QIOT_STATE_DISCONNECT_FAIL,
} QIot_state_e;
void Ql_iotInit(void);
qbool Ql_iotConfigSetConnmode(QIot_connMode_e mode);
QIot_connMode_e Ql_iotConfigGetConnmode(void);
qbool Ql_iotConfigSetPdpContextId(quint8_t contextID);
quint8_t Ql_iotConfigGetPdpContextId(void);
qbool Ql_iotConfigSetServer(QIot_protocolType_t type, const char *server_url);
void Ql_iotConfigGetServer(QIot_protocolType_t *type, char **server_url);
qbool Ql_iotConfigSetProductinfo(const char *pk, const char *ps);
void Ql_iotConfigGetProductinfo(char **pk, char **ps, char **ver);
qbool Ql_iotConfigSetLifetime(quint32_t lifetime);
quint32_t Ql_iotConfigGetLifetime(void);
qbool Ql_iotConfigAppendAppVersion(const char *appVer); /* 对APP层只有openC方案可用 */
char *Ql_iotConfigGetSoftVersion(void);
qbool Ql_iotConfigSetMcuVersion(const char *compno, const char *version);
quint32_t Ql_iotConfigGetMcuVersion(const char *compno, char **version);
void Ql_iotConfigSetEventCB(void (*eventCb)(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen));
QIot_state_e Ql_iotGetWorkState(void);

/* ql_iotLocator.h */
qbool Ql_iotLocatorConfigSet(const void *ttlvHead);
void *Ql_iotLocatorConfigGet(void);
void *Ql_iotLocatorDataGet(const void *titleTtlv);
char *Ql_iotLocatorTtlv2String(const void *ttlv);
qbool Ql_iotLocatoTitleClashCheck(const void *ttlvHead);
qbool Ql_iotLocatoTitleSupportCheck(const void *ttlvHead);

quint32_t Quos_stringSplit(char *src, char **words, quint32_t maxSize, char *delim, qbool keepEmptyParts);

#endif