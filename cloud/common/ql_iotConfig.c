/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : quecthing公共信息
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "ql_iotConfig.h"
#include "ql_iotCmdOTA.h"
#include "ql_iotCmdBus.h"
#include "ql_iotCmdSys.h"
#include "ql_iotSecure.h"
#include "ql_iotConn.h"
#include "ql_iotCmdLan.h"
#ifdef QUEC_ENABLE_HTTP_OTA
#include "ql_fotaConfig.h"
#endif
#ifdef QUEC_ENABLE_GATEWAY
#include "ql_iotDMSub.h"
#endif
#include "Qhal_driver.h"

QIot_userData_t QIot_userdata;

static const dsKeyValue_t QIotDSKV[] =
    {
#ifdef QUEC_ENABLE_AT
        {0x00, FALSE, &QIot_userdata.connMode, sizeof(QIot_userdata.connMode)},
        {0x06, FALSE, &QIot_userdata.deviceInfo.contextID, sizeof(QIot_userdata.deviceInfo.contextID)},
        {0x11, FALSE, &QIot_userdata.lifetime, sizeof(QIot_userdata.lifetime)},
        {0x12, TRUE, QIot_userdata.mcuVerList, sizeof(QIot_userdata.mcuVerList)},
        {0x14, FALSE, &QIot_userdata.sessionInfo.flag, sizeof(QIot_userdata.sessionInfo.flag)},
        {0x0F, TRUE, QIot_userdata.deviceInfo.deviceKey, sizeof(QIot_userdata.deviceInfo.deviceKey)},
#endif
        {0x04, TRUE, QIot_userdata.productInfo.productKey, sizeof(QIot_userdata.productInfo.productKey)},
        {0x05, TRUE, QIot_userdata.productInfo.productSecret, sizeof(QIot_userdata.productInfo.productSecret)},
        {0x02, TRUE, QIot_userdata.productInfo.serverUrl, sizeof(QIot_userdata.productInfo.serverUrl)},
        {0x10, TRUE, QIot_userdata.productInfo.secret, sizeof(QIot_userdata.productInfo.secret)},

        {0x30, TRUE, QIot_otaInfo.componentNo, sizeof(QIot_otaInfo.componentNo)},
        {0x31, TRUE, QIot_otaInfo.targetVersion, sizeof(QIot_otaInfo.targetVersion)},
        {0x32, FALSE, &QIot_otaInfo.componentType, sizeof(QIot_otaInfo.componentType)},
        {0x33, FALSE, &QIot_otaInfo.currStatus, sizeof(QIot_otaInfo.currStatus)},
        {0x34, FALSE, &QIot_otaInfo.extraMess, sizeof(QIot_otaInfo.extraMess)},
#ifdef QUEC_ENABLE_AT
        {0x50, FALSE, &QIot_busInfo.recvIsBuffer, sizeof(QIot_busInfo.recvIsBuffer)},
        {0x51, FALSE, &QIot_busInfo.dataFormat, sizeof(QIot_busInfo.dataFormat)},
#endif
        {0x07, TRUE, QIot_userdata.productInfo.serverIp, sizeof(QIot_userdata.productInfo.serverIp)},
        {0x08, TRUE, QIot_userdata.productInfoCloud.serverUrl, sizeof(QIot_userdata.productInfoCloud.serverUrl)},
        {0x09, TRUE, QIot_userdata.productInfoCloud.serverIp, sizeof(QIot_userdata.productInfoCloud.serverIp)},
        {0x0A, TRUE, QIot_userdata.productInfoCloud.productKey, sizeof(QIot_userdata.productInfoCloud.productKey)},
        {0x0B, TRUE, QIot_userdata.productInfoCloud.productSecret, sizeof(QIot_userdata.productInfoCloud.productSecret)},
        {0x0C, TRUE, QIot_userdata.productInfoCloud.secret, sizeof(QIot_userdata.productInfoCloud.secret)},
#ifdef QUEC_ENABLE_HTTP_OTA
        {0xF0, TRUE, Qota_userdata.productKey, sizeof(Qota_userdata.productKey)},
        {0xF1, TRUE, Qota_userdata.productSecret, sizeof(Qota_userdata.productSecret)},
        {0xF2, TRUE, Qota_userdata.serverUrl, sizeof(Qota_userdata.serverUrl)},
        {0xF3, TRUE, Qota_userdata.uid, sizeof(Qota_userdata.uid)},
        {0xF4, FALSE, &Qota_userdata.tlsMode, sizeof(Qota_userdata.tlsMode)},
        {0xF5, FALSE, &Qota_userdata.status, sizeof(Qota_userdata.status)},
        {0xF6, TRUE, Qota_userdata.targetVersion, sizeof(Qota_userdata.targetVersion)},
        {0xF7, FALSE, &Qota_userdata.tokenOvertime, sizeof(Qota_userdata.tokenOvertime)},
        {0xF8, TRUE, Qota_userdata.token, sizeof(Qota_userdata.token)},
#endif
        {0, FALSE, NULL, 0}};
/**************************************************************************
** 功能	@brief : Quec IOT初始化，需要在使用IOT服务前调用
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotInit(void)
{
    HAL_MEMSET(&QIot_userdata, 0, sizeof(QIot_userdata));
    Quos_kernelInit();
    if (FALSE == Qhal_beforeMain())
    {
        return FALSE;
    }
    Qhal_propertyDev_t dInfo;
    Qhal_propertyDevGet(&dInfo);
    Ql_iotConnInit();
    Ql_iotCmdBusInit();
    Ql_iotCmdSysInit();
    Ql_iotCmdOtaInit();
#ifdef QUEC_ENABLE_LAN
    Ql_iotCmdLanInit();
#endif
    char key[QUOS_AES_KEYLEN];
    HAL_MEMSET(key, '1', QUOS_AES_KEYLEN);
    HAL_STRCPY(key, Qhal_devUuidGet());
    if (FALSE == Quos_dsKvRead(QIOT_FILE_CONFIG, QIotDSKV, key))
    {
        Quos_logPrintf(QUEC_IOT, LL_DBG, "read dskv with aes fail");
        qbool kvret = Quos_dsKvRead(QIOT_FILE_CONFIG, QIotDSKV, NULL);
        Quos_logPrintf(QUEC_IOT, LL_DBG, "read dskv without aes %s", _BOOL2STR(kvret));
    }
    QIot_userdata.deviceInfo.contextID = dInfo.pdpCxtIdMin;
#ifdef QUEC_ENABLE_AT
    if (0 == HAL_STRLEN(QIot_userdata.deviceInfo.deviceKey))
#endif
    {
        HAL_SPRINTF(QIot_userdata.deviceInfo.deviceKey, "%s", Qhal_devUuidGet());
    }
    if (0 == HAL_STRLEN(QIot_userdata.productInfo.serverUrl))
    {
        HAL_SPRINTF(QIot_userdata.productInfo.serverUrl, "%s", QIOT_DMP_SERVERURL_MQTT_DEFAULT);
        HAL_SPRINTF(QIot_userdata.productInfo.serverIp, "%s", QIOT_DMP_IP_MQTT_DEFAULT);
    }
    else if (NULL != HAL_STRSTR(QIot_userdata.productInfo.serverUrl, QIOT_DMP_SERVERURL_HTTP_DEFAULT))
    {
        HAL_SPRINTF(QIot_userdata.productInfo.serverUrl, "%s", QIOT_DMP_SERVERURL_MQTT_DEFAULT);
        HAL_SPRINTF(QIot_userdata.productInfo.serverIp, "%s", QIOT_DMP_IP_MQTT_DEFAULT);
    }
    else if (NULL != HAL_STRSTR(QIot_userdata.productInfo.serverUrl, QIOT_DMP_SERVERURL_HTTPS_DEFAULT))
    {
        HAL_SPRINTF(QIot_userdata.productInfo.serverUrl, "%s", QIOT_DMP_SERVERURL_MQTTS_DEFAULT);
        HAL_SPRINTF(QIot_userdata.productInfo.serverIp, "%s", QIOT_DMP_IP_MQTT_DEFAULT);
    }
    if (0 == QIot_userdata.lifetime)
    {
        QIot_userdata.lifetime = 120;
    }
    /* 若云平台设置过服务器信息，则需要使用云平台设置的信息连接平台，不能切换到本地设置的信息中 */
    if (HAL_STRLEN(QIot_userdata.productInfoCloud.serverUrl))
    {
        QIot_userdata.connectProduct = &QIot_userdata.productInfoCloud;
        Quos_logPrintf(QUEC_IOT, LL_INFO, "connect server obj cloud");
    }
    else
    {
        Quos_logPrintf(QUEC_IOT, LL_INFO, "connect server obj local");
        QIot_userdata.connectProduct = &QIot_userdata.productInfo;
    }

    Ql_iotConfigSetAppVersion(NULL);
    Ql_iotCmdOtaStatusRecovery();

#ifdef QUEC_ENABLE_HTTP_OTA
    Ql_fotaStatusRecovery();
#endif
    QIot_userdata.workState = QIOT_STATE_INITIALIZED;
    if (QIOT_CONNMODE_REQ == QIot_userdata.connMode)
    {
        QIot_userdata.connMode = QIOT_CONNMODE_IDLE;
    }
    else if (QIOT_CONNMODE_AUTO == QIot_userdata.connMode)
    {
        Quos_netOpen();
    }

#ifdef QUEC_ENABLE_GATEWAY
    Ql_iotSubDevManagerInit();
    extern void Ql_gatewayDevicePackSubDisconnMsg(const char *pk, const char *dk, quint16_t pkgId);
    Ql_iotDMSubDevDeleteAPIregister(QIOT_DEV_SUBDEV_DIRECT, Ql_gatewayDevicePackSubDisconnMsg);
#endif
    //xjin.gao 20211206 adaptation//depot10/quecthing/quecthingSDK/2.9.0/ Initialize quecSDK Thread, detect events
    Qhal_quecsdk_init();

    Quos_logPrintf(QUEC_IOT, LL_INFO, "DK   \t:%s", QIot_userdata.deviceInfo.deviceKey);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "quecthingSDK\t:%s", QIOT_SDK_VERSION);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "firmware VER\t:%s", QIot_userdata.softversion);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "MCU VER\t:%s", QIot_userdata.mcuVerList);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "connmode\t:%s", QIOT_CONN_MODE_STRING(QIot_userdata.connMode));
    Quos_logPrintf(QUEC_IOT, LL_INFO, "contextID\t:%u", QIot_userdata.deviceInfo.contextID);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "sertype\t:%s", "MQTT");
    Quos_logPrintf(QUEC_IOT, LL_INFO, "serUrl\t:%s", QIot_userdata.productInfo.serverUrl);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "PK    \t:%s", QIot_userdata.productInfo.productKey);
    /*Quos_logPrintf(QUEC_IOT, LL_DBG, "PS\t:%s", QIot_userdata.productInfo.productSecret);*/
    Quos_logPrintf(QUEC_IOT, LL_INFO, "authVer\t:%s", Ql_iotSecureVerGet());
    /*Quos_logPrintf(QUEC_IOT, LL_INFO, "ds\t:%s", QIot_userdata.deviceInfo.secret);*/
    Quos_logPrintf(QUEC_IOT, LL_INFO, "lifetime\t:%u", QIot_userdata.lifetime);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "sessionFlag\t:%d", QIot_userdata.sessionInfo.flag);
    Quos_logPrintf(QUEC_IOT, LL_INFO, "buffer mode\t:%s", _BOOL2STR(QIot_busInfo.recvIsBuffer));
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 保存配置信息
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotDSKVSave(void)
{
    char key[QUOS_AES_KEYLEN];
    HAL_MEMSET(key, '1', QUOS_AES_KEYLEN);
    HAL_STRCPY(key, Qhal_devUuidGet());
    return Quos_dsKvWrite(QIOT_FILE_CONFIG, QIotDSKV, key);
}
/**************************************************************************
** 功能	@brief : 产品配置-连接模式配置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigSetConnmode(QIot_connMode_e mode)
{
#ifndef QUEC_ENABLE_AT
    if (QIOT_CONNMODE_AUTO == mode)
    {
        return FALSE;
    }
#endif
    /* 判断当前系统服务器地址(url)以及其他配置是否为空 */
    if (QIOT_CONNMODE_IDLE != mode &&
        (mode > QIOT_CONNMODE_AUTO ||
         (HAL_STRLEN(QIot_userdata.productInfo.productKey) == 0 ||
          HAL_STRLEN(QIot_userdata.productInfo.productSecret) == 0 ||
          HAL_STRLEN(QIot_userdata.productInfo.serverUrl) == 0)))
    {
        return FALSE;
    }

    if (mode != QIot_userdata.connMode)
    {
        QIot_connMode_e oldStatue = QIot_userdata.connMode;
        QIot_userdata.connMode = mode;
#ifdef QUEC_ENABLE_AT
        Ql_iotDSKVSave();
#endif
        if (QIOT_CONNMODE_IDLE == mode)
        {
            QIot_userdata.sessionInfo.usable = FALSE;
            QIot_userdata.connFailCnt = 0;
            Quos_netClose();
        }
        else if(QIOT_CONNMODE_IDLE == oldStatue)
        {
            Quos_netOpen();
        }
    }
    return TRUE;
}

/**************************************************************************
** 功能	@brief : 产品配置-连接模式获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
QIot_connMode_e Ql_iotConfigGetConnmode(void)
{
    return QIot_userdata.connMode;
}
/**************************************************************************
** 功能	@brief : 产品配置-PDP contextID配置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotConfigSetPdpContextId(quint8_t contextID)
{
    Qhal_propertyDev_t dInfo;

    Qhal_propertyDevGet(&dInfo);
    if (contextID > dInfo.pdpCxtIdMax || contextID < dInfo.pdpCxtIdMin || QIOT_CONNMODE_IDLE != Ql_iotConfigGetConnmode())
    {
        return FALSE;
    }

    if (contextID != QIot_userdata.deviceInfo.contextID)
    {
        QIot_userdata.deviceInfo.contextID = contextID;
#ifdef QUEC_ENABLE_AT
        Ql_iotDSKVSave();
#endif
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 产品配置-PDP contextID获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint8_t Ql_iotConfigGetPdpContextId(void)
{
    return QIot_userdata.deviceInfo.contextID;
}
/**************************************************************************
** 功能	@brief : 产品配置-服务器信息配置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigSetServer(QIot_protocolType_t type, const char *server_url)
{
    qbool needsave = FALSE;
    if (QIOT_CONNMODE_IDLE != Ql_iotConfigGetConnmode() || NULL == server_url || 0 == HAL_STRLEN(server_url))
    {
        return FALSE;
    }
    if (QIOT_PPROTOCOL_MQTT != type)
    {
        return FALSE;
    }
    if (NULL != HAL_STRSTR(server_url, QIOT_DMP_SERVERURL_HTTP_DEFAULT))
    {
        server_url = QIOT_DMP_SERVERURL_MQTT_DEFAULT;
    }
    else if (NULL != HAL_STRSTR(server_url, QIOT_DMP_SERVERURL_HTTPS_DEFAULT))
    {
        server_url = QIOT_DMP_SERVERURL_MQTTS_DEFAULT;
    }
    if (HAL_STRLEN(server_url) < sizeof(QIot_userdata.productInfo.serverUrl) && 0 != HAL_STRCMP(QIot_userdata.productInfo.serverUrl, server_url))
    {
        HAL_SPRINTF(QIot_userdata.productInfo.serverUrl, "%s", server_url);
        /* 设置新的域名与当前本地保存域名不一致清除本地域名解析得到IP地址 */
        HAL_MEMSET(QIot_userdata.productInfo.serverIp, 0x00, sizeof(QIot_userdata.productInfo.serverIp));
        needsave = TRUE;
    }
    if (needsave)
    {
        HAL_MEMSET(&QIot_userdata.productInfoCache, 0x00, sizeof(Qiot_productInfo_t));
        HAL_MEMSET(&QIot_userdata.productInfoCloud, 0x00, sizeof(Qiot_productInfo_t));
        QIot_userdata.connectProduct = &QIot_userdata.productInfo;
        QIot_userdata.productInfo.secret[0] = 0;
        Ql_iotDSKVSave();
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 产品配置-服务器信息获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void Ql_iotConfigGetServer(QIot_protocolType_t *type, char **server_url)
{
    if (type)
    {
        *type = QIOT_PPROTOCOL_MQTT;
    }
    if (server_url)
    {
        if (HAL_STRLEN(QIot_userdata.productInfoCloud.serverUrl))
        {
            *server_url = QIot_userdata.productInfoCloud.serverUrl;
        }
        else
        {
            *server_url = QIot_userdata.productInfo.serverUrl;
        }
    }
}
/**************************************************************************
** 功能	@brief : 产品配置-产品信息配置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigSetProductinfo(const char *pk, const char *ps)
{
    qbool needsave = FALSE;

    if (QIOT_CONNMODE_IDLE != Ql_iotConfigGetConnmode() || pk == NULL || ps == NULL || 0 == HAL_STRLEN(pk) || 0 == HAL_STRLEN(ps))
    {
        return FALSE;
    }

    if (HAL_STRLEN(pk) < sizeof(QIot_userdata.productInfo.productKey) && 0 != HAL_STRCMP(QIot_userdata.productInfo.productKey, pk))
    {
        HAL_SPRINTF(QIot_userdata.productInfo.productKey, "%s", pk);
        needsave = TRUE;
    }
    if (HAL_STRLEN(ps) < sizeof(QIot_userdata.productInfo.productSecret) && 0 != HAL_STRCMP(QIot_userdata.productInfo.productSecret, ps))
    {
        HAL_SPRINTF(QIot_userdata.productInfo.productSecret, "%s", ps);
        needsave = TRUE;
    }
    if (needsave)
    {
        HAL_MEMSET(&QIot_userdata.productInfoCache, 0x00, sizeof(Qiot_productInfo_t));
        HAL_MEMSET(&QIot_userdata.productInfoCloud, 0x00, sizeof(Qiot_productInfo_t));
        QIot_userdata.connectProduct = &QIot_userdata.productInfo;
        QIot_userdata.productInfo.secret[0] = 0;
        Ql_iotDSKVSave();
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 产品配置-产品信息获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotConfigGetProductinfo(char **pk, char **ps, char **ver)
{
    if (pk)
    {
        if (HAL_STRLEN(QIot_userdata.productInfoCloud.productKey))
        {
            *pk = QIot_userdata.productInfoCloud.productKey;
        }
        else
        {
            *pk = QIot_userdata.productInfo.productKey;
        }
    }
    if (ps)
    {
        if (HAL_STRLEN(QIot_userdata.productInfoCloud.productSecret))
        {
            *ps = QIot_userdata.productInfoCloud.productSecret;
        }
        else
        {
            *ps = QIot_userdata.productInfo.productSecret;
        }
    }
    if (ver)
        *ver = Ql_iotSecureVerGet();
}
/**************************************************************************
** 功能	@brief : 产品配置-设备生命周期配置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigSetLifetime(quint32_t lifetime)
{
    if (QIOT_CONNMODE_IDLE != Ql_iotConfigGetConnmode() || lifetime == 0 || lifetime > 65535)
    {
        return FALSE;
    }

    if (QIot_userdata.lifetime != lifetime)
    {
        QIot_userdata.lifetime = lifetime;
#ifdef QUEC_ENABLE_AT
        Ql_iotDSKVSave();
#endif
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 产品配置-设备生命周期获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Ql_iotConfigGetLifetime(void)
{
    return QIot_userdata.lifetime;
}
/**************************************************************************
** 功能	@brief : 产品配置-sessionKey配置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotConfigSetSessionFlag(qbool flag)
{
    if (QIOT_CONNMODE_IDLE != Ql_iotConfigGetConnmode())
    {
        return FALSE;
    }
    if (flag != QIot_userdata.sessionInfo.flag)
    {
        QIot_userdata.sessionInfo.flag = flag;
#ifdef QUEC_ENABLE_AT
        Ql_iotDSKVSave();
#endif
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 产品配置-sessionKey获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotConfigGetSessionFlag(void)
{
    return QIot_userdata.sessionInfo.flag;
}
/**************************************************************************
** 功能	@brief : 增加APP软件版本,在启用quecthing连接之前先配置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigSetAppVersion(const char *appVer)
{
    HAL_FREE(QIot_userdata.softversion);
    if (NULL == appVer)
    {
        QIot_userdata.softversion = HAL_STRDUP(Qhal_softversionGet());
    }
    else
    {
        quint32_t len = HAL_STRLEN(Qhal_softversionGet()) + HAL_STRLEN(appVer);
        QIot_userdata.softversion = HAL_MALLOC(len + 1);
        if (QIot_userdata.softversion)
        {
            HAL_SPRINTF(QIot_userdata.softversion, "%s%s", Qhal_softversionGet(), appVer);
        }
    }
    Quos_logPrintf(QUEC_IOT, LL_INFO, "new soft version:%s", QIot_userdata.softversion);
    return NULL == QIot_userdata.softversion ? FALSE : TRUE;
}
/**************************************************************************
** 功能	@brief : 获取版本号
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotConfigGetSoftVersion(void)
{
    return QIot_userdata.softversion;
}
/**************************************************************************
** 功能	@brief : 设置MCU版本号
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigSetMcuVersion(const char *compno, const char *version)
{
    qbool ret = FALSE;
    if (0 == HAL_STRLEN(compno) || HAL_STRLEN(compno) > QIOT_COMPNO_MAXSIZE || HAL_STRLEN(version) > QIOT_COMPVER_MAXSIZE)
    {
        return ret;
    }
    char *oldVersion;
    qint32_t oldLen = Quos_keyValueExtract(QIot_userdata.mcuVerList, compno, QIOT_MCUVERSION_STRING_SEP, &oldVersion, QIOT_MCUVERSION_STRING_ENDSTR);
    if ((0 >= oldLen && 0 != HAL_STRLEN(version)) || (0 < oldLen && 0 != HAL_STRCMP(oldVersion, version)))
    {
        ret = Quos_keyValueInsert(QIot_userdata.mcuVerList, sizeof(QIot_userdata.mcuVerList), compno, QIOT_MCUVERSION_STRING_SEP, version, QIOT_MCUVERSION_STRING_ENDSTR);
#ifdef QUEC_ENABLE_AT
        Ql_iotDSKVSave();
#endif
    }
    else
    {
        ret = TRUE;
    }

    Ql_iotCmdOtaMcuVersionModify(compno, version);
    return ret;
}
/**************************************************************************
** 功能	@brief : 获取MCU版本号
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Ql_iotConfigGetMcuVersion(const char *compno, char **version)
{
    if (version)
    {
        if (NULL == compno)
        {
            *version = QIot_userdata.mcuVerList;
            return HAL_STRLEN(QIot_userdata.mcuVerList);
        }
        else
        {
            return Quos_keyValueExtract(QIot_userdata.mcuVerList, compno, QIOT_MCUVERSION_STRING_SEP, version, QIOT_MCUVERSION_STRING_ENDSTR);
        }
    }
    else
    {
        return 0;
    }
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void Ql_iotUrcEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen)
{
#ifdef QUEC_ENABLE_AT
    extern void Ql_iotAtEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen);
    Ql_iotAtEventCB(event, errcode, value, valLen);
#else
    if (QIot_userdata.eventCB)
    {
        QIot_userdata.eventCB(event, errcode, value, valLen);
    }
#endif
}
/**************************************************************************
** 功能	@brief : 注册事件回调函数
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
#ifndef QUEC_ENABLE_AT
void FUNCTION_ATTR_ROM Ql_iotConfigSetEventCB(void (*eventCb)(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen))
{
    QIot_userdata.eventCB = eventCb;
}
#endif
#ifdef QUEC_ENABLE_GATEWAY
/**************************************************************************
** 功能	@brief : 注册子设备事件回调函数
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotConfigSetSubDevEventCB(void (*eventCb)(quint32_t event, qint32_t errcode, const char *subPk, const char *subDk ,const void *value, quint32_t valLen))
{
    QIot_userdata.SubDevEventCB = eventCb;
}
#endif
/**************************************************************************
** 功能	@brief : 获取当前工作状态
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
QIot_state_e FUNCTION_ATTR_ROM Ql_iotGetWorkState(void)
{
    return QIot_userdata.workState;
}
/**************************************************************************
** 功能	@brief : 设置DK和DS,
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigSetDkDs(const char *dk, const char *ds)
{
    if (QIOT_CONNMODE_IDLE != Ql_iotConfigGetConnmode())
    {
        return FALSE;
    }
    if (HAL_STRLEN(dk) > QIOT_DK_MAXSIZE || HAL_STRLEN(ds) > QIOT_DS_MAXSIZE)
    {
        Quos_logPrintf(QUEC_IOT, LL_ERR, "dk or ds size over");
        return FALSE;
    }
    qint32_t len = HAL_STRLEN(dk);
    while (len--)
    {
        if (!(__IS_LETTER(dk[len]) || __IS_DIGIT(dk[len])))
            return FALSE;
    }
    len = HAL_STRLEN(ds);
    while (len--)
    {
        if (!(__IS_LETTER(ds[len]) || __IS_DIGIT(ds[len])))
            return FALSE;
    }
    qbool needSave = FALSE;
    if (0 == HAL_STRLEN(dk)) /* 若新设DK为空且旧DK也是通过本API设置时，将清空用户配置采用默认DK重新认证 */
    {
        if (0 != HAL_STRLEN(ds) || 0 == HAL_STRCMP(QIot_userdata.deviceInfo.deviceKey, Qhal_devUuidGet()))
        {
            return FALSE;
        }
        HAL_SPRINTF(QIot_userdata.deviceInfo.deviceKey, "%s", Qhal_devUuidGet());
        HAL_MEMSET(QIot_userdata.productInfo.secret, 0, sizeof(QIot_userdata.productInfo.secret));
        needSave = TRUE;
    }
    else if (0 == HAL_STRCMP(Qhal_devUuidGet(), dk))
    {
        return FALSE;
    }
    else if (0 != HAL_STRCMP(QIot_userdata.deviceInfo.deviceKey, dk))
    {
        HAL_SPRINTF(QIot_userdata.deviceInfo.deviceKey, "%s", dk);
        HAL_SPRINTF(QIot_userdata.productInfo.secret, "%s", ds ? ds : "");
        needSave = TRUE;
    }
    else if ((NULL == ds && HAL_STRLEN(QIot_userdata.productInfo.secret) > 0) || (ds && 0 != HAL_STRCMP(QIot_userdata.productInfo.secret, ds)))
    {
        HAL_SPRINTF(QIot_userdata.productInfo.secret, "%s", ds ? ds : "");
        needSave = TRUE;
    }

    if (needSave)
    {
        Ql_iotDSKVSave();
    }
    return TRUE;
}

/**************************************************************************
** 功能	@brief : 获取dk和ds,只有dk是外部设置的才允许查询
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotConfigGetDkDs(char **dk, char **ds)
{
    if (0 == HAL_STRCMP(QIot_userdata.deviceInfo.deviceKey, Qhal_devUuidGet()))
    {
        return FALSE;
    }
    if (dk)
    {
        *dk = QIot_userdata.deviceInfo.deviceKey;
    }
    if (ds)
    {
        *ds = QIot_userdata.connectProduct->secret;
    }
    return TRUE;
}
