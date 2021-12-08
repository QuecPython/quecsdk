/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 
** 功能   @brief   : dmp Syc,适配DMP2.2.0
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "ql_iotCmdSys.h"
#include "ql_iotDp.h"
#include "ql_iotTtlv.h"
#include "ql_iotConfig.h"
#include "ql_iotCmdLoc.h"
#include "Qhal_driver.h"
#ifdef QUEC_ENABLE_GATEWAY
#include "ql_iotGwDev.h"
#endif

enum
{
    QIOT_DEVICE_MANAGE_IP_SET = 0x01,
    QIOT_DEVICE_MANAGE_IP_CLEAR = 0x02,
    QIOT_DEVICE_MANAGE_REAUTH = 0x04,
    QIOT_DEVICE_MANAGE_PKPS_SET = 0x08,
    QIOT_DEVICE_MANAGE_PKPS_CLEAR = 0x10,
    QIOT_DEVICE_MANAGE_NEW_DS = 0x20,
};

/**************************************************************************
** 功能	@brief : 将设备状态赋值ttlv格式,使用完需要Ql_iotTtlvFree()释放资源
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM *Ql_iotSysGetDevStatus(quint16_t ids[], quint32_t size)
{
    void *ttlvHead = NULL;
    Qhal_propertyDev_t dInfo;
    Qhal_propertyNet_t nInfo;
    HAL_MEMSET(&dInfo, 0, sizeof(Qhal_propertyDev_t));
    HAL_MEMSET(&nInfo, 0, sizeof(Qhal_propertyNet_t));
    Qhal_propertyDevGet(&dInfo);
    Qhal_propertyNetGet(&nInfo, NULL, 0, NULL);
    quint32_t i;
    for (i = 0; i < size; i++)
    {
        switch (ids[i])
        {
        case QIOT_DPID_STATUS_BATTERY:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], dInfo.cellLevel);
            break;
        case QIOT_DPID_STATUS_VOLTAGE:
            Ql_iotTtlvIdAddFloat(&ttlvHead, ids[i], dInfo.cellVoltage);
            break;
        case QIOT_DPID_STATUS_SIGNAL:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.rssi);
            break;
        case QIOT_DPID_STATUS_FLASHFREE:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], dInfo.flashFree);
            break;
        case QIOT_DPID_STATUS_RSRP:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.rsrp);
            break;
        case QIOT_DPID_STATUS_RSRQ:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.rsrq);
            break;
        case QIOT_DPID_STATUS_SNR:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.snr);
            break;
        default:
            break;
        }
    }
    return ttlvHead;
}
/**************************************************************************
** 功能	@brief : 设备状态读取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdSysStatusRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    quint16_t ids[QIOT_DPID_STATUS_MAX];
    quint16_t i;
    for (i = 0; i < payloadLen - 1 && i / 2 < QIOT_DPID_STATUS_MAX; i += 2)
    {
        ids[i / 2] = _ARRAY01_U16(&payload[i]);
    }
    void *ttlvHead = Ql_iotSysGetDevStatus(ids, i / 2);
    Ql_iotDpSendTtlvRsp(app, endPoint, QIOT_DPCMD_STATUS_RSP, pkgId, ttlvHead);
    Ql_iotTtlvFree(&ttlvHead);
}
/**************************************************************************
** 功能	@brief : 设备状态上报
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdSysStatusReport(quint16_t ids[], quint32_t size)
{
    qbool ret = FALSE;
    void *ttlvHead = NULL;
    Quos_logPrintf(QUEC_SYS, LL_DBG, "sysStatus report");
    if ((ttlvHead = Ql_iotSysGetDevStatus(ids, size)) != NULL)
    {
        ret = Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M, NULL, 0, 1, QIOT_DPCMD_STATUS_EVENT, ttlvHead, NULL);
        Ql_iotTtlvFree(&ttlvHead);
    }
    return ret;
}
/**************************************************************************
** 功能	@brief : 将设备信息赋值ttlv格式,使用完需要Ql_iotTtlvFree()释放资源
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM *Ql_iotSysGetDevInfo(quint16_t ids[], quint32_t size)
{
    void *ttlvHead = NULL;
    Qhal_propertyDev_t dInfo;
    Qhal_propertyNet_t nInfo;
    Qhal_propertySim_t sInfo;
    HAL_MEMSET(&dInfo, 0, sizeof(Qhal_propertyDev_t));
    HAL_MEMSET(&nInfo, 0, sizeof(Qhal_propertyNet_t));
    HAL_MEMSET(&sInfo, 0, sizeof(Qhal_propertySim_t));
    Qhal_propertyDevGet(&dInfo);
    Qhal_propertyNetGet(&nInfo, NULL, 0, NULL);
    Qhal_propertySimGet(&sInfo);
    quint32_t i;
    for (i = 0; i < size; i++)
    {
        switch (ids[i])
        {
        case QIOT_DPID_INFO_MODEL_TYPE:
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], dInfo.modelType);
            break;
        case QIOT_DPID_INFO_MODEL_VER:
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], QIot_userdata.softversion);
            break;
        case QIOT_DPID_INFO_MCU_VER:
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], QIot_userdata.mcuVerList);
            break;
        case QIOT_DPID_INFO_CELLID:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.cellid);
            break;
        case QIOT_DPID_INFO_ICCID:
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], sInfo.iccid);
            break;
        case QIOT_DPID_INFO_MCC:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.mcc);
            break;
        case QIOT_DPID_INFO_MNC:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.mnc);
            break;
        case QIOT_DPID_INFO_LAC:
            Ql_iotTtlvIdAddInt(&ttlvHead, ids[i], nInfo.lac);
            break;
        case QIOT_DPID_INFO_PHONE_NUM:
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], sInfo.phoneid);
            break;
        case QIOT_DPID_INFO_SIM_NUM:
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], sInfo.imsi);
            break;
        case QIOT_DPID_INFO_SDK_VER:
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], QIOT_SDK_VERSION);
            break;
        case QIOT_DPID_INFO_LOC_SUPLIST:
        {
            void *titleTtlv = Ql_iotLocGetSupList();
            char *titleStr = Ql_iotLocatorTtlv2String(titleTtlv);
            Ql_iotTtlvFree(&titleTtlv);
            Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_INFO_LOC_SUPLIST, titleStr);
            HAL_FREE(titleStr);
            break;
        }
        case QIOT_DPIO_INFO_DP_VER:
        {
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], QIOT_DATA_PROTOCOL_VER);
            break;
        }
        case QIOT_DPIO_INFO_CP_VER:
        {
            Ql_iotTtlvIdAddString(&ttlvHead, ids[i], QIOT_COM_PROTOCOL_VER);
            break;
        }
        default:
            break;
        }
    }
    return ttlvHead;
}
/**************************************************************************
** 功能	@brief : 设备信息读取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdSysDevInfoRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    quint16_t ids[QIOT_DPID_INFO_MAX];
    quint16_t i;
    for (i = 0; i < payloadLen - 1 && i / 2 < QIOT_DPID_INFO_MAX; i += 2)
    {
        ids[i / 2] = _ARRAY01_U16(&payload[i]);
    }
    void *ttlvHead = Ql_iotSysGetDevInfo(ids, i / 2);
    Ql_iotDpSendTtlvRsp(app, endPoint, QIOT_DPCMD_INFO_RSP, pkgId, ttlvHead);
    Ql_iotTtlvFree(&ttlvHead);
}
/**************************************************************************
** 功能	@brief : 设备信息上报
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdSysDevInfoReport(quint16_t ids[], quint32_t size)
{
    qbool ret = FALSE;
    void *ttlvHead = NULL;
    Quos_logPrintf(QUEC_SYS, LL_DBG, "devinfo report");
    if ((ttlvHead = Ql_iotSysGetDevInfo(ids, size)) != NULL)
    {
        ret = Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M, NULL, 0, 1, QIOT_DPCMD_INFO_EVENT, ttlvHead, NULL);
        Ql_iotTtlvFree(&ttlvHead);
    }
    return ret;
}
/**************************************************************************
** 功能	@brief : 上报绑定信息，仅在有局域网通信时有效
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBindcodeReport(quint8_t bindcode[], quint32_t len)
{
    qbool ret = FALSE;
    void *ttlvHead = NULL;
    Quos_logPrintf(QUEC_SYS, LL_DBG, "bindcode report");
    Ql_iotTtlvIdAddByte(&ttlvHead, QIOT_DPID_DEV_BINDCODE, bindcode, len);

    if (ttlvHead)
    {
        ret = Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M, NULL, 0, 1, QIOT_DPCMD_DEV_BINDCODE_WRITE, ttlvHead, NULL);
        Ql_iotTtlvFree(&ttlvHead);
    }
    return ret;
}
/**************************************************************************
** 功能	@brief : 上报错误信息
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdSysExceReport(QIot_dpAppType_e app, const char *endPoint, qint32_t errcode, quint16_t pkgId)
{
    void *ttlvHead = NULL;
    Quos_logPrintf(QUEC_SYS, LL_WARN, "ecode:%s pkgid:%u", QIOT_SERVER_ERRCODE_STRING(errcode), pkgId);
    Ql_iotTtlvIdAddInt(&ttlvHead, QIOT_DPID_EXCE_ERRCODE, errcode);
    Ql_iotTtlvIdAddInt(&ttlvHead, QIOT_DPID_EXCE_PKGID, pkgId);
    Ql_iotDpSendTtlvReq(app, endPoint, 0, 1, QIOT_DPCMD_EXCE_EVENT, ttlvHead, NULL);
    Ql_iotTtlvFree(&ttlvHead);
}

/**************************************************************************
** 功能	@brief : 设备管理命令处理
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void Ql_iotCmdSysTerManage(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(app);
    UNUSED(endPoint);
    UNUSED(pkgId);
    qint32_t errcode = 0;
    quint16_t flag = 0;
    quint8_t event = 0;
    qbool need_save = FALSE;
    //Qiot_productType pt = QIot_userdata.productType;
    void *ttlvHead = Ql_iotTtlvUnformat(payload, payloadLen);
    if (NULL == ttlvHead)
    {
        return;
    }

    /**************************************************设备认证信息提取******************************************************************************/
    char *ds = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_DEV_SECRET);
    if (ds && HAL_STRLEN(ds))
    {
        HAL_SNPRINTF(QIot_userdata.connectProduct->secret, sizeof(QIot_userdata.connectProduct->secret), "%s", ds);
        Ql_iotDSKVSave();
        event = QIOT_ATEVENT_TYPE_AUTH;
        errcode = QIOT_AUTH_SUCC;
    }
    uint8_t *sessionKey = NULL;
    quint32_t sessionKeyLen = Ql_iotTtlvIdGetByte(ttlvHead, QIOT_DPID_DEV_SESSIONKEY, &sessionKey);
    if (sessionKey && sessionKeyLen)
    {
        SHA256_ctx_t sha256_ctx;
        quint8_t sha256Data[QUOS_SHA256_DIGEST_LENGTH];
        Quos_logHexDump(QUEC_SYS, LL_DUMP, "sessionKey", sessionKey, sessionKeyLen);
        HAL_MEMCPY(QIot_userdata.sessionInfo.key, sessionKey, sizeof(QIot_userdata.sessionInfo.key));
        Quos_sha256init(&sha256_ctx);
        Quos_sha256update(&sha256_ctx, (const quint8_t *)QIot_userdata.sessionInfo.key, sizeof(QIot_userdata.sessionInfo.key));
        Quos_sha256finish(&sha256_ctx, sha256Data);
        HAL_MEMCPY(QIot_userdata.sessionInfo.iv, sha256Data, sizeof(QIot_userdata.sessionInfo.iv));
        QIot_userdata.sessionInfo.usable = TRUE;
    }
    /*******************************************************设备控制命令提取****************************************************************************/
    void *dnsInfo = Ql_iotTtlvIdGetStruct(ttlvHead, QIOT_DPID_DEV_DNS);
    if (dnsInfo)
    {
        HAL_MEMSET(&QIot_userdata.productInfoCache, 0x00, sizeof(QIot_userdata.productInfoCache));

        char *domain = Ql_iotTtlvIdGetString(dnsInfo, QIOT_DPID_DEV_DOMAIN);
        char *ip = Ql_iotTtlvIdGetString(dnsInfo, QIOT_DPID_DEV_IP);
        qint64_t port = 0;
        qbool ret = Ql_iotTtlvIdGetInt(dnsInfo, QIOT_DPID_DEV_PORT, &port);
        /* 域名 ip 端口号等3个信息需要同时下发，否则认为无效 */
        if (domain && ip && ret)
        {
            if (HAL_STRLEN(domain) && HAL_STRLEN(ip) && port <= 65535)
            {
                urlAnalyze_t urlA;

                if (FALSE == Quos_urlAnalyze(QIot_userdata.connectProduct->serverUrl, &urlA))
                {
                    ret = TRUE;
                }

                if (0 == HAL_STRCMP(urlA.hostname, domain) && port == urlA.port)
                {
                    ret = FALSE;
                }
                if (ret == TRUE)
                {
                    HAL_SNPRINTF(QIot_userdata.productInfoCache.serverUrl, sizeof(QIot_userdata.productInfoCache.serverUrl), "%s:%d", domain, (quint16_t)port);
                    HAL_SNPRINTF(QIot_userdata.productInfoCache.serverIp, sizeof(QIot_userdata.productInfoCache.serverIp), "%s", ip);
                    QIot_userdata.connectProduct = &QIot_userdata.productInfoCache;
                    flag |= QIOT_DEVICE_MANAGE_IP_SET;
                    event = QIOT_ATEVENT_TYPE_CONN;
                    errcode = QIOT_CONN_ERR_SERVER_CHANGE;
                }
            }
            /* 下发内容为空，清除云平台下发的服务器信息 */
            else if (0 == HAL_STRLEN(domain) && 0 == HAL_STRLEN(ip) && 0 == port)
            {
                HAL_MEMSET(QIot_userdata.productInfoCloud.serverUrl, 0x00, sizeof(QIot_userdata.productInfoCloud.serverUrl));
                HAL_MEMSET(QIot_userdata.productInfoCloud.serverIp, 0x00, sizeof(QIot_userdata.productInfoCloud.serverIp));
                //QIot_userdata.connectProduct = &QIot_userdata.productInfo;
                flag |= QIOT_DEVICE_MANAGE_IP_CLEAR;
                QIot_userdata.connectProduct = &QIot_userdata.productInfoCache;
                need_save = TRUE;
            }
        }
    }
    char *pk = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_DEV_PK);
    char *ps = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_DEV_PS);
    if (pk && ps)
    {
        quint16_t pk_len = HAL_STRLEN(pk);
        quint16_t ps_len = HAL_STRLEN(ps);
        if (pk_len && ps_len)
        {
            if (0 != HAL_STRCMP(pk, QIot_userdata.connectProduct->productKey) || 0 != HAL_STRCMP(ps, QIot_userdata.connectProduct->productSecret))
            {
                HAL_SNPRINTF(QIot_userdata.productInfoCache.productKey, sizeof(QIot_userdata.productInfoCache.productKey), "%s", pk);
                HAL_SNPRINTF(QIot_userdata.productInfoCache.productSecret, sizeof(QIot_userdata.productInfoCache.productSecret), "%s", ps);
                flag |= QIOT_DEVICE_MANAGE_PKPS_SET;
                event = QIOT_ATEVENT_TYPE_CONN;
                errcode = QIOT_CONN_ERR_SERVER_CHANGE;
                QIot_userdata.connectProduct = &QIot_userdata.productInfoCache;
            }
        }
        else if (0 == ps_len && 0 == pk_len)
        {
            /* 清除pk ps */
            HAL_MEMSET(QIot_userdata.productInfoCloud.productKey, 0x00, sizeof(QIot_userdata.productInfoCloud.productKey));
            HAL_MEMSET(QIot_userdata.productInfoCloud.productSecret, 0x00, sizeof(QIot_userdata.productInfoCloud.productSecret));
            flag |= QIOT_DEVICE_MANAGE_PKPS_CLEAR;
            QIot_userdata.connectProduct = &QIot_userdata.productInfoCache;
            need_save = TRUE;
        }
    }

    if ((flag & (QIOT_DEVICE_MANAGE_PKPS_SET | QIOT_DEVICE_MANAGE_IP_SET)) == QIOT_DEVICE_MANAGE_PKPS_SET)
    {
        /* 平台仅设置了pk ps */
        if (HAL_STRLEN(QIot_userdata.productInfoCloud.serverUrl))
        {
            HAL_SNPRINTF(QIot_userdata.connectProduct->serverUrl, sizeof(QIot_userdata.connectProduct->serverUrl), "%s", QIot_userdata.productInfoCloud.serverUrl);
            HAL_SNPRINTF(QIot_userdata.connectProduct->serverIp, sizeof(QIot_userdata.connectProduct->serverIp), "%s", QIot_userdata.productInfoCloud.serverIp);
        }
        else
        {
            HAL_SNPRINTF(QIot_userdata.connectProduct->serverUrl, sizeof(QIot_userdata.connectProduct->serverUrl), "%s", QIot_userdata.productInfo.serverUrl);
            HAL_SNPRINTF(QIot_userdata.connectProduct->serverIp, sizeof(QIot_userdata.connectProduct->serverIp), "%s", QIot_userdata.productInfo.serverIp);
        }
    }
    else if ((flag & (QIOT_DEVICE_MANAGE_PKPS_SET | QIOT_DEVICE_MANAGE_IP_SET)) == QIOT_DEVICE_MANAGE_IP_SET)
    {
        /* 平台仅设置了DNS */
        if (HAL_STRLEN(QIot_userdata.productInfoCloud.productKey))
        {
            HAL_SNPRINTF(QIot_userdata.connectProduct->productKey, sizeof(QIot_userdata.connectProduct->productKey), "%s", QIot_userdata.productInfoCloud.productKey);
            HAL_SNPRINTF(QIot_userdata.connectProduct->productSecret, sizeof(QIot_userdata.connectProduct->productSecret), "%s", QIot_userdata.productInfoCloud.productSecret);
        }
        else
        {
            HAL_SNPRINTF(QIot_userdata.connectProduct->productKey, sizeof(QIot_userdata.connectProduct->productKey), "%s", QIot_userdata.productInfo.productKey);
            HAL_SNPRINTF(QIot_userdata.connectProduct->productSecret, sizeof(QIot_userdata.connectProduct->productSecret), "%s", QIot_userdata.productInfo.productSecret);
        }
    }
    else if (flag == (QIOT_DEVICE_MANAGE_PKPS_CLEAR | QIOT_DEVICE_MANAGE_IP_CLEAR))
    {
        /* 若平台同时清除DNS与pk ps信息，则需要在此切换连接平台对象为本地 */
        QIot_userdata.connectProduct = &QIot_userdata.productInfo;
        event = QIOT_ATEVENT_TYPE_CONN;
        errcode = QIOT_CONN_ERR_SERVER_CHANGE;
        need_save = TRUE;
    }

    qbool reauth_flag = FALSE;
    qbool ret = Ql_iotTtlvIdGetBool(ttlvHead, QIOT_DPID_DEV_REAUTH, &reauth_flag);
    char *new_ds = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_DEV_NEW_SECRET);

    if (ret && reauth_flag)
    {
        /* 重新认证标志下发未TRUE, 清除认证信息--若未下发pk ps dns，则清除当前连接对象 */
        flag |= QIOT_DEVICE_MANAGE_REAUTH;
        HAL_MEMSET(QIot_userdata.connectProduct->secret, 0x00, sizeof(QIot_userdata.connectProduct->secret));
        event = QIOT_ATEVENT_TYPE_CONN;
        errcode = QIOT_CONN_ERR_SERVER_CHANGE;
    }
    else if (new_ds && HAL_STRLEN(new_ds))
    {
        if (0 != HAL_STRCMP(new_ds, QIot_userdata.connectProduct->secret))
        {
            /* 新的ds信息保存到即将连接的对象里，--若未下发pk ps dns，则覆盖当前连接中ds信息 */
            HAL_SNPRINTF(QIot_userdata.connectProduct->secret, sizeof(QIot_userdata.connectProduct->secret), "%s", new_ds);

            event = QIOT_ATEVENT_TYPE_CONN;
            errcode = QIOT_CONN_ERR_SERVER_CHANGE;
            need_save = TRUE;
        }
    }
    else if (0 != (flag & (QIOT_DEVICE_MANAGE_PKPS_SET | QIOT_DEVICE_MANAGE_IP_SET)))
    {
        /* 若平台下发了pk ps 或dns，且没有下发重新认证标志与新的ds数据，设备需要从其他地方拷贝ds到当前缓存信息对象中 */
        if (HAL_STRLEN(QIot_userdata.productInfoCloud.secret))
        {
            HAL_SNPRINTF(QIot_userdata.connectProduct->secret, sizeof(QIot_userdata.connectProduct->secret), "%s", QIot_userdata.productInfoCloud.secret);
        }
        else
        {
            HAL_SNPRINTF(QIot_userdata.connectProduct->secret, sizeof(QIot_userdata.connectProduct->secret), "%s", QIot_userdata.productInfo.secret);
        }

        event = QIOT_ATEVENT_TYPE_CONN;
        errcode = QIOT_CONN_ERR_SERVER_CHANGE;
    }

    if (need_save)
    {
        if (QIot_userdata.connectProduct != &QIot_userdata.productInfoCache)
        {
            Ql_iotDSKVSave();
        }
    }

    if (errcode != 0)
    {
        if (errcode != QIOT_AUTH_SUCC)
        {
            Quos_mqttDeinit(QIot_userdata.m2mCtx);
            QIot_userdata.m2mCtx = NULL;
        }
        /* 设置完新的域名后，是直接连接DMP还是重新发起认证 */
        Quos_eventPost(event, (void *)(long)errcode);
    }

    Ql_iotTtlvFree(&ttlvHead);
}
/**************************************************************************
** 功能	@brief : 设备管理命令处理
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void Ql_iotCmdSysExceWrite(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(app);
    UNUSED(endPoint);
    UNUSED(pkgId);
    void *ttlvHead = Ql_iotTtlvUnformat(payload, payloadLen);
    if (NULL == ttlvHead)
    {
        return;
    }
    qint64_t errcode = 0, errPkgId = 0;
    qbool retErrCode = Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_EXCE_ERRCODE, &errcode);
    qbool retErrPkgId = Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_EXCE_PKGID, &errPkgId);
    Quos_logPrintf(QUEC_SYS, LL_INFO, "code:[%d]%s pkgid:%u", (int)errcode, retErrCode ? QIOT_SERVER_ERRCODE_STRING(errcode) : "unknown", retErrPkgId ? (int)errPkgId : 0);
    if (retErrCode && errcode > 10200)
    {
        switch (errcode)
        {
        case QIOT_AUTH_ERR_DONE:
        case QIOT_AUTH_ERR_PKPS_INVALID:
        case QIOT_AUTH_ERR_PAYLOAD_INVALID:
        case QIOT_AUTH_ERR_SIGN_INVALID:
        case QIOT_AUTH_ERR_VERSION_INVALID:
        case QIOT_AUTH_ERR_HASH_INVALID:
        case QIOT_AUTH_ERR_PK_CHANGE:
        case QIOT_AUTH_ERR_DK_ILLEGAL:
        case QIOT_AUTH_ERR_PK_VER_NOCOR:
            Quos_mqttDeinit(QIot_userdata.m2mCtx);
            QIot_userdata.m2mCtx = NULL;
            Quos_eventPost(QIOT_ATEVENT_TYPE_AUTH, (void *)(pointer_t)errcode);
            break;
#ifdef QUEC_ENABLE_GATEWAY
        case QIOT_SUB_DEV_ERR_UNLOGIN:
        {
            char *dk = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_EXCE_DK);
            char *pk = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_EXCE_PK);
            if (pk && dk)
            {
                Ql_gatewayDeviceExceWrite(dk, pk, errcode);
            }
            break;
        }
#endif
        case QIOT_SERVER_ERRCODE_RATE_LIMIT:
        case QIOT_SERVER_ERRCODE_QUANTITY_LIMIT:
        default:
            Quos_eventPost(QIOT_ATEVENT_TYPE_SERVER, (void *)(pointer_t)errcode);
            break;
        }
    }
    Ql_iotTtlvFree(&ttlvHead);
}
/**************************************************************************
** 功能	@brief : sys事件处理
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotCmdSysEventNotify(qint32_t event, void *arg)
{
    qint32_t errcode = (qint32_t)(pointer_t)arg;
    switch (event)
    {
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
        if (QIOT_SUBCRIBE_SUCC == errcode)
        {
            quint16_t statusIds[] = {QIOT_DPID_STATUS_BATTERY,
                                     QIOT_DPID_STATUS_VOLTAGE,
                                     QIOT_DPID_STATUS_SIGNAL,
                                     QIOT_DPID_STATUS_FLASHFREE};
            quint16_t infoIds[] = {QIOT_DPID_INFO_MODEL_TYPE,
                                   QIOT_DPID_INFO_MODEL_VER,
                                   QIOT_DPID_INFO_MCU_VER,
                                   QIOT_DPID_INFO_ICCID,
                                   QIOT_DPID_INFO_SDK_VER,
                                   QIOT_DPID_INFO_LOC_SUPLIST,
                                   QIOT_DPIO_INFO_DP_VER,
                                   QIOT_DPIO_INFO_CP_VER};
            Ql_iotCmdSysStatusReport(statusIds, sizeof(statusIds) / sizeof(statusIds[0]));
            Ql_iotCmdSysDevInfoReport(infoIds, sizeof(infoIds) / sizeof(infoIds[0]));
        }
        break;
    }
}
/**************************************************************************
** 功能	@brief : Sys任务初始化
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdSysInit(void)
{
    qint32_t event[] = {QIOT_ATEVENT_TYPE_SUBCRIBE};
    Quos_eventCbReg(event, sizeof(event) / sizeof(event[0]), ql_iotCmdSysEventNotify);
}