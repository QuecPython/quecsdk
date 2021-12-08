#ifndef __QIOT_API_H__
#define __QIOT_API_H__
#if 1
#include "Quos_kernel.h"
#else
#include "../driverLayer/Qhal_types.h"

typedef struct cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==QUOS_cJSON_String  and type == QUOS_cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==QUOS_cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;

#endif
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
    QIOT_ATEVENT_TYPE_UNAUTH = 9,
};
enum
{
    QIOT_AUTH_SUCC = 10200,                /* �豸��֤�ɹ� */
    QIOT_AUTH_ERR_DMP_INSIDE = 10404,      /* DMP�ڲ��ӿڵ���ʧ�� */
    QIOT_AUTH_ERR_REQDATA = 10420,         /* �������ݴ�������ʧ�ܣ�*/
    QIOT_AUTH_ERR_DONE = 10422,            /* �豸����֤������ʧ�ܣ�*/
    QIOT_AUTH_ERR_PKPS_INVALID = 10423,    /* û���ҵ���Ʒ��Ϣ������ʧ�ܣ�*/
    QIOT_AUTH_ERR_PAYLOAD_INVALID = 10424, /* PAYLOAD����ʧ�ܣ�����ʧ�ܣ�*/
    QIOT_AUTH_ERR_SIGN_INVALID = 10425,    /* ǩ����֤δͨ��������ʧ�ܣ�*/
    QIOT_AUTH_ERR_VERSION_INVALID = 10426, /* ��֤�汾��������ʧ�ܣ�*/
    QIOT_AUTH_ERR_HASH_INVALID = 10427,    /* ɢ����Ϣ���Ϸ�������ʧ�ܣ�*/
    QIOT_AUTH_ERR_PK_CHANGE = 10430,       /* PK�����ı� */
    QIOT_AUTH_ERR_DK_ILLEGAL = 10431,      /* DK���Ϸ� */
    QIOT_AUTH_ERR_PK_VER_NOCOR = 10432,    /* PK����֤�汾��ƥ�� */
    QIOT_AUTH_ERR_PSWORD = 10436,          /* ��¼�û������� */
    QIOT_AUTH_ERR_DEVICE_INFO = 10438,     /* û���ҵ��豸��Ϣ */
    QIOT_AUTH_ERR_DEVICE_INSIDE = 10450,   /* �豸�ڲ���������ʧ�ܣ�*/
    QIOT_AUTH_ERR_SERVER_NOTFOUND = 10466, /* ������������ַδ�ҵ�������ʧ�ܣ�*/
    QIOT_AUTH_ERR_FAIL = 10500,            /* �豸��֤ʧ�ܣ�ϵͳ����δ֪�쳣��*/
    QIOT_AUTH_ERR_UNKNOWN = 10300,         /* �������� */
};
enum
{
    QIOT_CONN_SUCC = 10200,                 /* ����ɹ� */
    QIOT_CONN_ERR_DMP_INSIDE = 10404,       /* DMP�ڲ��ӿڵ���ʧ�� */
    QIOT_CONN_ERR_DS_INVALID = 10430,       /* �豸��Կ����ȷ������ʧ�ܣ�*/
    QIOT_CONN_ERR_DEVICE_FORBID = 10431,    /* �豸�����ã�����ʧ�ܣ�*/
    QIOT_CONN_ERR_PSWORD = 10436,           /* ��¼�û������� */
    QIOT_CONN_ERR_DS = 10437,               /* �豸DS���� */
    QIOT_CONN_ERR_DEVICE_INFO = 10438,      /* û���ҵ��豸��Ϣ */
    QIOT_CONN_ERR_DEVICE_INSIDE = 10450,    /* �豸�ڲ���������ʧ�ܣ�*/
    QIOT_CONN_ERR_VERSION_NOTFOUND = 10471, /* ʵ�ַ����汾��֧�֣�����ʧ�ܣ�*/
    QIOT_CONN_ERR_PING = 10473,             /* ���������쳣 */
    QIOT_CONN_ERR_NET = 10474,              /* �����쳣 */
    QIOT_CONN_ERR_SERVER_CHANGE = 10475,    /* �����������ı� */
    QIOT_CONN_ERR_AP = 10476,               /* ����AP�쳣 */
    QIOT_CONN_ERR_UNKNOW = 10500,           /* ����ʧ�ܣ�ϵͳ����δ֪�쳣��*/
};
enum
{
    QIOT_SUBCRIBE_SUCC = 10200, /* ���ĳɹ� */
    QIOT_SUBCRIBE_ERR = 10300,  /* ����ʧ�� */
};
enum
{
    QIOT_SEND_SUCC_TRANS = 10200,    /* ͸�����ݷ��ͳɹ� */
    QIOT_SEND_ERR_TRANS = 10300,     /* ͸�����ݷ���ʧ�� */
    QIOT_SEND_SUCC_PHYMODEL = 10210, /* ��ģ�����ݷ��ͳɹ� */
    QIOT_SEND_ERR_PHYMODEL = 10310,  /* ��ģ�����ݷ���ʧ�� */
    QIOT_SEND_SUCC_LOC = 10220,      /* ��λ���ݷ��ͳɹ� */
    QIOT_SEND_ERR_FAIL_LOC = 10320,  /* ��λ���ݷ���ʧ�� */
    QIOT_SEND_SUCC_STATE = 10230,    /* ״̬���ݷ��ͳɹ� */
    QIOT_SEND_ERR_STATE = 10330,     /* ״̬���ݷ���ʧ�� */
    QIOT_SEND_SUCC_INFO = 10240,     /* �豸��Ϣ���ͳɹ� */
    QIOT_SEND_ERR_INFO = 10340,      /* �豸��Ϣ����ʧ�� */
};
enum
{
    QIOT_RECV_SUCC_TRANS = 10200,         /* �յ�͸������ */
    QIOT_RECV_SUCC_PHYMODEL_RECV = 10210, /* �յ���ģ���·����� */
    QIOT_RECV_SUCC_PHYMODEL_REQ = 10211,  /* �յ���ģ���������� */
    QIOT_RECV_SUCC_SUB_STATE_REQ = 10220, /* �յ����豸״̬�������� */
    QIOT_RECV_SUCC_SUB_INFO_REQ = 10230,  /* �յ����豸��Ϣ�������� */
    QIOT_RECV_ERR_BUFFER = 10473,         /* ����ʧ��,�յ����ݵ����ȳ���ģ��buffer���ƣ�AT�ǻ���ģʽ����Ч*/
    QIOT_RECV_ERR_LIMIT = 10428,          /* ���ݽ���ʧ�ܣ��豸��������Ϣͨ�ţ�����ģʽ����Ч */
};
enum
{
    QIOT_LOGOUT_SUCC = 10200, /* �Ͽ����ӳɹ� */
};
enum
{
    QIOT_OTA_TASK_NOTIFY = 10700, /* ���������� */
    QIOT_OTA_START = 10701,       /* ģ�鿪ʼ���� */
    QIOT_OTA_DOWNLOADING = 10702, /* �������� */
    QIOT_OTA_DOWNLOADED = 10703,  /* ��������� */
    QIOT_OTA_UPDATING = 10704,    /* �������� */
    QIOT_OTA_UPDATE_OK = 10705,   /* ��������� */
    QIOT_OTA_UPDATE_FAIL = 10706, /* ������ʧ�� */
    QIOT_OTA_UPDATE_FLAG = 10707, /* �׸��豸��������㲥 */
};
enum
{
    QIOT_SERVER_ERRCODE_RATE_LIMIT = 10428,
    QIOT_SERVER_ERRCODE_QUANTITY_LIMIT = 10429,
};
enum
{
    QIOT_SUB_DEV_ERR_No_ASSOCIATION = 10440, /* ���豸�뵱ǰ����û�й�����ϵ */
    QIOT_SUB_DEV_ERR_ALREADY_CONN = 10441,   /* ���豸�ظ���¼ */
    QIOT_SUB_DEV_ERR_UNLOGIN = 10442,        /* ���豸δ��¼ */
};
/* ql_iotDp.h */
typedef enum
{
    QIOT_DPCMD_TYPE_SYS = 0, /* sys�������� */
    QIOT_DPCMD_TYPE_BUS,     /* ҵ��������������*/
    QIOT_DPCMD_TYPE_OTA,     /* OTA�������� */
    QIOT_DPCMD_TYPE_LAN,     /* LAN�������� */
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

/* idΪ0ʱ��Ϊ�������ڵ� */
qbool Ql_iotTtlvIdAddBool(void **ttlvHead, quint16_t id, qbool value);
qbool Ql_iotTtlvIdAddInt(void **ttlvHead, quint16_t id, qint64_t num);
qbool Ql_iotTtlvIdAddFloat(void **ttlvHead, quint16_t id, double num);
qbool Ql_iotTtlvIdAddByte(void **ttlvHead, quint16_t id, const quint8_t *data, quint32_t len);
qbool Ql_iotTtlvIdAddString(void **ttlvHead, quint16_t id, const char *data);
qbool Ql_iotTtlvIdAddStruct(void **ttlvHead, quint16_t id, void *vStruct);
//#define Ql_iotTtlvIdAddString(ttlvHead, id, data) Ql_iotTtlvIdAddByte(ttlvHead, id, (quint8_t *)data, HAL_STRLEN(data))

void *Ql_iotJson2Ttlv(const cJSON *json);
cJSON *Ql_iotTtlv2Json(const void *ttlvHead);
/* ql_iotCmdBus.h */
qbool Ql_iotCmdBusPassTransSend(quint16_t mode, quint8_t *payload, quint32_t len);
qbool Ql_iotCmdBusPhymodelReport(quint16_t mode, const void *ttlvHead);
qbool Ql_iotCmdBusPhymodelAck(quint16_t mode, quint16_t pkgId, const void *ttlvHead);

/* ql_iotCmdLoc.h */
qbool Ql_iotCmdBusLocReportInside(void *titleTtlv);
qbool Ql_iotCmdBusLocReportOutside(void *nmeaTtlv);
void *Ql_iotLocGetData(const void *titleTtlv);
void *Ql_iotLocGetSupList(void);

/* ql_iotCmdOTA.h */
qbool Ql_iotCmdOtaRequest(quint32_t mode);
qbool Ql_iotCmdOtaAction(quint8_t action);
quint32_t Ql_iotCmdOtaMcuFWDataRead(quint32_t startAddr, quint8_t data[], quint32_t maxLen);

/* ql_iotCmdSys.h */
/* �豸״̬ */
enum
{
    QIOT_DPID_STATUS_BATTERY = 1,   /* ���� */
    QIOT_DPID_STATUS_VOLTAGE = 2,   /* ��ѹ */
    QIOT_DPID_STATUS_SIGNAL = 3,    /* �ź�ǿ�� */
    QIOT_DPID_STATUS_FLASHFREE = 4, /* ʣ��ռ� */
    QIOT_DPID_STATUS_RSRP = 5,      /* �ο��źŽ��չ��� */
    QIOT_DPID_STATUS_RSRQ = 6,      /* LTE�ο��źŽ������� */
    QIOT_DPID_STATUS_SNR = 7,       /* �ź�����ż������� */
    QIOT_DPID_STATUS_MAX,
};
/* ģ����Ϣ */
enum
{
    QIOT_DPID_INFO_MODEL_TYPE = 1,   /* ģ���ͺ� */
    QIOT_DPID_INFO_MODEL_VER = 2,    /* ģ��汾 */
    QIOT_DPID_INFO_MCU_VER = 3,      /* MCU�汾 */
    QIOT_DPID_INFO_CELLID = 4,       /* ��վid */
    QIOT_DPID_INFO_ICCID = 5,        /* SIM���� */
    QIOT_DPID_INFO_MCC = 6,          /* �ƶ����Ҵ��� */
    QIOT_DPID_INFO_MNC = 7,          /* �ƶ�������� */
    QIOT_DPID_INFO_LAC = 8,          /* λ�������� */
    QIOT_DPID_INFO_PHONE_NUM = 9,    /* phone�� */
    QIOT_DPID_INFO_SIM_NUM = 10,     /* SIM�� */
    QIOT_DPID_INFO_SDK_VER = 11,     /* quecthingSDK�汾��*/
    QIOT_DPID_INFO_LOC_SUPLIST = 12, /* ��λ����֧���б� */
    QIOT_DPIO_INFO_DP_VER = 13,      /* ����Э��汾 */
    QIOT_DPIO_INFO_CP_VER = 14,      /* ͨ��Э��汾�� */
    QIOT_DPID_INFO_MAX,
};

qbool Ql_iotCmdSysStatusReport(quint16_t ids[], quint32_t size);
qbool Ql_iotCmdSysDevInfoReport(quint16_t ids[], quint32_t size);
void *Ql_iotSysGetDevStatus(quint16_t ids[], quint32_t size);
void *Ql_iotSysGetDevInfo(quint16_t ids[], quint32_t size);
qbool Ql_iotCmdBindcodeReport(quint8_t bindcode[], quint32_t len);
/* ql_iotConn.h */
typedef enum
{
    QIOT_DPAPP_M2M = (1 << 0),
    QIOT_DPAPP_SUBDEV = (1 << 1),
    QIOT_DPAPP_LANPHONE = (1 << 2),
} QIot_dpAppType_e;

/* ql_iotConfig.h */
typedef enum
{
    QIOT_CONNMODE_IDLE, /* ������IOT */
    QIOT_CONNMODE_REQ,  /* �ֶ���������IOT */
    QIOT_CONNMODE_AUTO, /* ��������������IOT */
} QIot_connMode_e;
typedef enum
{
    QIOT_PPROTOCOL_COAP = 0,
    QIOT_PPROTOCOL_MQTT,
} QIot_protocolType_t;
typedef enum
{
    QIOT_STATE_UNINITIALIZE = 0,
    QIOT_STATE_INITIALIZED = 1,
    QIOT_STATE_AUTHENTICATING = 2,
    QIOT_STATE_AUTHENTICATED = 3,
    QIOT_STATE_AUTHENTICATE_FAILED = 4,
    QIOT_STATE_CONNECTING = 5,
    QIOT_STATE_CONNECTED = 6,
    QIOT_STATE_CONNECT_FAIL = 7,
    QIOT_STATE_SUBSCRIBED = 8,
    QIOT_STATE_SUBSCRIBE_FAIL = 9,
    QIOT_STATE_DISCONNECTING = 10,
    QIOT_STATE_DISCONNECTED = 11,
    QIOT_STATE_DISCONNECT_FAIL = 12,
} QIot_state_e;
qbool Ql_iotInit(void);
qbool Ql_iotConfigSetConnmode(QIot_connMode_e mode);
QIot_connMode_e Ql_iotConfigGetConnmode(void);
qbool Ql_iotConfigSetProductinfo(const char *pk, const char *ps);
void Ql_iotConfigGetProductinfo(char **pk, char **ps, char **ver);
qbool Ql_iotConfigSetServer(QIot_protocolType_t type, const char *server_url);
void Ql_iotConfigGetServer(QIot_protocolType_t *type, char **server_url);
qbool Ql_iotConfigSetProductinfo(const char *pk, const char *ps);
void Ql_iotConfigGetProductinfo(char **pk, char **ps, char **ver);
qbool Ql_iotConfigSetLifetime(quint32_t lifetime);
quint32_t Ql_iotConfigGetLifetime(void);
qbool Ql_iotConfigSetPdpContextId(quint8_t contextID);
quint8_t Ql_iotConfigGetPdpContextId(void);
qbool Ql_iotConfigSetSessionFlag(qbool flag);
qbool Ql_iotConfigGetSessionFlag(void);
qbool Ql_iotConfigSetAppVersion(const char *appVer); /* ��APP��ֻ��openC�������� */
char *Ql_iotConfigGetSoftVersion(void);
qbool Ql_iotConfigSetMcuVersion(const char *compno, const char *version);
quint32_t Ql_iotConfigGetMcuVersion(const char *compno, char **version);
void Ql_iotConfigSetEventCB(void (*eventCb)(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen));
QIot_state_e Ql_iotGetWorkState(void);
qbool Ql_iotConfigSetDkDs(const char *dk, const char *ds);
qbool Ql_iotConfigGetDkDs(char **dk, char **ds);

/* ql_fotaConfig.h */
#ifdef QUEC_ENABLE_HTTP_OTA
void Ql_iotConfigSetHttpOtaEventCb(void (*eventCb)(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen));
qbool Ql_iotConfigSetHttpOtaProductInfo(const char *pk, const char *ps);
void Ql_iotConfigGetHttpOtaProductInfo(char **pk, char **ps);
qbool Ql_iotConfigSetHttpOtaTls(qbool tls);
qbool Ql_iotConfigGetHttpOtaTls(void);
qbool Ql_iotConfigSetHttpOtaServer(const char *server_url);
void Ql_iotConfigGetHttpOtaServer(char **server_url);
qbool Ql_iotConfigSetHttpOtaUp(quint8_t battery, quint8_t upmode, const char *url);
void Ql_iotConfigGetHttpOtaUp(quint8_t *battery, quint8_t *upmode, char **url);
#endif

/* ql_iotGwDev.h */
#ifdef QUEC_ENABLE_GATEWAY
void Ql_iotConfigSetSubDevEventCB(void (*eventCb)(quint32_t event, qint32_t errcode, const char *subPk, const char *subDk, const void *value, quint32_t valLen));
qbool Ql_iotSubDevConn(const char *subPk, const char *subPs, const char *subDk, const char *subDs, quint8_t sessionType, quint16_t keepalive);
qbool Ql_iotSubDevDisconn(const char *subPk, const char *subDk);
qbool Ql_iotSubDevPassTransSend(const char *subPk, const char *subDk, quint8_t *payload, quint16_t payloadlen);
qbool Ql_iotSubDevTslReport(const char *subPk, const char *subDk, const void *ttlvHead);
qbool Ql_iotSubDevTslAck(const char *subPk, const char *subDk, quint16_t pkgId, const void *ttlvHead);
qbool Ql_iotSubDevDeauth(const char *subPk, const char *subPs, const char *subDk, const char *subDs);
qbool Ql_iotSubDevHTB(const char *subPk, const char *subDk);
#endif
#endif