#ifndef __QUOS_MQTT_H__
#define __QUOS_MQTT_H__
#include "quos_config.h"
#if (SDK_ENABLE_MQTT == 1)
#include "quos_socket.h"
#include "MQTTPacket.h"

enum
{
    QUOS_MQTT_ERR_INSIDE = -5,
    QUOS_MQTT_ERR_NET = -4,
    QUOS_MQTT_ERR_PING = -3,
    QUOS_MQTT_ERR_SUBSCRI = -2,
    QUOS_MQTT_ERR_CONNECT = -1,
    QUOS_MQTT_OK_CONNECT = 0,
    QUOS_MQTT_OK_SUBSCRIBE = 100,
};

#define MQTT_ERR_STRING(X)                                                                                                           \
    (                                                                                                                                \
        (X == QUOS_MQTT_ERR_PING) ? "QUOS_MQTT_ERR_PING" : (X == QUOS_MQTT_ERR_SUBSCRI)            ? "MQTT_ERR_SUBSCRI"              \
                                                       : (X == QUOS_MQTT_ERR_CONNECT)              ? "MQTT_ERR_CONNECT"              \
                                                       : (X == QUOS_MQTT_ERR_NET)                  ? "MQTT_ERR_NET"                  \
                                                       : (X == QUOS_MQTT_OK_CONNECT)               ? "MQTT_OK_CONNECT"               \
                                                       : (X == QUOS_MQTT_OK_SUBSCRIBE)             ? "MQTT_OK_SUBSCRIBE"             \
                                                       : (X == QUOS_MQTT_UNNACCEPTABLE_PROTOCOL)   ? "MQTT_UNNACCEPTABLE_PROTOCOL"   \
                                                       : (X == QUOS_MQTT_CLIENTID_REJECTED)        ? "MQTT_CLIENTID_REJECTED"        \
                                                       : (X == QUOS_MQTT_SERVER_UNAVAILABLE)       ? "MQTT_SERVER_UNAVAILABLE"       \
                                                       : (X == QUOS_MQTT_BAD_USERNAME_OR_PASSWORD) ? "MQTT_BAD_USERNAME_OR_PASSWORD" \
                                                       : (X == QUOS_MQTT_NOT_AUTHORIZED)           ? "MQTT_NOT_AUTHORIZED"           \
                                                                                                   : "Unknown")

typedef void (*MqttEventCb_f)(void *chlFd, qint32_t event);
typedef void (*MqttpublishRecv_f)(MQTTString *topicName, quint8_t *payload, quint32_t payloadlen);

qbool Quos_mqttInit(void **chlFdPoint,
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
void Quos_mqttDeinit(void *chlFd);
qint32_t Quos_mqttPublishSend(const void *chlFd, char *topicString, const void *param, qint32_t qos, void *buf, quint16_t bufLen, socketRecvNodeCb_f recvCB);
qbool Quos_mqttPublishReslover(quint8_t *srcData, quint32_t srcLen, MQTTString *topicName, quint8_t **payload, int *payloadlen);
#endif
#endif