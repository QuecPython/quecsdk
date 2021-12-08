/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : MQTT通信管理
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_mqtt.h"
#if (SDK_ENABLE_MQTT == 1)
#include "Quos_kernel.h"
#include "Qhal_driver.h"
#ifndef QUOS_MQTT_RESEND_TIME
#define QUOS_MQTT_RESEND_TIME 3
#endif
#ifndef QUOS_MQTT_SEND_TIMEOUT
#define QUOS_MQTT_SEND_TIMEOUT 2 * SWT_ONE_SECOND
#endif
#ifndef QUOS_MQTT_CONNECT_TIMEOUT
#define QUOS_MQTT_CONNECT_TIMEOUT 5 * SWT_ONE_SECOND
#endif
#ifndef QUOS_MQTT_PING_TIMEOUT
#define QUOS_MQTT_PING_TIMEOUT 5 * SWT_ONE_SECOND
#endif
typedef struct
{
    void *pingTimer;
    quint32_t keepAlive;
    MqttEventCb_f eventCB;
    MqttpublishRecv_f pubRecv;
    quint16_t pkgId;
    quint16_t qos2RecvId;
} mqttSock_t;

static void quos_mqttPing(void *swTimer);
/**************************************************************************
** 功能	@brief : MQTT协议通信数据包解析
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_mqttUnform(const quint8_t *buf, quint32_t bufLen, quint32_t *offset, Quos_socketTempData_t *unformTemp)
{
    if (unformTemp->bufLen == 0)
    {
        unformTemp->buf = (quint8_t *)HAL_MALLOC(5);
        if (NULL == unformTemp->buf)
        {
            Quos_logPrintf(LSDK_MQTT, LL_ERR, "mcf");
            *offset = bufLen;
            return FALSE;
        }
        unformTemp->offset = 0;
        unformTemp->bufLen = 5;
    }

    int pkgLen = 0;
    quint32_t i;
    for (i = 1; i < unformTemp->offset; i++)
    {
        if ((unformTemp->buf[i] & 0x80) == 0x00)
        {
            MQTTPacket_decodeBuf(unformTemp->buf + 1, &pkgLen);
            pkgLen = MQTTPacket_len(pkgLen);
        }
    }
    for (*offset = 0; *offset < bufLen; (*offset)++)
    {
        if (unformTemp->offset == 0)
        {
            quint8_t type = buf[*offset] >> 4;
            quint8_t flag = buf[*offset] & 0x0F;

            if ((0 == flag && (CONNECT == type || CONNACK == type || PUBACK == type || PUBREC == type || PUBCOMP == type || SUBACK == type || UNSUBACK == type || PINGREQ == type || PINGRESP == type || DISCONNECT == type)) ||
                (1 == flag && (PUBREL == type || SUBSCRIBE == type || UNSUBSCRIBE == type)) ||
                PUBLISH == type)
            {
                unformTemp->buf[unformTemp->offset++] = buf[*offset];
                Quos_logPrintf(LSDK_MQTT, LL_DBG, "mqtt head type:%X flag:%X", type, flag);
            }
        }
        else if (0 == pkgLen)
        {
            unformTemp->buf[unformTemp->offset++] = buf[*offset];
            if ((buf[*offset] & 0x80) == 0x00)
            {
                MQTTPacket_decodeBuf(unformTemp->buf + 1, &pkgLen);
                pkgLen = MQTTPacket_len(pkgLen);
                if (pkgLen > (int)unformTemp->bufLen)
                {
                    quint8_t *newBuf = (quint8_t *)HAL_MALLOC(pkgLen);
                    if (newBuf)
                    {
                        HAL_MEMCPY(newBuf, unformTemp->buf, unformTemp->offset);
                        HAL_FREE(unformTemp->buf);
                        unformTemp->buf = newBuf;
                        unformTemp->bufLen = pkgLen;
                    }
                    else
                    {
                        unformTemp->offset = 0;
                        *offset = bufLen;
                        return FALSE;
                    }
                }
                else if (pkgLen == 2)
                {
                    Quos_logPrintf(LSDK_MQTT, LL_DBG, "mqtt ok head[0x%02X] len[%u]", unformTemp->buf[0], unformTemp->offset);
                    (*offset)++;
                    return TRUE;
                }
            }
            else if (unformTemp->offset >= 5)
            {
                unformTemp->offset = 0;
                pkgLen = 0;
            }
        }
        else
        {
            unformTemp->buf[unformTemp->offset++] = buf[*offset];
            if (unformTemp->offset >= (quint32_t)pkgLen)
            {
                Quos_logPrintf(LSDK_MQTT, LL_DBG, "mqtt ok head[0x%02X] len[%u]", unformTemp->buf[0], unformTemp->offset);
                (*offset)++;
                return TRUE;
            }
        }
    }

    return FALSE;
}
/**************************************************************************
** 功能	@brief : m2m publish send ack
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static void quos_mqttPublishRecvAck(void *chlFd, const void *sendData, const void *recvData)
{
    UNUSED(chlFd);
    UNUSED(sendData);
    if (recvData)
    {
        Quos_logPrintf(LSDK_MQTT, LL_DBG, "publish success");
    }
    else
    {
        Quos_logPrintf(LSDK_MQTT, LL_DBG, "publish fail");
    }
}
/**************************************************************************
** 功能	@brief : m2m publish send
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qint32_t FUNCTION_ATTR_ROM Quos_mqttPublish(const void *chlFd, char *topicString, const void *param, qint32_t qos, void *buf, quint16_t bufLen, socketRecvNodeCb_f recvCB, qbool isAck)
{
    if (NULL == chlFd || NULL == topicString || NULL == buf || 0 == bufLen)
    {
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "chlFd[%p] topicString:%s buf:%p bufLen:%u", chlFd, topicString, buf, bufLen);
        return -1;
    }
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    if (NULL == chlNode->param)
    {
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "mqttSock invalid");
        return -1;
    }
    mqttSock_t *mqttSock = (mqttSock_t *)chlNode->param;
    MQTTString topic;
    HAL_MEMSET(&topic, 0, sizeof(topic));
    topic.cstring = topicString;
    quint16_t len = MQTTPacket_len(MQTTSerialize_publishLength(qos, topic, bufLen));
    quint8_t *pubBuf = HAL_MALLOC(len);
    if (NULL == pubBuf)
    {
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "mcf pubBuf");
        return -1;
    }
    len = MQTTSerialize_publish(pubBuf, len, 0, (int)qos, 0, 0 == qos ? 0 : ((++mqttSock->pkgId) == 0 ? (++mqttSock->pkgId) : mqttSock->pkgId), topic, buf, bufLen);
    if (isAck)
    {
        return Quos_socketTxDisorder(chlFd, NULL, pubBuf, len) ? mqttSock->pkgId : -1;
    }
    else if (0 == qos)
    {
        return Quos_socketTx(chlFd, NULL, 0, 0, NULL, NULL, 0, pubBuf, len, param) ? mqttSock->pkgId : -1;
    }
    else
    {
        return Quos_socketTx(chlFd, NULL, 0, 0, NULL, recvCB ? recvCB : quos_mqttPublishRecvAck, mqttSock->pkgId, pubBuf, len, param) ? mqttSock->pkgId : -1;
    }
}

/**************************************************************************
** 功能	@brief : 解析publish包
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_mqttPublishReslover(quint8_t *srcData, quint32_t srcLen, MQTTString *topicName, quint8_t **payload, int *payloadlen)
{
    int qos;
    quint8_t dup = 0;
    quint16_t packetid = 0;
    quint8_t retained;
    if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, topicName, payload, payloadlen, srcData, srcLen))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/**************************************************************************
** 功能	@brief : mqtt接收数据处理
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_mqttRecv(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData)
{
    UNUSED(peer);
    UNUSED(peerSize);
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    mqttSock_t *mqttSock = (mqttSock_t *)chlNode->param;
    if (NULL == recvData)
    {
        Quos_swTimerDelete(mqttSock->pingTimer);
        mqttSock->eventCB(chlFd, QUOS_MQTT_ERR_NET);
        return FALSE;
    }
    Quos_logHexDump(LSDK_MQTT, LL_DUMP, "m2m recv", recvData->Buf, recvData->bufLen);

    switch (recvData->Buf[0] >> 4)
    {
    case PUBLISH:
    {
        int qos;
        quint8_t dup = 0;
        quint16_t packetid = 0;
        quint8_t retained;
        MQTTString topicName;
        quint8_t *payload;
        int payloadlen;
        if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen, recvData->Buf, recvData->bufLen))
        {
            Quos_logPrintf(LSDK_MQTT, LL_DBG, "dup[%u] qos[%d] retained[%u] packetid[%u] topicName[%.*s] payload[%p] payloadlen[%d]", dup, qos, retained, packetid, topicName.lenstring.len, topicName.lenstring.data, payload, payloadlen);
            if (0 == qos)
            {
                mqttSock->pubRecv(&topicName, payload, (quint32_t)payloadlen);
            }
            else if (1 == qos)
            {
                mqttSock->pubRecv(&topicName, payload, payloadlen);
                quint16_t len = 4;
                quint8_t *buf = HAL_MALLOC(len);
                if (buf)
                {
                    MQTTSerialize_ack(buf, len, PUBACK, 0, packetid);
                    Quos_socketTxDisorder(chlFd, NULL, buf, len);
                }
            }
            else if (2 == qos)
            {
                if (mqttSock->qos2RecvId != packetid)
                {
                    mqttSock->qos2RecvId = packetid;
                    mqttSock->pubRecv(&topicName, payload, payloadlen);
                }
                quint16_t len = 4;
                quint8_t *buf = HAL_MALLOC(len);
                if (buf)
                {
                    MQTTSerialize_ack(buf, len, PUBREC, dup, packetid);
                    Quos_socketTxDisorder(chlFd, NULL, buf, len);
                }
            }
        }
        break;
    }
    case CONNACK:
    {
        Quos_socketTxAck(chlFd, NULL, 0, recvData);
        break;
    }
    case PINGRESP:
        Quos_swTimerTimeoutSet(mqttSock->pingTimer, mqttSock->keepAlive - (QUOS_MQTT_RESEND_TIME + 1) * QUOS_MQTT_PING_TIMEOUT);
        Quos_swTimerRepeatSet(mqttSock->pingTimer, QUOS_MQTT_RESEND_TIME);
        Quos_logPrintf(LSDK_MQTT, LL_INFO, "PINGRESP");
        break;
    case SUBACK:
    case PUBCOMP:
    case PUBACK:
    case UNSUBACK:
    {
        quint8_t type = 0, dup = 0;
        quint16_t packetid = 0;
        MQTTDeserialize_ack(&type, &dup, &packetid, recvData->Buf, recvData->bufLen);
        Quos_socketTxAck(chlFd, NULL, packetid, recvData);
        break;
    }
    case PUBREC:
    {
        quint16_t len = 4;
        quint8_t *buf = HAL_MALLOC(len);
        if (buf)
        {
            quint8_t type = 0, dup = 0;
            quint16_t packetid = 0;
            MQTTDeserialize_ack(&type, &dup, &packetid, recvData->Buf, recvData->bufLen);
            MQTTSerialize_ack(buf, len, PUBREL, dup, packetid);
            Quos_socketTxDisorder(chlFd, NULL, buf, len);
        }
        break;
    }
    case PUBREL:
    {
        quint16_t len = 4;
        quint8_t *buf = HAL_MALLOC(len);
        if (buf)
        {
            quint8_t type = 0, dup = 0;
            quint16_t packetid = 0;
            MQTTDeserialize_ack(&type, &dup, &packetid, recvData->Buf, recvData->bufLen);
            MQTTSerialize_pubcomp(buf, len, packetid);
            Quos_socketTxDisorder(chlFd, NULL, buf, len);
        }
        break;
    }
    default:
        break;
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : mqtt断开
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_mqttParamFree(void *param)
{
    mqttSock_t *mqttSock = (mqttSock_t *)param;
    Quos_swTimerDelete(mqttSock->pingTimer);
    HAL_FREE(mqttSock);
}
/**************************************************************************
** 功能	@brief : Mqtt connect应答结果
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_mqttConnectAck(void *chlFd, const void *sendData, const void *recvDataIn)
{
    UNUSED(sendData);
    Quos_socketRecvDataNode_t *recvData = (Quos_socketRecvDataNode_t *)recvDataIn;
    quint8_t sessionPresent, connack_rc = 0;
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    mqttSock_t *mqttSock = (mqttSock_t *)chlNode->param;
    if (NULL == recvData)
    {
        mqttSock->eventCB(chlFd, QUOS_MQTT_ERR_NET);
        Quos_socketChannelDel((void *)chlNode);
    }
    else if (1 != MQTTDeserialize_connack(&sessionPresent, &connack_rc, (quint8_t *)recvData->Buf, recvData->bufLen))
    {
        mqttSock->eventCB(chlFd, QUOS_MQTT_ERR_CONNECT);
        Quos_socketChannelDel(chlFd);
    }
    else if (QUOS_MQTT_CONNECTION_ACCEPTED != connack_rc)
    {
        mqttSock->eventCB(chlFd, (qint32_t)connack_rc);
        Quos_socketChannelDel(chlFd);
    }
    else if (FALSE == Quos_swTimerStart(&mqttSock->pingTimer, "MQTT ping", (mqttSock->keepAlive - (QUOS_MQTT_RESEND_TIME + 1) * QUOS_MQTT_PING_TIMEOUT) / 2, QUOS_MQTT_RESEND_TIME, quos_mqttPing, chlFd))
    {
        mqttSock->eventCB(chlFd, QUOS_MQTT_ERR_INSIDE);
        Quos_socketChannelDel(chlFd);
    }
    else
    {
        mqttSock->eventCB(chlFd, QUOS_MQTT_OK_CONNECT);
    }
}
/**************************************************************************
** 功能	@brief : Mqtt connect&subscribe&ping应答结果
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_mqttSubcribeAck(void *chlFd, const void *sendData, const void *recvDataIn)
{
    Quos_socketRecvDataNode_t *recvData = (Quos_socketRecvDataNode_t *)recvDataIn;
    quint16_t packetid;
    int count, grantedQoSs[100];
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    mqttSock_t *mqttSock = (mqttSock_t *)chlNode->param;
    UNUSED(sendData);
    if (NULL == recvData ||
        1 != MQTTDeserialize_suback(&packetid, sizeof(grantedQoSs) / sizeof(grantedQoSs[0]), &count, grantedQoSs, recvData->Buf, recvData->bufLen))
    {
        Quos_swTimerDelete(mqttSock->pingTimer);
        mqttSock->eventCB(chlFd, QUOS_MQTT_ERR_SUBSCRI);
        Quos_socketChannelDel(NULL == recvData ? (void *)chlNode : chlFd);
    }
    else
    {
        mqttSock->eventCB(chlFd, QUOS_MQTT_OK_SUBSCRIBE);
    }
}
/**************************************************************************
** 功能	@brief : MQTT 心跳
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_mqttPing(void *swTimer)
{
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)Quos_swTimerParmGet(swTimer);
    if (0 == Quos_swTimerRepeatGet(swTimer) && Quos_socketCheckChlFd(chlNode) )
    {
        mqttSock_t *mqttSock = (mqttSock_t *)chlNode->param;
        mqttSock->eventCB(chlNode, QUOS_MQTT_ERR_PING);
        Quos_socketChannelDel((void *)chlNode);
        return;
    }
    quint16_t len = 2;
    quint8_t *buf = HAL_MALLOC(len);
    Quos_swTimerTimeoutSet(swTimer, QUOS_MQTT_PING_TIMEOUT);
    if (NULL == buf)
    {
        return;
    }
    Quos_logPrintf(LSDK_MQTT, LL_INFO, "ping");
    MQTTSerialize_pingreq(buf, len);
    Quos_socketTxDisorder(chlNode, NULL, buf, len);
}
/**************************************************************************
** 功能	@brief : Mqtt socket connet结果
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_mqttSockConnctCB(void *chlFd, qbool result)
{
    Quos_logPrintf(LSDK_MQTT, LL_DBG, "chlFd[%p] result:%s", chlFd, _BOOL2STR(result));
    if (result == FALSE && NULL != chlFd)
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        ((mqttSock_t *)(chlNode->param))->eventCB(chlFd, QUOS_MQTT_ERR_NET);
    }
    return TRUE;
}

/**************************************************************************
** 功能	@brief : Mqtt服务初始化
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_mqttInit(void **chlFdPoint,
                                      const char *url,
                                      const char *clientID,
                                      const char *username,
                                      const char *password,
                                      quint16_t keepAlive,
                                      quint8_t topicCount,
                                      char *topicString[],
                                      const int *requestedQoSs,
                                      MqttEventCb_f eventCB,
                                      MqttpublishRecv_f pubRecv)
{
    urlAnalyze_t urlA;
    if (NULL == url || NULL == clientID || NULL == password || (0 != topicCount && (NULL == topicString || NULL == requestedQoSs)) || NULL == eventCB || NULL == pubRecv)
    {
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "url[%p] clientID[%s] username[%s] password[%s] topicCount[%u] topic[%p] requestedQoSs[%p] eventCB[%p] pubRecv[%p]",
                       url, clientID, username, password, topicCount, topicString, requestedQoSs, eventCB, pubRecv);
        return FALSE;
    }
    if (FALSE == (Quos_urlAnalyze(url, &urlA)))
    {
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "url analyze fail");
        return FALSE;
    }
    urlA.port = urlA.port ? urlA.port : (urlA.isSecure ? 8883 : 1883);

    mqttSock_t *mqttSock = HAL_MALLOC(sizeof(mqttSock_t));
    if (NULL == mqttSock)
    {
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "mcf mqttSock");
        return FALSE;
    }
    HAL_MEMSET(mqttSock, 0, sizeof(mqttSock_t));

    Quos_socketChlInfoNode_t chlInfo;
    HAL_MEMSET(&chlInfo, 0, sizeof(Quos_socketChlInfoNode_t));
    if (urlA.isSecure)
    {
#if (SDK_ENABLE_TLS == 1)
        chlInfo.sockFd = Qhal_tcpSslClientInit(&chlInfo.type, urlA.hostname, urlA.port, &chlInfo.conn.timeout);
#else
        chlInfo.sockFd = SOCKET_FD_INVALID;
#endif
    }
    else
    {
        chlInfo.sockFd = Qhal_tcpClientInit(&chlInfo.type, urlA.hostname, urlA.port, &chlInfo.conn.timeout);
    }
    if (chlInfo.conn.timeout)
    {
        chlInfo.conn.notify = quos_mqttSockConnctCB;
    }
    if (SOCKET_FD_INVALID == chlInfo.sockFd)
    {
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "mqtt conn fail:%s[%u]", urlA.hostname, urlA.port);
        HAL_FREE(mqttSock);
        return FALSE;
    }

    chlInfo.io.send = Qhal_sockWrite;
    chlInfo.send.txCnt = QUOS_MQTT_RESEND_TIME;
    chlInfo.send.timeout = QUOS_MQTT_SEND_TIMEOUT;
    chlInfo.recvDataFunc = quos_mqttRecv;
    chlInfo.unformFunc = quos_mqttUnform;
    chlInfo.io.close = Qhal_sockClose;
    chlInfo.paramFree = quos_mqttParamFree;
    chlInfo.param = mqttSock;

    if (keepAlive <= (QUOS_MQTT_RESEND_TIME + 1) * QUOS_MQTT_PING_TIMEOUT / SWT_ONE_SECOND)
    {
        keepAlive = (QUOS_MQTT_RESEND_TIME + 1) * QUOS_MQTT_PING_TIMEOUT / SWT_ONE_SECOND + 1;
    }
    mqttSock->keepAlive = (quint32_t)keepAlive * SWT_ONE_SECOND;
    mqttSock->eventCB = eventCB;
    mqttSock->pubRecv = pubRecv;

    void *chlFd = Quos_socketChannelAdd(chlFdPoint, chlInfo);
    if (NULL == chlFd)
    {
        Qhal_sockClose(chlInfo.sockFd, chlInfo.type);
        HAL_FREE(mqttSock);
        Quos_logPrintf(LSDK_MQTT, LL_ERR, "add socket Channel fail");
        return FALSE;
    }

    MQTTPacket_connectData mqttConnData = MQTTPacket_connectData_initializer;
    mqttConnData.keepAliveInterval = keepAlive;
    mqttConnData.clientID.cstring = (char *)clientID;
    mqttConnData.username.cstring = (char *)username;
    mqttConnData.password.cstring = (char *)password;

    quint16_t len = MQTTPacket_len(MQTTSerialize_connectLength(&mqttConnData));
    quint8_t *pkg = HAL_MALLOC(len);
    if (NULL == pkg)
    {
        Quos_socketChannelDel(chlFd);
        return FALSE;
    }
    len = MQTTSerialize_connect(pkg, len, &mqttConnData);
    if (FALSE == Quos_socketTx(chlFd, NULL, 0, QUOS_MQTT_CONNECT_TIMEOUT, NULL, quos_mqttConnectAck, 0, pkg, len, NULL))
    {
        Quos_socketChannelDel(chlFd);
        return FALSE;
    }
    if (topicCount > 0)
    {
        MQTTString *topic = HAL_MALLOC(sizeof(MQTTString) * topicCount);
        if (NULL == topic)
        {
            Quos_socketChannelDel(chlFd);
            return FALSE;
        }
        HAL_MEMSET(topic, 0, sizeof(MQTTString) * topicCount);
        quint8_t i;
        for (i = 0; i < topicCount; i++)
        { 
            topic[i].cstring = topicString[i];
        }

        len = MQTTPacket_len(MQTTSerialize_subscribeLength(topicCount, topic));
        pkg = HAL_MALLOC(len);
        if (NULL == pkg)
        {
            HAL_FREE(topic);
            Quos_socketChannelDel(chlFd);
            return FALSE;
        }
        len = MQTTSerialize_subscribe(pkg, len, 0, ++mqttSock->pkgId, topicCount, topic, (int *)requestedQoSs);
        HAL_FREE(topic);
        if (FALSE == Quos_socketTx(chlFd, NULL, 0, QUOS_MQTT_CONNECT_TIMEOUT, NULL, quos_mqttSubcribeAck, mqttSock->pkgId, pkg, len, NULL))
        {
            Quos_logPrintf(LSDK_MQTT, LL_ERR, "mcf subscribe pkg");
            Quos_socketChannelDel(chlFd);
            return FALSE;
        }
    }
    Quos_logPrintf(LSDK_MQTT, LL_DBG, "mqtt init ok");
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 关闭MQTT服务
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_mqttDeinit(void *chlFd)
{
    Quos_socketChannelDel(chlFd);
}
#endif