/*
 * @Author: your name
 * @Date: 2021-06-21 10:50:46
 * @LastEditTime: 2021-07-07 20:14:11
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \quecthing_pythonSDK\components\quecsdk\driverLayer\qhal_patch.h
 */
#ifndef __QHAL_PATCH_H__
#define __QHAL_PATCH_H__
#include "Qhal_types.h"
#include "Ql_iotApi.h"

#if 0
enum iot_connack_return_codes
{
    QUOS_MQTT_CONNECTION_ACCEPTED = 0,
    QUOS_MQTT_UNNACCEPTABLE_PROTOCOL = 1,
    QUOS_MQTT_CLIENTID_REJECTED = 2,
    QUOS_MQTT_SERVER_UNAVAILABLE = 3,
    QUOS_MQTT_BAD_USERNAME_OR_PASSWORD = 4,
    QUOS_MQTT_NOT_AUTHORIZED = 5,
};
#endif

void qhal_mutex_create(Helios_Mutex_t *mutex);


/***************************MQTT function*******************************************/
enum
{
    QHAL_MQTT_ERR_INSIDE = -5,
    QHAL_MQTT_ERR_NET = -4,
    QHAL_MQTT_ERR_PING = -3,
    QHAL_MQTT_ERR_SUBSCRI = -2,
    QHAL_MQTT_ERR_CONNECT = -1,
    QHAL_MQTT_OK_CONNECT = 0,
    QHAL_MQTT_OK_SUBSCRIBE = 100,
};

#if 0
#define MQTT_ERR_STRING(X)                                                                                                           \
    (                                                                                                                                \
        (X == QUOS_MQTT_ERR_PING) ? "QUOS_MQTT_ERR_PING" : (X == QHAL_MQTT_ERR_SUBSCRI)            ? "MQTT_ERR_SUBSCRI"              \
                                                       : (X == QHAL_MQTT_ERR_CONNECT)              ? "MQTT_ERR_CONNECT"              \
                                                       : (X == QHAL_MQTT_ERR_NET)                  ? "MQTT_ERR_NET"                  \
                                                       : (X == QHAL_MQTT_OK_CONNECT)               ? "MQTT_OK_CONNECT"               \
                                                       : (X == QHAL_MQTT_OK_SUBSCRIBE)             ? "MQTT_OK_SUBSCRIBE"             \
                                                       : (X == QUOS_MQTT_UNNACCEPTABLE_PROTOCOL)   ? "MQTT_UNNACCEPTABLE_PROTOCOL"   \
                                                       : (X == QUOS_MQTT_CLIENTID_REJECTED)        ? "MQTT_CLIENTID_REJECTED"        \
                                                       : (X == QUOS_MQTT_SERVER_UNAVAILABLE)       ? "MQTT_SERVER_UNAVAILABLE"       \
                                                       : (X == QUOS_MQTT_BAD_USERNAME_OR_PASSWORD) ? "MQTT_BAD_USERNAME_OR_PASSWORD" \
                                                       : (X == QUOS_MQTT_NOT_AUTHORIZED)           ? "MQTT_NOT_AUTHORIZED"           \
                                                                                                   : "Unknown")
#endif
typedef void (*MqttEventCb_f)(void *chlFd, qint32_t event);
typedef void (*MqttpublishRecv_f)(MQTTString *topicName, quint8_t *payload, quint32_t payloadlen);

qbool Qhal_mqttInit(void **chlFdPoint,
                    const char *url,
                    const char *clientID,
                    const char *username,
                    const char *password,
                    quint16_t keepAlive,
                    quint8_t topicCount,
                    MQTTString *topic,
                    const int *requestedQoSs,
                    MqttEventCb_f eventCB,
                    MqttpublishRecv_f pubRecv);
void Qhal_mqttDeinit(void *chlFd);
qint32_t Qhal_mqttPublishSend(const void *chlFd, char *topicString, const void *param, qint32_t qos, void *buf, quint16_t bufLen, socketRecvNodeCb_f recvCB);
qbool Qhal_mqttPublishReslover(quint8_t *srcData, quint32_t srcLen, MQTTString *topicName, quint8_t **payload, int *payloadlen);

/***************************MQTT function*******************************************/


#endif

