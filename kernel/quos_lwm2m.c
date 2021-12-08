/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : lwm2m bootstrap
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_lwm2m.h"
#if (SDK_ENABLE_LWM2M == 1)
#include "internals.h"
#include "Qhal_driver.h"
extern char *get_server_uri(lwm2m_object_t *objectP, quint16_t secObjInstID);

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_lwm2mStepTimeCB(void *swTimer)
{
    lwm2m_context_t *ctx = (lwm2m_context_t *)swTimer->parm;
    lwm2mUsedata_t *userdata = (lwm2mUsedata_t *)ctx->userData;
    time_t stepPeriod = userdata->stepPeriod;
    int stepRet = lwm2m_step(ctx, &stepPeriod);
    Quos_logPrintf(LSDK_LWM2M, LL_DBG, "lwm2m step period:" PRINTF_FD " stepRet:0x%02X state:%s", stepPeriod, stepRet, STR_STATE(ctx->state));
    if (0 != stepRet && ctx->state == STATE_BOOTSTRAPPING)
    {
        /* 需要在此重新初始化资源 */
        ctx->state = STATE_INITIAL;
        stepPeriod = 10;
    }
    Quos_swTimerTimeoutSet(swTimer, stepPeriod * SWT_ONE_SECOND);
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM *quos_lwm2mInit(char *name, lwm2m_object_t *objects[], quint16_t count, lwm2mUsedata_t *userdata)
{
    if (NULL == userdata)
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "userdata param invalid");
        return NULL;
    }
    lwm2m_context_t *ctx = lwm2m_init(userdata);
    if (NULL == ctx)
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "init fail");
        HAL_FREE(userdata);
        return NULL;
    }
    if (FALSE == Quos_swTimerStart(&ctx->stepTimer, "lwm2m step", 1, 0, quos_lwm2mStepTimeCB, ctx))
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "step timer start fail");
        HAL_FREE(userdata);
        HAL_FREE(ctx);
        return NULL;
    }
    if (COAP_NO_ERROR != lwm2m_configure(ctx, name, NULL, NULL, count, objects))
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "config fail");
        Quos_swTimerDelete(ctx->stepTimer);
        HAL_FREE(userdata);
        HAL_FREE(ctx);
        return NULL;
    }
    Quos_logPrintf(LSDK_LWM2M, LL_DBG, "lwm2m init ok");
    return (void *)ctx;
}

/**************************************************************************
** 功能	@brief : coap接收数据处理
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_lwm2mSocketRecv(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData)
{
	UNUSED(peer);
	UNUSED(peerSize);
    Quos_logPrintf(LSDK_LWM2M, LL_DBG, "lwm2m recv chlFd[%p]", chlFd);
    
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    lwm2m_context_t *ctx = (lwm2m_context_t *)chlNode->param;
    if (NULL == recvData)
    {
        ctx->state = STATE_INITIAL;
        Quos_swTimerTimeoutSet(ctx->stepTimer, 0);
        return FALSE;
    }
    Quos_logHexDump(LSDK_LWM2M, LL_DUMP, "lwm2m recv", recvData->Buf, recvData->bufLen);
    lwm2m_handle_packet(ctx, recvData->Buf, recvData->bufLen, chlFd);
    Quos_swTimerTimeoutSet(ctx->stepTimer, 0);
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM *lwm2m_connect_server(void **fd, quint16_t secObjInstID, void *context)
{
    lwm2m_context_t *ctx = (lwm2m_context_t *)context;
    lwm2m_object_t *targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(ctx->objectList, LWM2M_SECURITY_OBJECT_ID);
    if (NULL == targetP)
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "not found security obj");
        return NULL;
    }
    char *url = get_server_uri(targetP, secObjInstID);
    if (NULL == url)
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "server url is empty");
        return NULL;
    }
    Quos_logPrintf(LSDK_LWM2M, LL_DBG, "url:%s", url);
    urlAnalyze_t urlA;
    if (FALSE == Quos_urlAnalyze(url, &urlA))
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "url[%s] analyze fail", url);
        return NULL;
    }
    urlA.port = urlA.port ? urlA.port : (urlA.isSecure ? 5684 : 5683);
    Quos_socketChlInfoNode_t chlInfo;
    HAL_MEMSET(&chlInfo, 0, sizeof(Quos_socketChlInfoNode_t));
    if (urlA.isSecure)
    {
#if (SDK_ENABLE_TLS == 1)
        chlInfo.sockFd = Qhal_udpSslInit(&chlInfo.type, 0, urlA.hostname, urlA.port);
#else
        chlInfo.sockFd = SOCKET_FD_INVALID;
#endif
    }
    else
    {
        chlInfo.sockFd = Qhal_udpInit(&chlInfo.type, 0, urlA.hostname, urlA.port, NULL);
    }
    if (SOCKET_FD_INVALID == chlInfo.sockFd)
    {
        Quos_logPrintf(LSDK_LWM2M, LL_ERR, "coap conn fail:%s[%u]", urlA.hostname, urlA.port);
        return FALSE;
    }

    chlInfo.io.send = Qhal_sockWrite;
    chlInfo.send.txCnt = COAP_MAX_RETRANSMIT;
    chlInfo.send.timeout = COAP_RESPONSE_TIMEOUT * SWT_ONE_SECOND;
    chlInfo.recvDataFunc = quos_lwm2mSocketRecv;
    chlInfo.io.close = Qhal_sockClose;
    chlInfo.param = context;
    void *chlFd = Quos_socketChannelAdd(fd, chlInfo);
    Quos_logPrintf(LSDK_LWM2M, LL_INFO, "lwm2m chlFd[%p]", chlFd);
    if(NULL == chlFd)
    {
        Qhal_sockClose(chlInfo.sockFd,chlInfo.type);
    }
    return chlFd;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM lwm2m_close_connection(void *sessionH, void *context)
{
    Quos_logPrintf(LSDK_LWM2M, LL_INFO, "lwm2m close sessionH[%p] context[%p]", sessionH, context);
    Quos_socketChannelDel(sessionH);
}

/**************************************************************************
** 功能	@brief : bootstrap payload
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint16_t FUNCTION_ATTR_ROM get_bootstrap_payload(lwm2m_context_t *context, quint8_t **valP)
{
    lwm2mUsedata_t *userdata = (lwm2mUsedata_t *)context->userData;
    *valP = userdata->bs.payload.val;
    return userdata->bs.payload.len;
}

#endif