#ifndef __QIOT_COMMON_H__
#define __QIOT_COMMON_H__
#include "Ql_iotApi.h"

#define QUEC_BUS LL_DBG
#define QUEC_LAN LL_DBG
#define QUEC_OTA LL_DBG
#define QUEC_SYS LL_DBG
#define QUEC_IOT LL_DBG
#define QUEC_CONN LL_DBG
#define QUEC_DP LL_DBG
#define QUEC_LOC LL_DBG
#define QUEC_SECURE LL_ERR
#define QUEC_AT LL_DBG

#define QIOT_DATA_PROTOCOL_VER          "1.3.0"         /* 数据协议版本 */
#define QIOT_COM_PROTOCOL_VER           "3.0.0"         /* 通信协议版本 */
#define QIOT_SDK_VERSION                "2.9.0"
#define QIOT_DMP_SERVERURL_MQTT_DEFAULT "iot-south.quectel.com:1883"
#define QIOT_DMP_SERVERURL_MQTTS_DEFAULT "mqtts://iot-south.quectel.com:8883"
#define QIOT_DMP_SERVERURL_HTTP_DEFAULT "iot-south.quectel.com:2883"
#define QIOT_DMP_SERVERURL_HTTPS_DEFAULT "https://iot-south.quectel.com:2884"
#define QIOT_DMP_IP_MQTT_DEFAULT "106.14.246.239"

#define QIOT_MCUVERSION_STRING_ENDSTR ";"
#define QIOT_MCUVERSION_STRING_SEP ":"
#define QIOT_PK_MAXSIZE (32)
#define QIOT_PS_MAXSIZE (32)
#define QIOT_DK_MAXSIZE (16)
#define QIOT_DS_MAXSIZE (32)
#define QIOT_M2M_CLIENTID_MAXSIZE (2 + QIOT_PK_MAXSIZE + QIOT_DK_MAXSIZE)

#define QIOT_OTA_FILEINFO_MAX_SIZE (5)
#define QIOT_ATEVENT_TYPE_STRING(X)                                       \
    (                                                                     \
        (X == QUOS_SYSTEM_EVENT_NETWORK)    ? "QUOS_SYSTEM_EVENT_NETWORK" \
        : (X == QIOT_ATEVENT_TYPE_AUTH)     ? "ATEVENT_TYPE_AUTH"         \
        : (X == QIOT_ATEVENT_TYPE_CONN)     ? "ATEVENT_TYPE_CONN"         \
        : (X == QIOT_ATEVENT_TYPE_SUBCRIBE) ? "ATEVENT_TYPE_SUBCRIBE"     \
        : (X == QIOT_ATEVENT_TYPE_SEND)     ? "ATEVENT_TYPE_SEND"         \
        : (X == QIOT_ATEVENT_TYPE_RECV)     ? "ATEVENT_TYPE_RECV"         \
        : (X == QIOT_ATEVENT_TYPE_LOGOUT)   ? "ATEVENT_TYPE_LOGOUT"       \
        : (X == QIOT_ATEVENT_TYPE_OTA)      ? "ATEVENT_TYPE_OTA"          \
        : (X == QIOT_ATEVENT_TYPE_SERVER)   ? "ATEVENT_TYPE_SERVER"       \
                                            : "Unknown")

typedef struct
{
    char serverUrl[QUOS_DNS_HOSTNANE_MAX_LENGHT];
    char productKey[QIOT_PK_MAXSIZE + 1];
    char productSecret[QIOT_PS_MAXSIZE + 1];
    char serverIp[QUOS_IP_ADDR_MAX_LEN];
    char secret[QIOT_DS_MAXSIZE + 1]; /* 2 == encryptType有效 */
} Qiot_productInfo_t;

typedef struct
{
        char key[16];
        char iv[16];
        qbool flag;
        qbool usable;
} Qiot_sessionInfo_t;
typedef struct
{
    void *m2mCtx;
    qbool netIsConn;          /* 网络是否可以通信 */
    QIot_state_e workState;   /* 工作状态,用于AT指令查询状态 */
    QIot_connMode_e connMode; /* IOT连接模式 */
    quint8_t connFailCnt;
    Qiot_productInfo_t *connectProduct;
    Qiot_productInfo_t productInfo;
    Qiot_productInfo_t productInfoCloud;
    Qiot_productInfo_t productInfoCache;
    struct
    {
        quint8_t contextID;
        char deviceKey[QIOT_DK_MAXSIZE + 1];
    } deviceInfo;
    Qiot_sessionInfo_t sessionInfo;
    quint32_t lifetime;
    char *softversion;
    char mcuVerList[512];
#ifndef QUEC_ENABLE_AT
    void (*eventCB)(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen);
#endif
#ifdef QUEC_ENABLE_GATEWAY
    void (*SubDevEventCB)(quint32_t event, qint32_t errcode, const char *subPk, const char * subDk, const void *value, quint32_t valLen);
#endif
} QIot_userData_t;

extern QIot_userData_t QIot_userdata;
qbool Ql_iotDSKVSave(void);
void Ql_iotUrcEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen);
#endif