/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 2020-12-25
** 功能   @brief   : dmp OTA,适配DMP2.2.0
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "ql_iotCmdOTA.h"
#include "ql_iotConn.h"
#include "ql_iotDp.h"
#include "ql_iotTtlv.h"
#include "ql_iotConfig.h"
#include "Qhal_driver.h"

Ql_iotCmdOtaInfo_t QIot_otaInfo;
static void *QIot_otaRuntimer = NULL;
static void *QIot_otaCachetimer = NULL;
static pointer_t QIot_otaFileFd = SOCKET_FD_INVALID;
static QIot_otaStatus_e QIot_otaCacheStatus = QIOT_OTA_STATUS_NOPLAN;
static Quos_socketChlInfoNode_t *sockInfo = NULL;
/**************************************************************************
** 功能	@brief : 升级过程事件上报
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM ql_iotCmdOtaStatusReport(QIot_otaStatus_e code)
{
    qbool ret = FALSE;
    void *ttlvHead = NULL;
    Quos_logPrintf(QUEC_OTA, LL_DBG, "event:%s", QIOT_OTA_STATUS_STRING(code));
    Ql_iotTtlvIdAddInt(&ttlvHead, QIOT_DPID_OTA_STATUS, code);
    Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_OTA_MESSAGE, QIOT_OTA_STATUS_STRING(code));
    Ql_iotTtlvIdAddInt(&ttlvHead, QIOT_DPID_OTA_COMPONENT_TYPE, QIot_otaInfo.componentType);
    Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_OTA_COMPONENT_NO, QIot_otaInfo.componentNo);
    if(QIOT_OTA_EXTERN_SHA256 == QIot_otaInfo.extraMess && QIOT_OTA_STATUS_SUBMITOTA == code)
    {
        Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_OTA_DOWN_SIGN,"sha256");
    }
    if (QIOT_OTA_STATUS_UPDATESUCCESS == code || QIOT_OTA_STATUS_UPDATEERROR == code)
    {
        Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_OTA_MODULE_VER, Qhal_softversionGet());
        char *version = NULL;
        if (0 != Ql_iotConfigGetMcuVersion(NULL, &version))
        {
            Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_OTA_MCU_VER, version);
        }
    }
    //ret = Ql_iotDpSendTtlvOta(QIOT_DPCMD_OTA_EVENT, ttlvHead);
    ret = Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M, NULL, 0, 2, QIOT_DPCMD_OTA_EVENT, ttlvHead, NULL);
    Ql_iotTtlvFree(&ttlvHead);
    return ret;
}
/**************************************************************************
** 功能	@brief : OTA请求
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdOtaRequest(quint32_t mode)
{
    qbool ret = FALSE;
    void *ttlvHead = NULL;
    Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_OTA_MODULE_VER, Qhal_softversionGet());
    char *oldVer = NULL;
    if (Ql_iotConfigGetMcuVersion(NULL, &oldVer) != 0)
    {
        Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_OTA_MCU_VER, oldVer);
    }
    //ret = Ql_iotDpSendTtlvOta(QIOT_DPCMD_OTA_REQUEST, ttlvHead);
    ret = Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M, NULL, 0, 2, QIOT_DPCMD_OTA_REQUEST, ttlvHead, NULL);
    Ql_iotTtlvFree(&ttlvHead);
    if(ret && mode >= QIOT_OTA_EXTERN_MAX)
    {
        ret = FALSE;
    }
    QIot_otaInfo.extraMess = mode;
    return ret;
}

/**************************************************************************
** 功能	@brief : 等待URL超时
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotOtaWaitUrlTimeout(void *swtimer)
{
    Quos_logPrintf(QUEC_OTA, LL_ERR, "timeout");
    Quos_swTimerTimeoutSet(swtimer, SWT_SUSPEND);
    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
}
/**************************************************************************
** 功能	@brief : HTTP下载超时
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotOtaWaitDownloadTimeout(void *swtimer)
{
    Quos_logPrintf(QUEC_OTA, LL_ERR, "timeout");
    Quos_swTimerTimeoutSet(swtimer, SWT_SUSPEND);
    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADERROR);
}
/**************************************************************************
** 功能	@brief : 更新超时
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotOtaWaitUpdateTimeout(void *swtimer)
{
	Quos_logPrintf(QUEC_OTA, LL_ERR, "timeout");
    Quos_swTimerTimeoutSet(swtimer, SWT_SUSPEND);
    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
}
/**************************************************************************
** 功能	@brief : 下载中通知
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotOtaDownloading(void *swtimer)
{
    UNUSED(swtimer);
    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADING);
}
#if QUEC_ENABLE_QTH_OTA
/**************************************************************************
** 功能	@brief : 进入OTA更新
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotOtaUpdate(void *swtimer)
{
    quint32_t delayTime = 0;
    delayTime = Qhal_devOtaNotify(QIOT_FILE_OTA, QIot_otaInfo.otaFileInfo.size);
    if (delayTime)
    {
        Quos_swTimerTimeoutSet(swtimer, delayTime);
        Quos_swTimerCBSet(swtimer, ql_iotOtaWaitUpdateTimeout);
    }
    else
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
        Quos_swTimerTimeoutSet(swtimer, SWT_SUSPEND);
    }
}
#endif
/**************************************************************************
** 功能	@brief : OTA文件下载通知
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM ql_iotOtaDownloadCB(qint32_t httpCode, char *retHeader, quint8_t *recvBuf, quint32_t recvLen)
{
    UNUSED(retHeader);
    UNUSED(recvBuf);
    Quos_logPrintf(QUEC_CONN, LL_DBG, "httpCode:%d recvLen:%u", httpCode, recvLen);
    if (200 != httpCode && 206 != httpCode)
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADERROR);
        return FALSE;
    }
    if (0 == recvLen)
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADING);
    }
    else
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADSUCCESS);
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 下载文件
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotOtaDownload(void)
{
    QIot_otaInfo.currentPiece.size = QIot_otaInfo.otaFileInfo.size - QIot_otaInfo.currentPiece.startAddr < QIot_otaInfo.currentPiece.size ? QIot_otaInfo.otaFileInfo.size - QIot_otaInfo.currentPiece.startAddr : QIot_otaInfo.currentPiece.size;
#ifndef QHAL_DEV_OTA_ENABLE
    char rawHeader[100];
    HttpReqData_t reqData;
    HAL_MEMSET(&reqData, 0, sizeof(reqData));
    Quos_logPrintf(QUEC_OTA, LL_DBG, "start:%u len:%u", QIot_otaInfo.currentPiece.startAddr, QIot_otaInfo.currentPiece.size);
    if (0 != QIot_otaInfo.currentPiece.startAddr || 0 != QIot_otaInfo.currentPiece.size)
    {
        reqData.rawHeaders = rawHeader;
        HAL_SPRINTF(rawHeader, "Accept-Ranges: bytes\r\nRange: bytes=%u-%u\r\n", QIot_otaInfo.currentPiece.startAddr, QIot_otaInfo.currentPiece.startAddr + QIot_otaInfo.currentPiece.size - 1);
    }
    if (FALSE == Quos_httpGetDownload((void **)(&sockInfo), QIot_otaInfo.otaFileInfo.downloadUrl, ql_iotOtaDownloadCB, &reqData, QIOT_FILE_OTA, 0))
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADERROR);
    }
#else
    qhal_devOta_e ret = Qhal_devOtaDownload(QIot_otaInfo.otaFileInfo.downloadUrl, QIot_otaInfo.currentPiece.startAddr, QIot_otaInfo.currentPiece.size, QIOT_FILE_OTA, QIot_otaInfo.componentType);
    Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND);
    if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType && (QHAL_OTA_UPDATE_SUCC == ret || QHAL_OTA_UPDATE_FAIL == ret))
    {
        Quos_logPrintf(QUEC_OTA, LL_ERR, "invalid download result");
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
        return;
    }
    switch (ret)
    {
    case QHAL_OTA_DOWN_START:
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADING);
        break;
    case QHAL_OTA_DOWN_SUCC:
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADSUCCESS);
        break;
    case QHAL_OTA_DOWN_FAIL:
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADERROR);
        break;
    case QHAL_OTA_UPDATE_SUCC:
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATESUCCESS);
        break;
    case QHAL_OTA_UPDATE_FAIL:
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
        break;
    default:
        break;
    }
#endif
}

/**************************************************************************
** 功能	@brief : MCU 版本确认
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdOtaMcuVersionModify(const char *compNo, const char *version)
{
    Quos_logPrintf(QUEC_OTA, LL_DBG, "version cur[%s:%s] new[%s:%s] componentType:%d currStatus:%s", compNo, version, QIot_otaInfo.componentNo, QIot_otaInfo.targetVersion, QIot_otaInfo.componentType, QIOT_OTA_STATUS_STRING(QIot_otaInfo.currStatus));
    QIot_otaStatus_e status = QIot_otaInfo.currStatus | QIot_otaCacheStatus;
    switch (status)
    {
    case QIOT_OTA_STATUS_NOPLAN:
    case QIOT_OTA_STATUS_REVICEPLAN:
    case QIOT_OTA_STATUS_REFUSEDOTA:
    case QIOT_OTA_STATUS_UPDATESUCCESS:
    case QIOT_OTA_STATUS_UPDATEERROR:
        break;
    case QIOT_OTA_STATUS_SUBMITOTA:
    case QIOT_OTA_STATUS_DOWNLOADSTART:
    case QIOT_OTA_STATUS_DOWNLOADING:
    case QIOT_OTA_STATUS_DOWNLOADERROR:
    case QIOT_OTA_STATUS_DOWNLOADSUCCESS:
    case QIOT_OTA_STATUS_UPDATING:
        if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType && 0 == HAL_STRCMP(compNo, QIot_otaInfo.componentNo))
        {
            if (status < QIOT_OTA_STATUS_DOWNLOADSUCCESS && status >= QIOT_OTA_STATUS_DOWNLOADSTART && NULL != sockInfo && Quos_socketCheckChlFd(sockInfo) && TRUE == sockInfo->valid)
            {
                Qhal_sockClose(sockInfo->sockFd, sockInfo->type);
            }
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND);
            status = (0 == HAL_STRCMP(version, QIot_otaInfo.targetVersion)) ? QIOT_OTA_STATUS_UPDATESUCCESS : QIOT_OTA_STATUS_UPDATEERROR;
            Quos_logPrintf(QUEC_OTA, LL_DBG, "status:%d", status);
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)status);
        }
    default:
        break;
    }
}
/**************************************************************************
** 功能	@brief : MCU数据读取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Ql_iotCmdOtaMcuFWDataRead(quint32_t startAddr, quint8_t data[], quint32_t maxLen)
{
    if (QIOT_OTA_STATUS_DOWNLOADSUCCESS != QIot_otaInfo.currStatus ||
        startAddr < QIot_otaInfo.currentPiece.startAddr || startAddr >= QIot_otaInfo.currentPiece.startAddr + QIot_otaInfo.currentPiece.size ||
        0 == maxLen)
    {
        Quos_logPrintf(QUEC_OTA, LL_ERR, "currStatus:%s startAddr:%d/%d size:%d", QIOT_OTA_STATUS_STRING(QIot_otaInfo.currStatus), startAddr, QIot_otaInfo.currentPiece.startAddr, QIot_otaInfo.currentPiece.size);
        return 0;
    }
    Quos_swTimerTimeoutSet(QIot_otaRuntimer, QIOT_OTA_WAIT_READ);
    if (QIot_otaFileFd == SOCKET_FD_INVALID)
    {
        QIot_otaFileFd = Qhal_fileOpen(QIOT_FILE_OTA, TRUE);
    }
    if (SOCKET_FD_INVALID != QIot_otaFileFd)
    {
        quint32_t offset = startAddr - QIot_otaInfo.currentPiece.startAddr;
        maxLen = QIot_otaInfo.currentPiece.size - offset < maxLen ? QIot_otaInfo.currentPiece.size - offset : maxLen;
        Quos_logPrintf(QUEC_OTA, LL_DBG, "offset:%d size:%d maxLen:%d", offset, QIot_otaInfo.currentPiece.size, maxLen);
        maxLen = Qhal_fileRead(QIot_otaFileFd, offset, data, maxLen);
        if (0 == maxLen)
        {
            Quos_logPrintf(QUEC_OTA, LL_ERR, "file read len=%d", maxLen);
        }

        return maxLen;
    }
    else
    {
        Quos_logPrintf(QUEC_OTA, LL_ERR, "open file fail fileFd:" PRINTF_LD, QIot_otaFileFd);
    }
    return 0;
}
/**************************************************************************
** 功能	@brief : 升级确认
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdOtaAction(quint8_t action)
{
    qbool ret = FALSE;
    Quos_logPrintf(QUEC_OTA, LL_DBG, "action:%u currStatus:%s", action, QIOT_OTA_STATUS_STRING(QIot_otaInfo.currStatus));
    switch (action)
    {
    case 0:
        /* 模组把状态保存起来 */
        if (QIOT_OTA_STATUS_REVICEPLAN == QIot_otaInfo.currStatus)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_REFUSEDOTA);
            ret = TRUE;
            if (TRUE == QIot_otaInfo.mutilPlansMode)
            {
                quint32_t flag = 0;
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_UPDATE_FLAG, &flag, sizeof(flag));
            }
        }
        break;
    case 1:
        if (QIOT_OTA_STATUS_REVICEPLAN == QIot_otaInfo.currStatus)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_SUBMITOTA);
            ret = TRUE;
            if (TRUE == QIot_otaInfo.mutilPlansMode)
            {
                quint32_t flag = 0;
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_UPDATE_FLAG, &flag, sizeof(flag));
            }
        }
        else if (QIOT_OTA_STATUS_SUBMITOTA == QIot_otaInfo.currStatus)
        {
            ret = TRUE;
        }
        break;
    case 2:
        if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType && QIot_otaInfo.otaFileInfo.size > 0 && QIOT_OTA_STATUS_DOWNLOADSUCCESS == QIot_otaInfo.currStatus)
        {
            if (QIot_otaInfo.currentPiece.startAddr + QIot_otaInfo.currentPiece.size < QIot_otaInfo.otaFileInfo.size)
            {
                QIot_otaInfo.currentPiece.startAddr += QIot_otaInfo.currentPiece.size;
                Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADSTART);
                ret = TRUE;
            }
        }
        break;
    case 3:
        if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType)
        {
            if (QIOT_OTA_STATUS_DOWNLOADSUCCESS == QIot_otaInfo.currStatus)
            {
                if (QIot_otaInfo.currentPiece.startAddr + QIot_otaInfo.currentPiece.size >= QIot_otaInfo.otaFileInfo.size)
                {
                    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATING);
                    ret = TRUE;
                }
            }
            else if (QIOT_OTA_STATUS_UPDATING == QIot_otaInfo.currStatus)
            {
                ret = TRUE;
            }
        }
        break;
    default:
        break;
    }
    return ret;
}
/**************************************************************************
** 功能	@brief : OTA 组件处理
** 输入	@param : ttlvHead：ttlv链表头   mode：FALSE：首次或单个  TRUE： 多个组件
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM ql_iotOtaPlanHandle(void *ttlvHead)
{
    qint64_t componentType = 0;
    char *componentNo = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_COMPONENT_NO);
    char *sourceVersion = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_SOURCE_VER);
    char *targetVersion = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_TARGET_VER);
    if (NULL == componentNo || NULL == targetVersion || QIOT_COMPVER_MAXSIZE < HAL_STRLEN(targetVersion) || FALSE == Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_COMPONENT_TYPE, &componentType)) // || FALSE == Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_COMPONENT_TYPE, &componentType)
    {
        Quos_logPrintf(QUEC_OTA, LL_DBG, "info invalid");
        return FALSE;
    }
    char *tmpBuf = HAL_MALLOC(HAL_STRLEN(componentNo) + HAL_STRLEN(sourceVersion) + HAL_STRLEN(targetVersion) + 4 * 11 + 11 + 1);
    if (tmpBuf)
    {
        qint64_t batteryLimit, minSignalIntensity, useSpace;
        int64_t planMode;
        HAL_SPRINTF(tmpBuf, "\"%s\",\"%s\",\"%s\",%u,%d,%u", (char *)componentNo,
                    sourceVersion ? sourceVersion : (char *)"",
                    (char *)targetVersion,
                    Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_BATTERY_LIMIT, &batteryLimit) ? (quint32_t)batteryLimit : 0,
                    Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_MIN_SIGNAL, &minSignalIntensity) ? (qint32_t)minSignalIntensity : 0,
                    Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_USE_SPACE, &useSpace) ? (quint32_t)useSpace : 0);

        Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_TASK_NOTIFY, tmpBuf, HAL_STRLEN(tmpBuf));
        HAL_SNPRINTF(QIot_otaInfo.componentNo, sizeof(QIot_otaInfo.componentNo), "%s", componentNo);
        QIot_otaInfo.componentType = componentType;
        if (TRUE == Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_HANDLE_TYPE, &planMode) && QIOT_COMPTYPE_MODULE == componentType)
        {
            Ql_iotCmdOtaAction(1);
        }
        HAL_FREE(tmpBuf);
    }
    return TRUE;
}

/**************************************************************************
** 功能	@brief : OTA任务下发
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdOtaNotify(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(app);
    UNUSED(endPoint);
    UNUSED(pkgId);
    Quos_logPrintf(QUEC_OTA, LL_DBG, "recv task");
    if (QIOT_OTA_STATUS_NOPLAN != QIot_otaInfo.currStatus && QIOT_OTA_STATUS_REVICEPLAN != QIot_otaInfo.currStatus)
    {
        Quos_logPrintf(QUEC_OTA, LL_ERR, "current is %s and no restart", QIOT_OTA_STATUS_STRING(QIot_otaInfo.currStatus));
        return;
    }
    void *ttlvHead = Ql_iotTtlvUnformat(payload, payloadLen);
    void *mutilPlans = Ql_iotTtlvIdGetStruct(ttlvHead, QIOT_DPID_OTA_TASK_INFO);
    int count = 0;
    if (mutilPlans != NULL)
    {
        count = Ql_iotTtlvCountGet(mutilPlans);
        if (count > 20)
        {
            Ql_iotTtlvFree(&ttlvHead);
            return;
        }
        int count_i = 0;
        QIot_otaInfo.mutilPlansMode = TRUE;
        for (count_i = 0; count_i < count; count_i++)
        {
            void *planStruct = Ql_iotTtlvNodeGet(mutilPlans, count_i, NULL, NULL);
            void *childNode = Ql_iotTtlvNodeGetStruct(planStruct);
            if (FALSE == ql_iotOtaPlanHandle(childNode))
                continue;
        }
    }
    else
    {
        QIot_otaInfo.mutilPlansMode = FALSE;
        ql_iotOtaPlanHandle(ttlvHead);
    }
    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_REVICEPLAN);
    Ql_iotTtlvFree(&ttlvHead);
    return;
}
#if QUEC_ENABLE_QTH_OTA==0    
/**************************************************************************
** 功能	@brief : 多固件下发
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool ql_iotCmdOtaFwList(quint8_t *payload, quint32_t len, qbool *isMutilFileInfo)
{
    qint64_t componentType;
    void *ttlvHead = Ql_iotTtlvUnformat(payload, len);

    char *componentNo = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_COMPONENT_NO);
    char *targetVersion = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_TARGET_VER);
    if (NULL == componentNo || NULL == targetVersion || FALSE == Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_COMPONENT_TYPE, &componentType))
    {
        goto exit;
    }
    char *filemd5 = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_MD5);
    char *filecrc = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_CRC);
    char *filesha256 = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_SHA256);
    char *download_url = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_URL);
    char *infoArray = Ql_iotTtlvIdGetStruct(ttlvHead, QIOT_DPID_OTA_DOWN_INFO);
    if (infoArray != NULL)
    {
        if (NULL == filemd5 && NULL == filecrc && NULL == filesha256 && NULL == download_url && QIOT_COMPTYPE_MODULE == componentType)
        {
            quint32_t count = Ql_iotTtlvCountGet(infoArray);
            if (count > 5)
            {
                goto exit;
            }
            *isMutilFileInfo = TRUE;
            QIot_otaFilePublicInfo_t otaInfoArray[QIOT_OTA_FILEINFO_MAX_SIZE];
            HAL_MEMSET(otaInfoArray, 0, sizeof(QIot_otaFilePublicInfo_t) * QIOT_OTA_FILEINFO_MAX_SIZE);
            quint32_t array_i = 0;
            for (array_i = 0; array_i < count; array_i++)
            {
                char *info = Ql_iotTtlvNodeGet(infoArray, array_i, NULL, NULL);
                void *childNode = Ql_iotTtlvNodeGetStruct(info);
                if (info != NULL)
                {
                    int64_t idex = 0;
                    char *info_url = Ql_iotTtlvIdGetString(childNode, QIOT_DPID_OTA_DOWN_URL);
                    char *info_filemd5 = Ql_iotTtlvIdGetString(childNode, QIOT_DPID_OTA_DOWN_MD5);
                    qint64_t info_filesize;
                    if (NULL == info_url || NULL == info_filemd5 ||
                        FALSE == Ql_iotTtlvIdGetInt(childNode, QIOT_DPID_OTA_DOWN_INFO_IDEX, (int64_t *)&idex) ||
                        FALSE == Ql_iotTtlvIdGetInt(childNode, QIOT_DPID_OTA_DOWN_SIZE, &info_filesize))
                    {
                        Quos_logPrintf(QUEC_OTA, LL_DBG, "info invalid");
                        goto exit;
                    }
                    otaInfoArray[array_i].idex = idex & 0xFF;
                    otaInfoArray[array_i].downloadUrl = HAL_MALLOC(HAL_STRLEN(info_url) + 1);
                    HAL_SNPRINTF(otaInfoArray[array_i].downloadUrl, HAL_STRLEN(info_url) + 1, "%s",info_url);
                    HAL_SNPRINTF(otaInfoArray[array_i].md5, sizeof(otaInfoArray[array_i].md5), "%s", info_filemd5);
                    otaInfoArray[array_i].size = info_filesize;
                }
                else
                {
                    goto exit;
                }
            }
            quint32_t timeout = Qhal_devOtaNotify(otaInfoArray, array_i);
            for (array_i = 0; array_i < count; array_i++)
                HAL_FREE(otaInfoArray[array_i].downloadUrl);
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, timeout);
            Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaWaitUpdateTimeout);
        }
    }
    Ql_iotTtlvFree(&ttlvHead);
    return TRUE;
exit:
    Ql_iotTtlvFree(&ttlvHead);
    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
    return FALSE;
}
#endif
/**************************************************************************
** 功能	@brief : OTA URL下发
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdOtaFwInfo(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(app);
    UNUSED(endPoint);
    UNUSED(pkgId);
    Quos_logPrintf(QUEC_OTA, LL_DBG, "recv url");
    if (QIOT_OTA_STATUS_NOPLAN != QIot_otaInfo.currStatus && QIOT_OTA_STATUS_SUBMITOTA != QIot_otaInfo.currStatus)
    {
        Quos_logPrintf(QUEC_OTA, LL_ERR, "current is %s and no repeat download", QIOT_OTA_STATUS_STRING(QIot_otaInfo.currStatus));
        return;
    }
    Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND); /* 收到URL，关闭等待URL超时定时器 */
    HAL_FREE(QIot_otaInfo.otaFileInfo.downloadUrl);
    QIot_otaInfo.otaFileInfo.downloadUrl = NULL;
    #if QUEC_ENABLE_QTH_OTA==0
        qbool isMutilFileInfo = FALSE;
        ql_iotCmdOtaFwList(payload, payloadLen, &isMutilFileInfo);
        if (isMutilFileInfo == TRUE)
        {
            return;
        }
    #endif
    void *ttlvHead = Ql_iotTtlvUnformat(payload, payloadLen);
    qint64_t filesize, componentType;
    char *filemd5 = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_MD5);
    char *filecrc = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_CRC);
    char *filesha256 = NULL;
    if (QIOT_OTA_EXTERN_SHA256 == QIot_otaInfo.extraMess)
    {
        filesha256 = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_SHA256);
    }
    char *componentNo = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_COMPONENT_NO);
    char *download_url = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_DOWN_URL);
    char *targetVersion = Ql_iotTtlvIdGetString(ttlvHead, QIOT_DPID_OTA_TARGET_VER);
    if (NULL == filemd5 || NULL == filecrc || NULL == componentNo || NULL == download_url || NULL == targetVersion ||
        (QIOT_OTA_EXTERN_SHA256 == QIot_otaInfo.extraMess && NULL == filesha256) ||
        FALSE == Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_DOWN_SIZE, &filesize) ||
        FALSE == Ql_iotTtlvIdGetInt(ttlvHead, QIOT_DPID_OTA_COMPONENT_TYPE, &componentType) ||
        QIOT_MD5_MAXSIZE != HAL_STRLEN(filemd5) ||
        QIOT_COMPNO_MAXSIZE < HAL_STRLEN(componentNo) ||
        QIOT_COMPVER_MAXSIZE < HAL_STRLEN(targetVersion) ||
        (QIot_otaInfo.otaFileInfo.downloadUrl = HAL_STRDUP((char *)download_url)) == NULL)
    {
        Quos_logPrintf(QUEC_OTA, LL_DBG, "info invalid");
        Ql_iotTtlvFree(&ttlvHead);
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
        return;
    }
    HAL_SPRINTF(QIot_otaInfo.componentNo, "%s", componentNo);
    HAL_SPRINTF(QIot_otaInfo.targetVersion, "%s", targetVersion);
    QIot_otaInfo.componentType = (QIot_otaComptype_e)componentType;
    QIot_otaInfo.otaFileInfo.size = filesize;
    HAL_SPRINTF(QIot_otaInfo.otaFileInfo.md5, "%s", filemd5);
    QIot_otaInfo.crc = HAL_STRTOUL(filecrc, NULL, 16);
    if (filesha256 != NULL)
    {
        HAL_SPRINTF(QIot_otaInfo.sha256, "%s", filesha256);
    }
    Ql_iotTtlvFree(&ttlvHead);
    Qhal_propertyDev_t dInfo;
    HAL_MEMSET(&dInfo, 0, sizeof(Qhal_propertyDev_t));
    Qhal_propertyDevGet(&dInfo);
    if (QIOT_COMPTYPE_MODULE == QIot_otaInfo.componentType && dInfo.flashFree < QIot_otaInfo.otaFileInfo.size)
    {
        Quos_logPrintf(QUEC_OTA, LL_ERR, "no enough space");
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
        return;
    }

    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADSTART);
    QIot_otaInfo.currentPiece.startAddr = 0;
    QIot_otaInfo.currentPiece.size = dInfo.flashFree > QIot_otaInfo.otaFileInfo.size ? QIot_otaInfo.otaFileInfo.size : dInfo.flashFree;
}
/**************************************************************************
** 功能	@brief : 缓存状态上报
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotCmdOtaCacheReport(void *swtimer)
{
    UNUSED(swtimer);
    Quos_logPrintf(QUEC_OTA, LL_DBG, "cache status report:%s.", QIOT_OTA_STATUS_STRING(QIot_otaCacheStatus));
    if (QIOT_OTA_STATUS_NOPLAN != QIot_otaCacheStatus)
    {
        if (TRUE == ql_iotCmdOtaStatusReport(QIot_otaCacheStatus))
        {
            QIot_otaCacheStatus = QIOT_OTA_STATUS_NOPLAN;
        }
    }
}

/**************************************************************************
** 功能	@brief : OTA事件处理
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotCmdOtaEventNotify(qint32_t event, void *arg)
{
    static quint8_t downLoadFailCount = 0;
    static qbool downloadStartFlag = TRUE;
    switch (event)
    {
    case QIOT_ATEVENT_TYPE_OTA:
    {
        QIot_otaStatus_e status = (QIot_otaStatus_e)(pointer_t)arg;
        Quos_logPrintf(QUEC_OTA, LL_DBG, "event:%s status:[%d]%s", QIOT_ATEVENT_TYPE_STRING(event), status, QIOT_OTA_STATUS_STRING(status));
        QIot_otaInfo.currStatus = status;
        Ql_iotDSKVSave();
        switch (status)
        {
        case QIOT_OTA_STATUS_NOPLAN:
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND);
            HAL_FREE(QIot_otaInfo.otaFileInfo.downloadUrl);
            QIot_otaInfo.otaFileInfo.downloadUrl = NULL;
            QIot_otaInfo.otaFileInfo.size = 0;
            QIot_otaInfo.currentPiece.startAddr = 0;
            QIot_otaInfo.currentPiece.size = 0;
            Ql_iotDSKVSave();
            if (QIot_otaFileFd != SOCKET_FD_INVALID)
            {
                Qhal_fileClose(QIot_otaFileFd);
                QIot_otaFileFd = SOCKET_FD_INVALID;
            }
            Qhal_fileErase(QIOT_FILE_OTA);
            break;
        case QIOT_OTA_STATUS_REVICEPLAN:
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND);
            break;
        case QIOT_OTA_STATUS_SUBMITOTA:
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, QIOT_OTA_WAIT_URL_TIMEOUT);
            Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaWaitUrlTimeout);
            break;
        case QIOT_OTA_STATUS_REFUSEDOTA:
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_NOPLAN);
            break;
        case QIOT_OTA_STATUS_DOWNLOADSTART:
            if (downloadStartFlag)
            {
                if (QIOT_COMPTYPE_MODULE == QIot_otaInfo.componentType)
                {
                    Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_START, NULL, 0);
                }
                else if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType)
                {
                    char tmpBuf[QIOT_COMPNO_MAXSIZE + 10 + QIOT_MD5_MAXSIZE + 10 + 7 + QUOS_SHA256_DIGEST_LENGTH * 2 + 1];
                    HAL_SPRINTF(tmpBuf, "\"%.*s\",%u,\"%s\",%u", QIOT_COMPNO_MAXSIZE, QIot_otaInfo.componentNo, QIot_otaInfo.otaFileInfo.size, QIot_otaInfo.otaFileInfo.md5, QIot_otaInfo.crc);
                    if (QIOT_OTA_EXTERN_SHA256 == QIot_otaInfo.extraMess)
                    {
                        HAL_SPRINTF(tmpBuf + HAL_STRLEN(tmpBuf), ",\"%s\"", QIot_otaInfo.sha256);
                    }
                    Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_START, tmpBuf, HAL_STRLEN(tmpBuf));
                }
                downloadStartFlag = FALSE;
            }
            else
            {
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_DOWNLOADING, NULL, 0);
            }
            #if QUEC_ENABLE_QTH_OTA == 0
                if (QIOT_COMPTYPE_MODULE == QIot_otaInfo.componentType)
                {
                    quint32_t timeout = Qhal_devOtaNotify(&(QIot_otaInfo.otaFileInfo), 1);
                    Quos_swTimerTimeoutSet(QIot_otaRuntimer, timeout);
                    Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaWaitUpdateTimeout);
                }
                else
                {
            #endif
                    Quos_swTimerTimeoutSet(QIot_otaRuntimer, QIOT_OTA_DOWNLOADING_NOTIFY);
                    Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaWaitDownloadTimeout);
                    ql_iotOtaDownload();
            #if QUEC_ENABLE_QTH_OTA == 0
                }
            #endif
            break;
        case QIOT_OTA_STATUS_DOWNLOADING:
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, QIOT_OTA_DOWNLOADING_NOTIFY);
            Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaDownloading);
            Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_DOWNLOADING, NULL, 0);
            break;
        case QIOT_OTA_STATUS_DOWNLOADERROR:
            /* 连续下载QIOT_OTA_DOWNLOAD_FAIL_COUNT_MAX次失败则上报更新失败 */
            downLoadFailCount++;
            Quos_logPrintf(QUEC_OTA, LL_ERR, "downLoadFailCount:%d", downLoadFailCount);
            if (downLoadFailCount > QIOT_OTA_DOWNLOAD_FAIL_COUNT_MAX)
            {
                Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATEERROR);
                return;
            }
            else
            {
                Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_NOPLAN);
            }
            break;
        case QIOT_OTA_STATUS_DOWNLOADSUCCESS:
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND);
            if (QIOT_COMPTYPE_MODULE == QIot_otaInfo.componentType)
            {
                char md5String[33];
                if (FALSE == Quos_fileMd5(QIOT_FILE_OTA, QIot_otaInfo.otaFileInfo.size, md5String) || 0 != HAL_STRCMP(QIot_otaInfo.otaFileInfo.md5, md5String))
                {
                    Quos_logPrintf(QUEC_OTA, LL_ERR, "md5 calc:%s ser:%s", md5String, QIot_otaInfo.otaFileInfo.md5);
                    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADERROR);
                }
                else
                {
                    Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_DOWNLOADED, NULL, 0);
                    Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATING);
                }
            }
            else if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType && QIot_otaInfo.otaFileInfo.size > 0)
            {
                Quos_swTimerTimeoutSet(QIot_otaRuntimer, QIOT_OTA_WAIT_READ);
                Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaWaitUpdateTimeout);
                char tmpBuf[QIOT_COMPNO_MAXSIZE + 3 * 10 + 5 + 1];
                HAL_SPRINTF(tmpBuf, "\"%.*s\",%u,%u,%u", QIOT_COMPNO_MAXSIZE, QIot_otaInfo.componentNo, QIot_otaInfo.otaFileInfo.size, QIot_otaInfo.currentPiece.startAddr, QIot_otaInfo.currentPiece.size);
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_DOWNLOADED, tmpBuf, HAL_STRLEN(tmpBuf));
            }
            break;
        case QIOT_OTA_STATUS_UPDATING:
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND);
            if (QIOT_COMPTYPE_MODULE == QIot_otaInfo.componentType)
            {
                #if QUEC_ENABLE_QTH_OTA
                    Quos_swTimerTimeoutSet(QIot_otaRuntimer, QIOT_OTA_UPDATE_DELAY);
                    Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaUpdate);
                #endif
            }
            else if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType)
            {
                Quos_swTimerTimeoutSet(QIot_otaRuntimer, QIOT_OTA_UPDATE_TIMEOUT);
                Quos_swTimerCBSet(QIot_otaRuntimer, ql_iotOtaWaitUpdateTimeout);
            }
            Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, QIOT_OTA_UPDATING, NULL, 0);
            break;
        case QIOT_OTA_STATUS_UPDATESUCCESS:
        case QIOT_OTA_STATUS_UPDATEERROR:
            Quos_swTimerTimeoutSet(QIot_otaRuntimer, SWT_SUSPEND);
            downLoadFailCount = 0;
            downloadStartFlag = TRUE;
            Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_OTA, status == QIOT_OTA_STATUS_UPDATESUCCESS ? QIOT_OTA_UPDATE_OK : QIOT_OTA_UPDATE_FAIL, NULL, 0);
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_NOPLAN);
            break;
        default:
            return;
        }
        if (QIOT_OTA_STATUS_NOPLAN != status)
        {
            if (FALSE == ql_iotCmdOtaStatusReport(status))
            {
                Quos_logPrintf(QUEC_OTA, LL_DBG, "report status fail,cache status:%s.", QIOT_OTA_STATUS_STRING(status));
                QIot_otaCacheStatus = status;
            }
            else
            {
                Quos_swTimerDelete(QIot_otaCachetimer);
                QIot_otaCacheStatus = QIOT_OTA_STATUS_NOPLAN;
            }
        }
        break;
    }
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
    {
        switch ((qint32_t)(pointer_t)arg)
        {
        case QIOT_SUBCRIBE_SUCC:
        {
            Quos_logPrintf(QUEC_OTA, LL_DBG, "cache status:%s.", QIOT_OTA_STATUS_STRING(QIot_otaCacheStatus));
            if (QIOT_OTA_STATUS_NOPLAN != QIot_otaCacheStatus)
            {
                Quos_swTimerStart(&QIot_otaCachetimer, "Cache", 0, 1, ql_iotCmdOtaCacheReport, NULL);
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

/**************************************************************************
** 功能	@brief : OTA状态恢复
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdOtaStatusRecovery(void)
{
    Quos_logPrintf(QUEC_OTA, LL_DBG, "ota status:%s.", QIOT_OTA_STATUS_STRING(QIot_otaInfo.currStatus));
#ifdef QHAL_DEV_OTA_ENABLE
    if (QIOT_OTA_STATUS_DOWNLOADSTART == QIot_otaInfo.currStatus && QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType)
    {
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, 0 == HAL_STRCMP(QIot_otaInfo.targetVersion, QIot_userdata.softversion) ? (void *)QIOT_OTA_STATUS_UPDATESUCCESS : (void *)QIOT_OTA_STATUS_UPDATEERROR);
        return;
    }
#endif
    switch (QIot_otaInfo.currStatus)
    {
    case QIOT_OTA_STATUS_NOPLAN:
        break;
    case QIOT_OTA_STATUS_SUBMITOTA:
    case QIOT_OTA_STATUS_DOWNLOADSTART:
    case QIOT_OTA_STATUS_DOWNLOADING:
    case QIOT_OTA_STATUS_DOWNLOADSUCCESS:
        if (QIOT_COMPTYPE_MODULE == QIot_otaInfo.componentType && 0 == HAL_STRCMP(QIot_otaInfo.targetVersion, QIot_userdata.softversion))
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATESUCCESS);
        }
        else
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADERROR);
        }
        break;
    case QIOT_OTA_STATUS_DOWNLOADERROR:
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_DOWNLOADERROR);
        break;
    case QIOT_OTA_STATUS_UPDATING:
        if (QIOT_COMPTYPE_MODULE == QIot_otaInfo.componentType)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, 0 == HAL_STRCMP(QIot_otaInfo.targetVersion, QIot_userdata.softversion) ? (void *)QIOT_OTA_STATUS_UPDATESUCCESS : (void *)QIOT_OTA_STATUS_UPDATEERROR);
        }
        else if (QIOT_COMPTYPE_MCU == QIot_otaInfo.componentType)
        {
            Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_UPDATING);
        }
        break;
    default:
        Quos_eventPost(QIOT_ATEVENT_TYPE_OTA, (void *)QIOT_OTA_STATUS_NOPLAN);
        break;
    }
}

/**************************************************************************
** 功能	@brief : OTA任务初始化
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdOtaInit(void)
{
    HAL_MEMSET(&QIot_otaInfo, 0, sizeof(QIot_otaInfo));
    qint32_t event[] = {QIOT_ATEVENT_TYPE_OTA, QIOT_ATEVENT_TYPE_SUBCRIBE, QIOT_ATEVENT_TYPE_LOGOUT};
    Quos_eventCbReg(event, sizeof(event) / sizeof(event[0]), ql_iotCmdOtaEventNotify);
    Quos_swTimerStart(&QIot_otaRuntimer, "OTA", SWT_SUSPEND, 0, NULL, NULL);
    Qhal_fileErase(QIOT_FILE_OTA);
}
