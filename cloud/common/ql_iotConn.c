/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 2021-01-08
** 功能   @brief   : DMP接入
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "ql_iotConn.h"
#include "ql_iotConfig.h"
#include "ql_iotSecure.h"
#include "ql_iotDp.h"
#include "Qhal_driver.h"
#include "quos_net.h"
#ifdef QUEC_ENABLE_GATEWAY
#include "ql_iotDMSub.h"
#include "ql_iotGwDev.h"
#endif
#include "ql_iotTtlv.h"

enum
{
    QL_IOT_ERRCODE_LEVEL_FIRST = 0,
    QL_IOT_ERRCODE_LEVEL_SECOND,
    QL_IOT_ERRCODE_LEVEL_THIRD,
    QL_IOT_ERRCODE_LEVEL_FOURTH,
    QL_IOT_ERRCODE_LEVEL_FIFTH,
    QL_IOT_ERRCODE_LEVEL_MAX,
};
const quint32_t ql_errTypeTimeout[] = {SWT_ONE_SECOND, SWT_ONE_SECOND * 30, SWT_ONE_MINUTE * 5, SWT_ONE_MINUTE * 10, SWT_SUSPEND};
typedef struct
{
    quint32_t errcode : 24;
    quint32_t errType : 8;
} ql_errcodeType;

const ql_errcodeType ql_errcodeAuth[] =
    {
        {
            QIOT_AUTH_ERR_UNKNOWN,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            QIOT_AUTH_ERR_DONE,
            QL_IOT_ERRCODE_LEVEL_FOURTH,
        },
        {
            QIOT_AUTH_ERR_PKPS_INVALID,
            QL_IOT_ERRCODE_LEVEL_SECOND,
        },
        {
            QIOT_AUTH_ERR_PAYLOAD_INVALID,
            QL_IOT_ERRCODE_LEVEL_SECOND,
        },
        {
            QIOT_AUTH_ERR_SIGN_INVALID,
            QL_IOT_ERRCODE_LEVEL_SECOND,
        },
        {
            QIOT_AUTH_ERR_VERSION_INVALID,
            QL_IOT_ERRCODE_LEVEL_FOURTH,
        },
        {
            QIOT_AUTH_ERR_HASH_INVALID,
            QL_IOT_ERRCODE_LEVEL_SECOND,
        },
        {
            QIOT_AUTH_ERR_PK_CHANGE,
            QL_IOT_ERRCODE_LEVEL_FOURTH,
        },
        {
            QIOT_AUTH_ERR_DEVICE_INSIDE,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            QIOT_AUTH_ERR_SERVER_NOTFOUND,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            QIOT_AUTH_ERR_FAIL,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            0xFFFFFF,
            0,
        },
};

const ql_errcodeType ql_errcodeConn[] =
    {
        {
            QIOT_CONN_ERR_DS_INVALID,
            QL_IOT_ERRCODE_LEVEL_FIFTH,
        },
        {
            QIOT_CONN_ERR_DEVICE_FORBID,
            QL_IOT_ERRCODE_LEVEL_FOURTH,
        },
        {
            QIOT_CONN_ERR_DEVICE_INSIDE,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            QIOT_CONN_ERR_VERSION_NOTFOUND,
            QL_IOT_ERRCODE_LEVEL_FOURTH,
        },
        {
            QIOT_CONN_ERR_PING,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            QIOT_CONN_ERR_NET,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            QIOT_CONN_ERR_SERVER_CHANGE,
            QL_IOT_ERRCODE_LEVEL_FIRST,
        },
        {
            0xFFFFFF,
            0,
        },
};


typedef struct
{
    char *topicType;
    quint16_t *cmd;
} QIot_connCmdOutType_t;
quint16_t QIot_cmdOutSys[] = {QIOT_DPCMD_STATUS_RSP, QIOT_DPCMD_STATUS_EVENT, QIOT_DPCMD_INFO_RSP, QIOT_DPCMD_INFO_EVENT, QIOT_DPCMD_EXCE_EVENT, 
                                QIOT_DPCMD_SUB_AUTH, QIOT_DPCMD_SUB_AUTH_RSP, QIOT_DPCMD_SUB_LOGIN, QIOT_DPCMD_SUB_LOGIN_RSP, 
                                QIOT_DPCMD_SUB_LOGOUT, QIOT_DPCMD_SUB_LOGOUT_RSP, QIOT_DPCMD_SUB_UNAUTH_EVENT, QIOT_DPCMD_SUB_UNAUTH_EVENT_RSP, 
                                QIOT_DPCMD_SUB_OFFLINE_EVENT, 0};
quint16_t QIot_cmdOutBus[] = {QIOT_DPCMD_TSL_RSP, QIOT_DPCMD_TSL_EVENT, QIOT_DPCMD_PASS_EVENT, QIOT_DPCMD_LOC_REPORT, QIOT_DPCMD_LOC_RSP, 0};
quint16_t QIot_cmdOutOta[] = {QIOT_DPCMD_OTA_COMFIRM, QIOT_DPCMD_OTA_EVENT, QIOT_DPCMD_OTA_REQUEST, 0};
QIot_connCmdOutType_t QIot_connCmdOutType[] = {
    {.topicType = "sys", .cmd = QIot_cmdOutSys},
    {.topicType = "bus", .cmd = QIot_cmdOutBus},
    {.topicType = "ota", .cmd = QIot_cmdOutOta}};

#define QIOT_SUB_TOPIC_ROOT "q/1/d/" /* 根订阅topic */
#define QIOT_PUB_TOPIC_ROOT "q/2/d/" /* 根发布topic */

static void *QIot_RunTimer = NULL;
static void ql_iotConnStart(void);

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotConnServerInfoCheck(void)
{
    urlAnalyze_t urlA;
    if (FALSE == Quos_urlAnalyze(QIot_userdata.connectProduct->serverUrl, &urlA))
    {
        Quos_logPrintf(QUEC_CONN, LL_DBG, "get ip from url and ip manage");
        return;
    }
    /* 若ip发生改变，则需要触发保存,若当前连接服务器信息为缓存区，则不需要进行保存，需要等到连接DMP平台成功以后才可以保存 */
    char ip[QUOS_IP_ADDR_MAX_LEN];
    if (Quos_netHostnameValidIpGet(urlA.hostname, ip) && 0 != HAL_STRCMP(ip, QIot_userdata.connectProduct->serverIp) && QIot_userdata.connectProduct != &QIot_userdata.productInfoCache)
    {
        HAL_MEMCPY(QIot_userdata.connectProduct->serverIp, ip, QUOS_IP_ADDR_MAX_LEN);
        Quos_logPrintf(QUEC_CONN, LL_DBG, "ip is changeed, save new ip:%s", QIot_userdata.connectProduct->serverIp);
        Ql_iotDSKVSave();
    }
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotConnEventErrcodeDeal(void)
{
    if (QIot_userdata.connectProduct != &QIot_userdata.productInfoCache)
    {
        return;
    }

    /* 切换连接对象 */
    if (QIot_userdata.connFailCnt > 3)
    {
        if (0 != HAL_STRLEN(QIot_userdata.productInfoCloud.serverUrl))
        {
            QIot_userdata.connectProduct = &QIot_userdata.productInfoCloud;
        }
        else
        {
            QIot_userdata.connectProduct = &QIot_userdata.productInfo;
        }
        QIot_userdata.connFailCnt = 0;
    }
}
/**************************************************************************
** 功能	@brief : 连接失败，获取设置定时器超时时间
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static quint32_t ql_iotConnTimerTimeoutGet(const ql_errcodeType *errcodeType, qint32_t errcode)
{
    qint32_t timeout = SWT_ONE_SECOND;
    quint8_t idx = 0;

    if (errcodeType != NULL)
    {
        while (errcodeType[idx].errcode != 0xFFFFFF)
        {
            if (errcodeType[idx].errcode == errcode)
            {
                if (QL_IOT_ERRCODE_LEVEL_FIFTH == errcodeType[idx].errType)
                {
                    return ql_errTypeTimeout[QL_IOT_ERRCODE_LEVEL_FIFTH];
                }
                timeout = ql_errTypeTimeout[errcodeType[idx].errType];

                break;
            }
            idx++;
        }
    }

    /* 若连续失败超过6次，则强制设置连接间隔为30分钟,并重新打开网络 */
    if (++QIot_userdata.connFailCnt > 6)
    {
        timeout = SWT_ONE_MINUTE * 30;
        return timeout;
    }
    /* 若连接间隔为1秒则需要对连接时间间隔加一个随机时间，随机时间范围是：-500ms~1500ms，防止同一时间过多设备上线 */
    else if (timeout == SWT_ONE_SECOND)
    {
        qint16_t rand = (Qhal_randomGet() % 2000);

        timeout += rand - 500;
    }
    /* 连接失败判断当前是否为cache连接对象，若是则需要切换连接对象为cloud或本地 */
    ql_iotConnEventErrcodeDeal();

    return timeout * QIot_userdata.connFailCnt;
}
/**************************************************************************
** 功能	@brief : 根据命令查找topic类型
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static char *FUNCTION_ATTR_ROM ql_iotConnTopicTypeByCmdGet(quint16_t cmd)
{
    quint32_t i;
    for (i = 0; i < sizeof(QIot_connCmdOutType) / sizeof(QIot_connCmdOutType[0]); i++)
    {
        quint32_t j = 0;
        while (QIot_connCmdOutType[i].cmd[j])
        {
            if (QIot_connCmdOutType[i].cmd[j++] == cmd)
            {
                return QIot_connCmdOutType[i].topicType;
            }
        }
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 接入定时响应
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotConnTimeoutCB(void *swtimer)
{
    if (QIot_userdata.connFailCnt > 6)
    {
        Quos_netOpen();
        QIot_userdata.connFailCnt = 0;
        return;
    }
    Quos_swTimerTimeoutSet(swtimer, SWT_SUSPEND);
    ql_iotConnStart();
}
/**************************************************************************
** 功能	@brief : MQTT事件回调
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotConnEventCb(void *chlFd, qint32_t result)
{
    Quos_logPrintf(QUEC_CONN, LL_DBG, "chlFd[%p] result:%s", chlFd, MQTT_ERR_STRING(result));
    switch (result)
    {
    case QUOS_MQTT_OK_CONNECT:
        break;
    case QUOS_MQTT_ERR_CONNECT:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_UNKNOW);
        break;
    case QUOS_MQTT_ERR_INSIDE:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_DEVICE_INSIDE);
        break;
    case QUOS_MQTT_ERR_PING:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_PING);
        break;
    case QUOS_MQTT_ERR_NET:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_NET);
        break;
    case QUOS_MQTT_UNNACCEPTABLE_PROTOCOL:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_VERSION_NOTFOUND);
        break;
    case QUOS_MQTT_CLIENTID_REJECTED:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_DS_INVALID);
        break;
    case QUOS_MQTT_SERVER_UNAVAILABLE:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_DS_INVALID);
        break;
    case QUOS_MQTT_BAD_USERNAME_OR_PASSWORD:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_DS_INVALID);
        break;
    case QUOS_MQTT_NOT_AUTHORIZED:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_DS_INVALID);
        break;
    case QUOS_MQTT_OK_SUBSCRIBE:
        if (QIOT_STATE_CONNECTING == QIot_userdata.workState)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_SUCC);
            Quos_eventPost(QIOT_ATEVENT_TYPE_SUBCRIBE, (void *)QIOT_SUBCRIBE_SUCC);
        }
        break;
    case QUOS_MQTT_ERR_SUBSCRI:
        Quos_eventPost(QIOT_ATEVENT_TYPE_SUBCRIBE, (void *)QIOT_SUBCRIBE_ERR);
        break;
    default:
        Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_UNKNOW);
        break;
    }
}
/**************************************************************************
** 功能	@brief : MQTT底层接收接口
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotConnRecv(MQTTString *topicName, quint8_t *payload, quint32_t payloadlen)
{
    Quos_logPrintf(QUEC_CONN, LL_DBG, "topic:%.*s", topicName->lenstring.len, topicName->lenstring.data);
    Quos_logHexDump(QUEC_CONN, LL_DUMP, "recv", payload, payloadlen);
#ifdef QUEC_ENABLE_GATEWAY
    QIot_Subdev_t *subdev = NULL;
#endif
    char * p;
    if((p = HAL_STRSTR(topicName->lenstring.data,QIot_userdata.deviceInfo.deviceKey)))
    {
        if (Ql_iotConfigGetSessionFlag() && QIot_userdata.sessionInfo.usable)
        {
            payloadlen = Ql_iotSecureDecryptPayload(payload, payloadlen, QIot_userdata.sessionInfo.key, QIot_userdata.sessionInfo.iv);
            Quos_logHexDump(QUEC_CONN, LL_DUMP, "Decrypt data", payload, payloadlen);
        }
        #if 0
        if (FALSE == Ql_iotDpRawDataPickup(payload, payloadlen, &pkg))
        {
            Quos_logPrintf(QUEC_CONN, LL_ERR, "data pickup fail: %.*s", payloadlen, payload);
            return;
        }
        #endif
    }
#ifdef QUEC_ENABLE_GATEWAY
    else
    {
        subdev = Ql_iotDMSubDevFindByEndPoint(topicName->lenstring.data);
        if (NULL == subdev)
        {
            Quos_logPrintf(QUEC_CONN, LL_ERR, "subdev not connected: %.*s", topicName->lenstring.len, topicName->lenstring.data);
            return;
        }
        else if (subdev->m2msessionInfo.usable)
        {
            payloadlen = Ql_iotSecureDecryptPayload(payload, payloadlen, subdev->m2msessionInfo.key, subdev->m2msessionInfo.iv);
            Quos_logHexDump(QUEC_CONN, LL_DUMP, "Decrypt data", payload, payloadlen);
        }
        #if 0
        if (FALSE == Ql_iotDpRawDataPickup(payload, payloadlen, &pkg))
        {
            return;
        }
        #endif
    }
#endif
    char *cliId = HAL_STRSTR(topicName->lenstring.data, QIOT_SUB_TOPIC_ROOT);
    if (NULL == cliId || 0 == payloadlen)
    {
        return;
    }
    cliId += HAL_STRLEN(QIOT_SUB_TOPIC_ROOT);
    char *words[10];
    quint32_t count = Quos_stringSplit(cliId, topicName->lenstring.len - (cliId - topicName->lenstring.data), words, sizeof(words) / sizeof(words[0]), "/", FALSE);
    if (2 == count || 3 == count)
    {
        topicName->lenstring.len = HAL_SPRINTF(topicName->lenstring.data, "%s", words[0]);
        if (3 == count)
        {
            topicName->lenstring.len += HAL_SPRINTF(topicName->lenstring.data + topicName->lenstring.len, "/%s", words[2]);
        }
#ifdef QUEC_ENABLE_GATEWAY
        if (NULL == subdev)
            Ql_iotDpHandle(QIOT_DPAPP_M2M, topicName->lenstring.data, payload, payloadlen);
        else 
#endif
            Ql_iotDpHandle(QIOT_DPAPP_SUBDEV, topicName->lenstring.data, payload, payloadlen);
    }
}
/**************************************************************************
** 功能	@brief : mqtt发送
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConnSend(const char *endPoint, quint16_t mode, quint16_t cmd, quint16_t srcpkgID, quint16_t pkgId, const quint8_t *payload, quint32_t payloadLen, socketRecvNodeCb_f recvCB)
{
#ifndef QUEC_ENABLE_GATEWAY
    UNUSED(srcpkgID);
#endif
    static quint16_t m2mPkgId = 0;
    if (NULL == QIot_userdata.m2mCtx || mode > 2)
    {
        return FALSE;
    }
    qbool isAck = TRUE;
    char *topicType = ql_iotConnTopicTypeByCmdGet(cmd);
    if (NULL == topicType)
    {
        return FALSE;
    }

    char clientId[QIOT_M2M_CLIENTID_MAXSIZE + 1];
    HAL_SPRINTF(clientId, "qd%s%s", QIot_userdata.connectProduct->productKey, QIot_userdata.deviceInfo.deviceKey);
    if (0 == pkgId)
    {
        isAck = FALSE;
        if(NULL == endPoint || 0 == HAL_STRNCMP(endPoint, clientId, HAL_STRLEN(clientId)))
        {
            pkgId = Ql_iotDpPkgIdUpdate(&m2mPkgId);
        }
#ifdef QUEC_ENABLE_GATEWAY
        else
        {
            QIot_Subdev_t *subdev = Ql_iotDMSubDevFindByEndPoint(endPoint);
            if (NULL != subdev)
            {
                pkgId = Ql_iotDpPkgIdUpdate(&subdev->m2mpkgId);
            }
        }
#endif
    }
#ifdef QUEC_ENABLE_GATEWAY
    if (0 != srcpkgID)
    {
        if (!Ql_iotDMSubDevAddAckmap(endPoint, pkgId, srcpkgID, 0))
        {
            return FALSE;
        }
    }
#endif
    char pubTopic[sizeof(QIOT_PUB_TOPIC_ROOT) + QIOT_M2M_CLIENTID_MAXSIZE * 2 + 1 + 3 + 1] = {0};
    char *next = NULL;
#ifdef QUEC_ENABLE_GATEWAY
    QIot_Subdev_t *subdev = NULL;
#endif
    if (NULL == endPoint)       /* local与m2m交互消息 */
    {
        HAL_SPRINTF(pubTopic, QIOT_PUB_TOPIC_ROOT "%s/%s", clientId, topicType);
    }
    else if ((next = HAL_STRSTR(endPoint, "/")))    /* APP消息 */
    {
        HAL_SPRINTF(pubTopic, QIOT_PUB_TOPIC_ROOT "%.*s/%s/%s", (int)(next - endPoint), endPoint, topicType, next + 1);
    }
    else /* 子设备消息 */
    {
#ifdef QUEC_ENABLE_GATEWAY
        if (0 == HAL_STRNCMP(topicType, "bus", 3))
        {
            subdev = Ql_iotDMSubDevFindByEndPoint(endPoint);
            HAL_SPRINTF(pubTopic, QIOT_PUB_TOPIC_ROOT "%s/%s", endPoint, topicType);
        }
        else
        {
#endif
            HAL_SPRINTF(pubTopic, QIOT_PUB_TOPIC_ROOT "%s/%s", clientId, topicType);
#ifdef QUEC_ENABLE_GATEWAY
        }
#endif
    }

    quint8_t *pkg = NULL;
    payloadLen = Ql_iotDpFormat(&pkg, pkgId, cmd, payload, payloadLen);
    Quos_logPrintf(QUEC_CONN, LL_DBG, "len:%u mode:%d topic:%s", payloadLen, mode, pubTopic);
    if (0 == payloadLen)
    {
        return FALSE;
    }
    qbool ret = FALSE;
#ifdef QUEC_ENABLE_GATEWAY
    if (NULL != subdev && TRUE == subdev->m2msessionInfo.usable)
    {
        quint8_t *outData = NULL;
        payloadLen = Ql_iotSecureEncryptPayload(pkg, payloadLen, &outData, subdev->m2msessionInfo.key, subdev->m2msessionInfo.iv);
        HAL_FREE(pkg);
        if (outData)
        {
            ret = Quos_mqttPublish(QIot_userdata.m2mCtx, pubTopic, NULL, mode, outData, payloadLen, recvCB, isAck) < 0 ? FALSE : TRUE;
            HAL_FREE(outData);
        }
    }
    else if (NULL == subdev && Ql_iotConfigGetSessionFlag() && QIot_userdata.sessionInfo.usable)         /* 主设备使用加密 */
    {
        quint8_t *outData = NULL;
        payloadLen = Ql_iotSecureEncryptPayload(pkg, payloadLen, &outData, QIot_userdata.sessionInfo.key, QIot_userdata.sessionInfo.iv);
        HAL_FREE(pkg);
        if (outData)
        {
            ret = Quos_mqttPublish(QIot_userdata.m2mCtx, pubTopic, NULL, mode, outData, payloadLen, recvCB, isAck) < 0 ? FALSE : TRUE;
            HAL_FREE(outData);
        }
    }
#else
    if (Ql_iotConfigGetSessionFlag() && QIot_userdata.sessionInfo.usable)         /* 主设备使用加密 */
    {
        quint8_t *outData = NULL;
        payloadLen = Ql_iotSecureEncryptPayload(pkg, payloadLen, &outData, QIot_userdata.sessionInfo.key, QIot_userdata.sessionInfo.iv);
        HAL_FREE(pkg);
        if (outData)
        {
            ret = Quos_mqttPublish(QIot_userdata.m2mCtx, pubTopic, NULL, mode, outData, payloadLen, recvCB, isAck) < 0 ? FALSE : TRUE;
            HAL_FREE(outData);
        }
    }
#endif
    else
    {
        ret = Quos_mqttPublish(QIot_userdata.m2mCtx, pubTopic, NULL, mode, pkg, payloadLen, recvCB, isAck) < 0 ? FALSE : TRUE;
        HAL_FREE(pkg);
    }
    return ret;
}

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotConnStart(void)
{
    urlAnalyze_t urlA;
    Quos_logPrintf(QUEC_CONN, LL_DBG, "netIsConn:%s connMode:%s m2mCtx:%p", _BOOL2STR(QIot_userdata.netIsConn), QIOT_CONN_MODE_STRING(QIot_userdata.connMode), QIot_userdata.m2mCtx);
    if (TRUE != QIot_userdata.netIsConn || QIOT_CONNMODE_IDLE == QIot_userdata.connMode || NULL != QIot_userdata.m2mCtx)
    {
        return;
    }
    if (FALSE == Quos_urlAnalyze(QIot_userdata.connectProduct->serverUrl, &urlA))
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_AUTH, (void *)QIOT_AUTH_ERR_SERVER_NOTFOUND);
    }
    else if (0 == HAL_STRLEN(QIot_userdata.productInfo.productKey) || 0 == HAL_STRLEN(QIot_userdata.productInfo.productSecret))
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_AUTH, (void *)QIOT_AUTH_ERR_PKPS_INVALID);
    }
    else
    {
        Quos_netHostnameSetDefault(urlA.hostname, QIot_userdata.connectProduct->serverIp);
        qbool isToAuth = (0 == HAL_STRLEN(QIot_userdata.connectProduct->secret));
        char *password = NULL;
        QIot_userdata.sessionInfo.usable = FALSE;
        if (isToAuth)
        {
            password = Ql_iotSecureGenMqttAuthData(QIot_userdata.connectProduct->productKey, QIot_userdata.connectProduct->productSecret, QIot_userdata.deviceInfo.deviceKey);
        }
        else
        {
            password = Ql_iotSecureGenMqttConnData(QIot_userdata.connectProduct->productSecret, QIot_userdata.connectProduct->secret);
        }
        int subQos = 1;
        char clientId[QIOT_M2M_CLIENTID_MAXSIZE + 1];
        char subTopicRoot[sizeof(QIOT_SUB_TOPIC_ROOT) + sizeof(clientId) + 3];
        HAL_SPRINTF(clientId, "qd%s%s", QIot_userdata.connectProduct->productKey, QIot_userdata.deviceInfo.deviceKey);
        HAL_SPRINTF(subTopicRoot, QIOT_SUB_TOPIC_ROOT "%s/+", clientId);
        char *subTopic[1];
        subTopic[0] = subTopicRoot;
        qbool ret = Quos_mqttInit(&QIot_userdata.m2mCtx, QIot_userdata.connectProduct->serverUrl, clientId, NULL, password, QIot_userdata.lifetime, 1, subTopic, &subQos, ql_iotConnEventCb, ql_iotConnRecv);
        HAL_FREE(password);
        if (TRUE == ret)
        {
            QIot_userdata.workState = isToAuth ? QIOT_STATE_AUTHENTICATING : QIOT_STATE_CONNECTING;
        }
        else
        {
            Quos_eventPost(isToAuth ? QIOT_ATEVENT_TYPE_AUTH : QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_DEVICE_INSIDE);
        }
    }
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotConnStop(void)
{
    Quos_logPrintf(QUEC_CONN, LL_DBG, "netIsConn:%s connMode:%s m2mCtx:%p", _BOOL2STR(QIot_userdata.netIsConn), QIOT_CONN_MODE_STRING(QIot_userdata.connMode), QIot_userdata.m2mCtx);
    if (QIot_userdata.m2mCtx)
    {
        Quos_mqttDeinit(QIot_userdata.m2mCtx);
        QIot_userdata.m2mCtx = NULL;

        if (QIOT_CONNMODE_IDLE == QIot_userdata.connMode && TRUE == QIot_userdata.netIsConn)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_LOGOUT, (void *)QIOT_LOGOUT_SUCC);
            QIot_userdata.workState = QIOT_STATE_DISCONNECTED;
        }
        else if (QIOT_CONNMODE_IDLE != QIot_userdata.connMode && TRUE == QIot_userdata.netIsConn)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_NET);
        }
    }
    Quos_swTimerTimeoutSet(QIot_RunTimer, SWT_SUSPEND);
}

/**************************************************************************
** 功能	@brief : conn事件处理
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotConnEventNotify(qint32_t event, void *arg)
{
    qint32_t errcode = (qint32_t)(pointer_t)arg;
    Quos_logPrintf(QUEC_CONN, LL_DBG, "type:%s errcode:%d connMode:%s netIsConn:%s", QIOT_ATEVENT_TYPE_STRING(event), errcode, QIOT_CONN_MODE_STRING(QIot_userdata.connMode), _BOOL2STR(QIot_userdata.netIsConn));
    switch (event)
    {
    case QIOT_ATEVENT_TYPE_AUTH:
        QIot_userdata.workState = QIOT_AUTH_SUCC == errcode ? QIOT_STATE_AUTHENTICATED : QIOT_STATE_AUTHENTICATE_FAILED;
        if (QIOT_AUTH_SUCC == errcode)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_SUCC);
            Quos_eventPost(QIOT_ATEVENT_TYPE_SUBCRIBE, (void *)QIOT_SUBCRIBE_SUCC);
            ql_iotConnServerInfoCheck();
            QIot_userdata.connFailCnt = 0;
        }
        else
        {
            Quos_swTimerTimeoutSet(QIot_RunTimer, ql_iotConnTimerTimeoutGet(ql_errcodeAuth, errcode));
        }
        break;
    case QIOT_ATEVENT_TYPE_CONN:
    {
        QIot_userdata.workState = QIOT_CONN_SUCC == errcode ? QIOT_STATE_CONNECTED : QIOT_STATE_CONNECT_FAIL;

        if (QIOT_CONN_SUCC == errcode)
        {
            QIot_userdata.connFailCnt = 0;
            Quos_swTimerTimeoutSet(QIot_RunTimer, SWT_SUSPEND);
            ql_iotConnServerInfoCheck();
            if (QIot_userdata.connectProduct == &QIot_userdata.productInfoCache)
            {
                /* 若当前连接服务器信息为缓存区，则需要将数据复制到cloud信息区，并执行保存 */
                HAL_MEMCPY(&QIot_userdata.productInfoCloud, QIot_userdata.connectProduct, sizeof(Qiot_productInfo_t));
                Ql_iotDSKVSave();
            }
        }
        else
        {
            if (QIOT_CONN_ERR_DS_INVALID == errcode)
            {
                HAL_MEMSET(QIot_userdata.connectProduct->secret, 0, sizeof(QIot_userdata.connectProduct->secret));
                Ql_iotDSKVSave();
            }
#ifdef QUEC_ENABLE_GATEWAY
            Ql_iotDMSubDevBusDisconnNotify();
#endif
            Quos_swTimerTimeoutSet(QIot_RunTimer, ql_iotConnTimerTimeoutGet(ql_errcodeConn, errcode));
        }
        break;
    }
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
        switch (errcode)
        {
        case QIOT_SUBCRIBE_SUCC:
            QIot_userdata.workState = QIOT_STATE_SUBSCRIBED;
            break;
        default:
            QIot_userdata.workState = QIOT_STATE_SUBSCRIBE_FAIL;
            Quos_swTimerTimeoutSet(QIot_RunTimer, ql_iotConnTimerTimeoutGet(NULL, errcode));
            break;
        }
        break;
    case QIOT_ATEVENT_TYPE_LOGOUT:
        break;
    case QIOT_ATEVENT_TYPE_SERVER:
        break;
    case QUOS_SYSTEM_EVENT_NETWORK:
        if(QUOS_SEVENT_NET_CONNTIMEOUT == errcode)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_CONN, (void *)QIOT_CONN_ERR_AP);
        }
        else if(QUOS_SEVENT_NET_CONNECTED  == errcode && FALSE == QIot_userdata.netIsConn)
        {
            QIot_userdata.netIsConn = TRUE;
            ql_iotConnStart();
        }
        else if(QUOS_SEVENT_NET_DISCONNECT  == errcode && TRUE == QIot_userdata.netIsConn)
        {
            ql_iotConnStop();
            QIot_userdata.netIsConn = FALSE;
#ifdef QUEC_ENABLE_GATEWAY
            Ql_iotDMSubDevBusDisconnNotify();
#endif
        }
        return;
    default:
        return;
    }
    Ql_iotUrcEventCB(event, errcode, NULL, 0);
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotConnInit(void)
{
    Quos_swTimerStart(&QIot_RunTimer, "quec run", SWT_SUSPEND, 0, ql_iotConnTimeoutCB, NULL);
    qint32_t event[] = {QIOT_ATEVENT_TYPE_AUTH, QIOT_ATEVENT_TYPE_CONN, QIOT_ATEVENT_TYPE_SUBCRIBE, QIOT_ATEVENT_TYPE_LOGOUT, QIOT_ATEVENT_TYPE_SERVER, QUOS_SYSTEM_EVENT_NETWORK};
    Quos_eventCbReg(event, sizeof(event) / sizeof(event[0]), ql_iotConnEventNotify);
}
