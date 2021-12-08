/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 通信socket管理
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_socket.h"
#include "Quos_kernel.h"
static TWLLHead_T *chlInfoListHead = NULL;
HAL_LOCK_DEF(static, lockId)
#ifndef SOCKET_SENDDATA_BYTESIZE_MAX
#define SOCKET_SENDDATA_BYTESIZE_MAX (50 * 1024)
#endif
static void *quos_socketGetChlFd(pointer_t sockFd, quint8_t type);
static qbool quos_socketCheckChlFd(const void *chlFd);
static quint32_t quos_socketSendDataByteSize(void *chlFd);
static qbool quos_socketChannelDel(void *chlFd);
/**************************************************************************
** 功能	@brief : socket通道初始化
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_socketInit(void)
{
    HAL_LOCK_INIT(lockId);
}

/**************************************************************************
** 功能	@brief : socket connect成功处理
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_socketIOConnResult(pointer_t sockFd, quint8_t type, qbool result)
{
    HAL_LOCK(lockId);
    Quos_socketChlInfoNode_t *node = (Quos_socketChlInfoNode_t *)quos_socketGetChlFd(sockFd, type);
    if (node && node->valid)
    {
        Quos_swTimerDelete(node->conn.timer);
        if (node->conn.notify)
        {
            HAL_UNLOCK(lockId);
            node->conn.notify((void *)node, result);
            HAL_LOCK(lockId);
        }
        if (FALSE == result)
        {
            quos_socketChannelDel((void *)node);
        }
        Quos_kernelResume();
    }
    HAL_UNLOCK(lockId);
}
/**************************************************************************
** 功能	@brief : socket connect超时处理,超时删除掉channel
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_socketConnTimeout(void *swTimer)
{
    HAL_LOCK(lockId);
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)Quos_swTimerParmGet(swTimer);
    Quos_logPrintf(LSDK_SOCK, LL_ERR, "chlNode[%p] conn timeout", chlNode);
    if (FALSE == quos_socketCheckChlFd(chlNode))
    {
        HAL_UNLOCK(lockId);
        return;
    }
    if (chlNode->valid)
    {
        if (chlNode->conn.notify)
        {
            HAL_UNLOCK(lockId);
            chlNode->conn.notify((void *)chlNode, FALSE);
            HAL_LOCK(lockId);
        }
        quos_socketChannelDel((void *)chlNode);
        Quos_kernelResume();
        Quos_logPrintf(LSDK_SOCK, LL_INFO, "del sockChlInfo");
    }
    HAL_UNLOCK(lockId);
}
/**************************************************************************
** 功能	@brief : socket通道增加
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM *Quos_socketChannelAdd(void **chlFdPoint, Quos_socketChlInfoNode_t chlInfo)
{
    Quos_logPrintf(LSDK_SOCK, LL_INFO, "sockFd[" PRINTF_FD "] type[%u] sendCnt[%u] sendTimeout[%u] io.send[%p] recvDataFunc[%p] unformFunc[%p] param:%p",
                   chlInfo.sockFd, chlInfo.type, chlInfo.send.txCnt, chlInfo.send.timeout, chlInfo.io.send, chlInfo.recvDataFunc, chlInfo.unformFunc, chlInfo.param);

    if (SOCKET_FD_INVALID == chlInfo.sockFd || 0 == chlInfo.send.txCnt || 0 == chlInfo.send.timeout)
    {
        Quos_logPrintf(LSDK_SOCK, LL_ERR, "arg is invalid sockFd:" PRINTF_FD " sendCnt:%u sendTimeout:%u cb:%p timeout:%u",
                       chlInfo.sockFd, chlInfo.send.txCnt, chlInfo.send.timeout, chlInfo.conn.notify, chlInfo.conn.timeout);
        return NULL;
    }
    HAL_LOCK(lockId);
    Quos_socketChlInfoNode_t *node = (Quos_socketChlInfoNode_t *)quos_socketGetChlFd(chlInfo.sockFd, chlInfo.type);
    if (node)
    {
        Quos_logPrintf(LSDK_SOCK, LL_WARN, "find invalid fd in socket channel list");
        quos_socketChannelDel((void *)node);
        Quos_kernelResume();
    }
    HAL_UNLOCK(lockId);

    Quos_socketChlInfoNode_t *newChlInfo = HAL_MALLOC(sizeof(Quos_socketChlInfoNode_t));
    if (NULL == newChlInfo)
    {
        Quos_logPrintf(LSDK_SOCK, LL_ERR, "MALLOC newChlInfo fail");
        return NULL;
    }
    HAL_MEMCPY(newChlInfo, &chlInfo, sizeof(Quos_socketChlInfoNode_t));
    if (newChlInfo->conn.timeout)
    {
        if (FALSE == Quos_swTimerStart(&newChlInfo->conn.timer, "socket connect", newChlInfo->conn.timeout, 1, quos_socketConnTimeout, (void *)newChlInfo))
        {
            Quos_logPrintf(LSDK_SOCK, LL_ERR, "conntime start fail");
            HAL_FREE(newChlInfo);
            return NULL;
        }
    }
    if (chlFdPoint)
    {
        *chlFdPoint = newChlInfo;
    }
    newChlInfo->self = chlFdPoint;
    newChlInfo->valid = TRUE;
    HAL_LOCK(lockId);
    Quos_twllHeadAdd(&chlInfoListHead, &newChlInfo->head);
    Quos_logPrintf(LSDK_SOCK, LL_INFO, "ok, node count:%u", Quos_twllHeadGetNodeCount(chlInfoListHead));
    HAL_UNLOCK(lockId);
    Quos_kernelResume();
    return (void *)newChlInfo;
}
/**************************************************************************
** 功能	@brief : socket通道删除
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_socketChannelDel(void *chlFd)
{
    Quos_logPrintf(LSDK_SOCK, LL_INFO, "chlFd[%p]", chlFd);
    if (quos_socketCheckChlFd(chlFd))
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        if (chlNode->valid)
        {
            chlNode->valid = FALSE;
            if (chlNode->self)
            {
                *chlNode->self = NULL;
                chlNode->self = NULL;
            }
            Quos_swTimerDelete(chlNode->conn.timer);
            Quos_kernelResume();
            return TRUE;
        }
        Quos_logPrintf(LSDK_SOCK, LL_DBG, "del chl and send node ok");
    }
    return FALSE;
}
qbool FUNCTION_ATTR_ROM Quos_socketChannelDel(void *chlFd)
{
    HAL_LOCK(lockId);
    qbool ret = quos_socketChannelDel(chlFd);
    HAL_UNLOCK(lockId);
    return ret;
}
/**************************************************************************
** 功能	@brief : socket通信 TASK
** 输入	@param :
** 输出	@retval: FALSE:任何通道无数据发送
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_socketTask(void)
{
    quint32_t idleTime = SWT_SUSPEND;
    HAL_LOCK(lockId);
    TWLLHead_T *chlTemp, *chlNext;
    TWLIST_FOR_DATA(chlInfoListHead, chlTemp, chlNext)
    {
        Quos_socketChlInfoNode_t *chlNode = __GET_STRUCT_BY_ELEMENT(chlTemp, Quos_socketChlInfoNode_t, head);
        qint32_t interval = Quos_sysTickdiff(Quos_sysTickGet(), chlNode->send.beginTime, TRUE);
        if (interval < 0)
        {
            interval = 0;
            chlNode->send.beginTime = Quos_sysTickGet();
        }
        if (FALSE == chlNode->valid) /*socket通道已被删除，则在此FREE内存*/
        {
            Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlNode[%p] memory free", chlNode);
            Quos_twllHeadDelete(&chlInfoListHead, &chlNode->head);
            if (chlNode->unformTemp.buf)
            {
                HAL_FREE(chlNode->unformTemp.buf);
            }
            TWLLHead_T *sendTemp, *sendNext;
            TWLIST_FOR_DATA(chlNode->send.disorderList, sendTemp, sendNext)
            {
                Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(sendTemp, Quos_socketSendDataNode_t, head);
                Quos_twllHeadDelete(&chlNode->send.disorderList, &sendNode->head);
                HAL_FREE(sendNode->Buf);
                HAL_FREE(sendNode);
            }
            TWLIST_FOR_DATA(chlNode->send.orderList, sendTemp, sendNext)
            {
                Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(sendTemp, Quos_socketSendDataNode_t, head);
                Quos_twllHeadDelete(&chlNode->send.orderList, &sendNode->head);
                HAL_FREE(sendNode->Buf);
                HAL_FREE(sendNode);
            }
            if (chlNode->io.close)
            {
                (chlNode->io.close)(chlNode->sockFd, chlNode->type);
            }
            if (chlNode->paramFree && chlNode->param)
            {
                chlNode->paramFree(chlNode->param);
            }
            HAL_FREE(chlNode);
        }
        else if (chlNode->conn.timer) /* socket connecting中 */
        {
            /* do no */
        }
        else if ((quint32_t)interval < chlNode->send.sendInterVal) /*处在限速中 */
        {
            Quos_logPrintf(LSDK_SOCK, LL_DBG, "send rate limited:%d/%u", interval, chlNode->send.sendInterVal);
            if (idleTime > chlNode->send.sendInterVal - interval)
            {
                idleTime = chlNode->send.sendInterVal - interval;
            }
        }
        else if (0 == chlNode->send.waitIoSendAck && (chlNode->send.disorderList || FALSE == chlNode->send.waitPkgAck))
        {
            qbool iosendRet = TRUE;
            qbool isSending = FALSE;
            if (chlNode->send.disorderList)
            {
                Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(chlNode->send.disorderList, Quos_socketSendDataNode_t, head);
                iosendRet = (chlNode->io.send)(chlNode->sockFd, chlNode->type, sendNode->peer, sendNode->Buf, sendNode->bufLen, &isSending);
                Quos_logPrintf(LSDK_SOCK, LL_DBG, "disorderList send,sendNode:%p iosendRet:%s isSending:%s", sendNode, _BOOL2STR(iosendRet), _BOOL2STR(isSending));
                chlNode->send.beginTime = Quos_sysTickGet();
                if (iosendRet)
                {
                    if (FALSE == isSending)
                    {
                        Quos_twllHeadDelete(&chlNode->send.disorderList, chlNode->send.disorderList);
                        HAL_FREE(sendNode->Buf);
                        HAL_FREE(sendNode);
                        idleTime = 0;
                    }
                    else
                    {
                        chlNode->send.waitIoSendAck = 1;
                        if (idleTime > sendNode->sendTimeout)
                        {
                            idleTime = sendNode->sendTimeout;
                        }
                    }
                }
                else
                {
                    idleTime = 0;
                }
            }
            else if (chlNode->send.orderList && FALSE == chlNode->send.waitPkgAck)
            {
                Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(chlNode->send.orderList, Quos_socketSendDataNode_t, head);
                iosendRet = (chlNode->io.send)(chlNode->sockFd, chlNode->type, sendNode->peer, sendNode->Buf, sendNode->bufLen, &isSending);
                Quos_logPrintf(LSDK_SOCK, LL_DBG, "orderList send,sendNode:%p iosendRet:%s isSending:%s", sendNode, _BOOL2STR(iosendRet), _BOOL2STR(isSending));
                chlNode->send.beginTime = Quos_sysTickGet();
                if (TRUE == iosendRet)
                {
                    Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] sendNode[%p] io send ok,isSending[%s] sendCnt:%u", chlNode, sendNode, _BOOL2STR(isSending), sendNode->sendCnt);
                    if (FALSE == isSending && sendNode->sendCB)
                    {
                        HAL_UNLOCK(lockId);
                        qbool sendCbRet = sendNode->sendCB((void *)chlNode, sendNode, TRUE);
                        HAL_LOCK(lockId);
                        if (sendCbRet)
                        {
                            if (sendNode == __GET_STRUCT_BY_ELEMENT(chlNode->send.orderList, Quos_socketSendDataNode_t, head))
                            {
                                Quos_twllHeadDelete(&chlNode->send.orderList, chlNode->send.orderList);
                                HAL_FREE(sendNode->Buf);
                                HAL_FREE(sendNode);
                            }
                            continue;
                        }
                    }

                    if (FALSE == isSending && 0 == sendNode->sendCnt)
                    {
                        Quos_twllHeadDelete(&chlNode->send.orderList, chlNode->send.orderList);
                        HAL_FREE(sendNode->Buf);
                        HAL_FREE(sendNode);
                        idleTime = 0;
                    }
                    else
                    {
                        if (TRUE == isSending)
                        {
                            chlNode->send.waitIoSendAck = 2;
                            Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] sendNode[%p] need wait sendcb", chlNode, sendNode);
                        }
                        if (0 != sendNode->sendCnt)
                        {
                            chlNode->send.waitPkgAck = TRUE;
                            Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] sendNode[%p] need wait pkg ack", chlNode, sendNode);
                        }
                        if (idleTime > sendNode->sendTimeout)
                        {
                            idleTime = sendNode->sendTimeout;
                        }
                    }
                }
                else
                {
                    idleTime = 0;
                }
            }
            if (FALSE == iosendRet)
            {
                Quos_logPrintf(LSDK_SOCK, LL_ERR, "chlFd[%p] io send fail", chlNode);
                HAL_UNLOCK(lockId);
                chlNode->recvDataFunc((void *)chlNode, NULL, 0, NULL);
                HAL_LOCK(lockId);
                quos_socketChannelDel((void *)chlNode);
            }
        }
        else if (1 == chlNode->send.waitIoSendAck)
        {
            Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(chlNode->send.disorderList, Quos_socketSendDataNode_t, head);
            if (interval > (qint32_t)sendNode->sendTimeout)
            {
                Quos_twllHeadDelete(&chlNode->send.disorderList, chlNode->send.disorderList);
                HAL_FREE(sendNode->Buf);
                HAL_FREE(sendNode);
                chlNode->send.waitIoSendAck = 0;
                idleTime = 0;
            }
        }
        else if (2 == chlNode->send.waitIoSendAck || TRUE == chlNode->send.waitPkgAck)
        {
            Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(chlNode->send.orderList, Quos_socketSendDataNode_t, head);
            qint32_t recvInter = Quos_sysTickdiff(Quos_sysTickGet(), chlNode->send.recvTime, TRUE);
            if (interval > (qint32_t)sendNode->sendTimeout && recvInter >= (qint32_t)chlNode->send.timeout / 2)
            {
                chlNode->send.waitIoSendAck = 0;
                chlNode->send.waitPkgAck = FALSE;
                if (sendNode->sendCnt > 0)
                {
                    sendNode->sendCnt--;
                }
                if (0 == sendNode->sendCnt)
                {
                    if (sendNode->recvCB)
                    {
                        HAL_UNLOCK(lockId);
                        sendNode->recvCB((void *)chlNode, sendNode, NULL);
                        HAL_LOCK(lockId);
                    }
                    if (sendNode == __GET_STRUCT_BY_ELEMENT(chlNode->send.orderList, Quos_socketSendDataNode_t, head))
                    {
                        Quos_twllHeadDelete(&chlNode->send.orderList, chlNode->send.orderList);
                        HAL_FREE(sendNode->Buf);
                        HAL_FREE(sendNode);
                    }
                }
                idleTime = 0;
            }
            else if ((quint32_t)interval < sendNode->sendTimeout)
            {
                idleTime = sendNode->sendTimeout - interval;
            }
            else
            {
                idleTime = chlNode->send.timeout / 2 - recvInter;
            }
        }
    }
    HAL_UNLOCK(lockId);
    return idleTime;
}
/**************************************************************************
** 功能	@brief : socket添加发送数据到有序发送链表
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_socketTx(const void *chlFd, void *peer, quint8_t sendCnt, quint32_t sendTimeout, socketsendNodeCb_f sendCB, socketRecvNodeCb_f recvCB,
                                      quint32_t pkgId, quint8_t *buf, quint16_t bufLen, const void *param)
{
    qbool ret = FALSE;
    Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] sendCnt[%u] sendTimeout[%u] sendCB[%p] recvCB[%p] pkgId[%u] bufLen[%u]", chlFd, sendCnt, sendTimeout, sendCB, recvCB, pkgId, bufLen);
    Quos_logHexDump(LSDK_SOCK, LL_DUMP, "send", buf, bufLen);
    HAL_LOCK(lockId);
    if (quos_socketSendDataByteSize(NULL) < SOCKET_SENDDATA_BYTESIZE_MAX && quos_socketCheckChlFd(chlFd))
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        Quos_socketSendDataNode_t *sendNode;
        if (chlNode->valid && chlNode->io.send && (sendNode = HAL_MALLOC(sizeof(Quos_socketSendDataNode_t))) != NULL)
        {
            HAL_MEMSET(sendNode, 0, sizeof(Quos_socketSendDataNode_t));
            Quos_twllHeadAdd(&chlNode->send.orderList, &sendNode->head);
            Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] add sendNode[%p]", chlNode, sendNode);
            sendNode->param = (void *)param;

            sendNode->sendCnt = (recvCB) ? ((sendCnt == 0) ? chlNode->send.txCnt : sendCnt) : 0;
            sendNode->sendTimeout = sendTimeout ? sendTimeout : chlNode->send.timeout;
            sendNode->sendCB = sendCB;
            sendNode->recvCB = recvCB;
            sendNode->pkgId = pkgId;
            sendNode->peer = peer;
            sendNode->Buf = buf;
            sendNode->bufLen = bufLen;
            Quos_kernelResume();
            ret = TRUE;
        }
    }
    if (FALSE == ret && buf)
    {
        HAL_FREE(buf);
    }
    HAL_UNLOCK(lockId);
    return ret;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_socketTxDisorder(const void *chlFd, void *peer, quint8_t *buf, quint16_t bufLen)
{
    qbool ret = FALSE;
    HAL_LOCK(lockId);
    Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] bufLen[%u]", chlFd, bufLen);
    Quos_logHexDump(LSDK_SOCK, LL_DUMP, "send", buf, bufLen);
    if (quos_socketSendDataByteSize(NULL) < SOCKET_SENDDATA_BYTESIZE_MAX && quos_socketCheckChlFd(chlFd))
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        Quos_socketSendDataNode_t *sendNode;
        if (chlNode->valid && chlNode->io.send && (sendNode = HAL_MALLOC(sizeof(Quos_socketSendDataNode_t))) != NULL)
        {
            HAL_MEMSET(sendNode, 0, sizeof(Quos_socketSendDataNode_t));
            Quos_twllHeadAdd(&chlNode->send.disorderList, &sendNode->head);
            Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] add sendNode[%p]", chlNode, sendNode);
            sendNode->sendTimeout = chlNode->send.timeout;
            sendNode->peer = peer;
            sendNode->Buf = buf;
            sendNode->bufLen = bufLen;
            Quos_kernelResume();
            ret = TRUE;
        }
    }
    if (FALSE == ret && buf)
    {
        HAL_FREE(buf);
    }
    HAL_UNLOCK(lockId);
    return ret;
}
/**************************************************************************
** 功能	@brief : socket发送数据出去完成回调
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_socketIOTxCb(pointer_t sockFd, quint8_t type)
{
    HAL_LOCK(lockId);
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)quos_socketGetChlFd(sockFd, type);
    if (chlNode && chlNode->valid)
    {
        if (1 == chlNode->send.waitIoSendAck && chlNode->send.disorderList)
        {
            Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(chlNode->send.disorderList, Quos_socketSendDataNode_t, head);
            Quos_twllHeadDelete(&chlNode->send.disorderList, chlNode->send.disorderList);
            HAL_FREE(sendNode->Buf);
            HAL_FREE(sendNode);
            chlNode->send.waitIoSendAck = 0;
        }
        else if (2 == chlNode->send.waitIoSendAck && chlNode->send.orderList)
        {
            Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(chlNode->send.orderList, Quos_socketSendDataNode_t, head);
            qbool sendCbRet = FALSE;
            if (sendNode->sendCB)
            {
                HAL_UNLOCK(lockId);
                sendCbRet = sendNode->sendCB((void *)chlNode, sendNode, TRUE);
                HAL_LOCK(lockId);
            }
            if (quos_socketCheckChlFd(chlNode))
            {
                if ((sendCbRet || FALSE == chlNode->send.waitPkgAck) && sendNode == __GET_STRUCT_BY_ELEMENT(chlNode->send.orderList, Quos_socketSendDataNode_t, head))
                {
                    Quos_twllHeadDelete(&chlNode->send.orderList, chlNode->send.orderList);
                    HAL_FREE(sendNode->Buf);
                    HAL_FREE(sendNode);
                }
                chlNode->send.waitIoSendAck = 0;
            }
        }
        else
        {
            chlNode->send.waitIoSendAck = 0;
        }
        Quos_kernelResume();
    }
    HAL_UNLOCK(lockId);
}
/**************************************************************************
** 功能	@brief : socket通信数据收到发送应答
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_socketTxAck(void *chlFd, const void *peer, quint32_t pkgId, const void *recvData)
{
    Quos_logPrintf(LSDK_SOCK, LL_DBG, "recv chlFd[%p] pkgId[%u]", chlFd, pkgId);
    qbool ret = FALSE;
    HAL_LOCK(lockId);
    if (quos_socketCheckChlFd(chlFd))
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        if (chlNode->valid && chlNode->send.orderList && TRUE == chlNode->send.waitPkgAck)
        {
            Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(chlNode->send.orderList, Quos_socketSendDataNode_t, head);
            if (sendNode->pkgId == pkgId && sendNode->peer == peer)
            {
                Quos_logPrintf(LSDK_SOCK, LL_DBG, "pkgId is match send's");
                chlNode->send.waitIoSendAck = 0;
                chlNode->send.waitPkgAck = FALSE;
                Quos_twllHeadDelete(&chlNode->send.orderList, chlNode->send.orderList);
                HAL_UNLOCK(lockId);
                sendNode->recvCB((void *)chlNode, sendNode, recvData);
                HAL_LOCK(lockId);
                if (quos_socketCheckChlFd(chlNode))
                {
                    HAL_FREE(sendNode->Buf);
                    HAL_FREE(sendNode);
                }
                Quos_kernelResume();
                ret = TRUE;
            }
        }
    }
    HAL_UNLOCK(lockId);
    return ret;
}
/**************************************************************************
** 功能	@brief : socket添加接收数据到接收链表
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_socketIORx(pointer_t sockFd, quint8_t type, const void *peer, quint32_t peerSize, quint8_t *buf, quint32_t bufLen)
{
    HAL_LOCK(lockId);
    Quos_logPrintf(LSDK_SOCK, LL_DBG, "sockFd[" PRINTF_FD "] bufLen[%u]", sockFd, bufLen);
    Quos_logHexDump(LSDK_SOCK, LL_DUMP, "recv", buf, bufLen);
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)quos_socketGetChlFd(sockFd, type);
    if (chlNode && chlNode->valid)
    {
        chlNode->send.recvTime = Quos_sysTickGet();
        if (chlNode->recvDataFunc)
        {
            Quos_socketRecvDataNode_t newNode;
            HAL_MEMSET(&newNode, 0, sizeof(Quos_socketRecvDataNode_t));
            if (0 == bufLen)
            {
                chlNode->io.close = NULL; /* no close repeat */
                HAL_UNLOCK(lockId);
                chlNode->recvDataFunc((void *)chlNode, peer, peerSize, NULL);
                HAL_LOCK(lockId);
                quos_socketChannelDel((void *)chlNode);
                Quos_kernelResume();
            }
            else if (chlNode->unformFunc)
            {
                Quos_logPrintf(LSDK_SOCK, LL_DBG, "need to unform before");
                quint32_t dealLen = 0;
                while (dealLen < bufLen && quos_socketCheckChlFd(chlNode))
                {
                    quint32_t offset = 0;
                    if (FALSE == chlNode->unformFunc(buf + dealLen, bufLen - dealLen, &offset, &chlNode->unformTemp))
                    {
                        Quos_logPrintf(LSDK_SOCK, LL_DBG, "unform fail");
                    }
                    else
                    {
                        Quos_logHexDump(LSDK_SOCK, LL_DUMP, "unform ok", chlNode->unformTemp.buf, chlNode->unformTemp.offset);
                        newNode.bufLen = chlNode->unformTemp.offset;
                        newNode.Buf = chlNode->unformTemp.buf;
                        chlNode->unformTemp.offset = 0;
                        HAL_UNLOCK(lockId);
                        chlNode->recvDataFunc((void *)chlNode, peer, peerSize, &newNode);
                        HAL_LOCK(lockId);
                    }
                    dealLen += offset;
                }
            }
            else
            {
                Quos_logPrintf(LSDK_SOCK, LL_DBG, "recvDataFunc direct");
                newNode.bufLen = bufLen;
                newNode.Buf = buf;
                HAL_UNLOCK(lockId);
                chlNode->recvDataFunc((void *)chlNode, peer, peerSize, &newNode);
                HAL_LOCK(lockId);
            }
        }
    }
    HAL_UNLOCK(lockId);
}

/**************************************************************************
** 功能	@brief : 获取socket类型
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_socketGetSockFdType(void *chlFd, pointer_t *sockFd, quint8_t *type)
{
    qbool ret = FALSE;
    HAL_LOCK(lockId);
    if (quos_socketCheckChlFd(chlFd))
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        if (chlNode->valid)
        {
            if (sockFd)
            {
                *sockFd = chlNode->sockFd;
            }
            if (type)
            {
                *type = chlNode->type;
            }
            ret = TRUE;
        }
    }
    HAL_UNLOCK(lockId);
    return ret;
}
/**************************************************************************
** 功能	@brief : 获取指定节点
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM *quos_socketGetChlFd(pointer_t sockFd, quint8_t type)
{
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA(chlInfoListHead, temp, next)
    {
        Quos_socketChlInfoNode_t *chlNode = __GET_STRUCT_BY_ELEMENT(temp, Quos_socketChlInfoNode_t, head);
        if (chlNode->sockFd == sockFd && chlNode->type == type)
        {
            Quos_logPrintf(LSDK_SOCK, LL_DBG, "find chlFd[%p] ok", chlNode);
            return chlNode;
        }
    }
    return NULL;
}
void FUNCTION_ATTR_ROM *Quos_socketGetChlFd(pointer_t sockFd, quint8_t type)
{
    HAL_LOCK(lockId);
    void *node = quos_socketGetChlFd(sockFd, type);
    HAL_UNLOCK(lockId);
    return node;
}
/**************************************************************************
** 功能	@brief : 获取指定类型和参数指针匹配的chlFd列表
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t Quos_socketGetChlFdList(quint8_t type, const void *param, void *chlFd[], quint32_t maxSize)
{
    quint32_t count = 0;
    HAL_LOCK(lockId);
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA(chlInfoListHead, temp, next)
    {
        Quos_socketChlInfoNode_t *chlNode = __GET_STRUCT_BY_ELEMENT(temp, Quos_socketChlInfoNode_t, head);
        if (chlNode->type == type && chlNode->param == param)
        {
            chlFd[count++] = chlNode;
            if(count == maxSize)
            {
                break;
            }
        }
    }
    HAL_UNLOCK(lockId);
    return count;
}
/**************************************************************************
** 功能	@brief : 判断chlFd是否存在
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_socketCheckChlFd(const void *chlFd)
{
    qbool ret = FALSE;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA(chlInfoListHead, temp, next)
    {
        Quos_socketChlInfoNode_t *chlNode = __GET_STRUCT_BY_ELEMENT(temp, Quos_socketChlInfoNode_t, head);
        if (chlNode == (const Quos_socketChlInfoNode_t *)chlFd)
        {
            ret = TRUE;
            break;
        }
    }
    Quos_logPrintf(LSDK_SOCK, LL_DBG, "find chlFd[%p] %s", chlFd, _BOOL2STR(ret));
    return ret;
}
qbool FUNCTION_ATTR_ROM Quos_socketCheckChlFd(const void *chlFd)
{
    HAL_LOCK(lockId);
    qbool ret = quos_socketCheckChlFd(chlFd);
    HAL_UNLOCK(lockId);
    return ret;
}
/**************************************************************************
** 功能	@brief : socket通道链表获取
** 输入	@param :
** 输出	@retval:
***************************************************************************/
TWLLHead_T FUNCTION_ATTR_ROM *Quos_socketGetChlHead(void)
{
    return chlInfoListHead;
}
/**************************************************************************
** 功能	@brief : 变更socket接收处理回调函数
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_socketInfoModity(void *chlFd, quint8_t sendCnt, quint32_t sendTimeout, socketRecvNotify_f recvDataFunc)
{
    HAL_LOCK(lockId);
    qbool ret = quos_socketCheckChlFd(chlFd);
    if (ret)
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        chlNode->send.txCnt = sendCnt ? sendCnt : chlNode->send.txCnt;
        chlNode->send.timeout = sendTimeout ? sendTimeout : chlNode->send.timeout;
        chlNode->recvDataFunc = recvDataFunc ? recvDataFunc : chlNode->recvDataFunc;
        Quos_kernelResume();
    }
    HAL_UNLOCK(lockId);
    return ret;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static quint32_t FUNCTION_ATTR_ROM quos_socketSendDataByteSize(void *chlFd)
{
    quint32_t byteSize = 0;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA(chlInfoListHead, temp, next)
    {
        Quos_socketChlInfoNode_t *chlNode = __GET_STRUCT_BY_ELEMENT(temp, Quos_socketChlInfoNode_t, head);
        if (NULL == chlFd || chlNode == (Quos_socketChlInfoNode_t *)chlFd)
        {
            TWLLHead_T *sendTemp, *sendNext;
            quint32_t nodeByteSize = 0;
            TWLIST_FOR_DATA(chlNode->send.orderList, sendTemp, sendNext)
            {
                Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(sendTemp, Quos_socketSendDataNode_t, head);
                nodeByteSize += sendNode->bufLen;
            }
            TWLIST_FOR_DATA(chlNode->send.disorderList, sendTemp, sendNext)
            {
                Quos_socketSendDataNode_t *sendNode = __GET_STRUCT_BY_ELEMENT(sendTemp, Quos_socketSendDataNode_t, head);
                nodeByteSize += sendNode->bufLen;
            }
            byteSize += nodeByteSize;
            //Quos_logPrintf(LSDK_SOCK, LL_DBG, "chlFd[%p] byteSize=%u/%u", chlNode, nodeByteSize, byteSize);
        }
    }
    return byteSize;
}
quint32_t FUNCTION_ATTR_ROM Quos_socketSendDataByteSize(void *chlFd)
{
    HAL_LOCK(lockId);
    quint32_t size = quos_socketSendDataByteSize(chlFd);
    HAL_UNLOCK(lockId);
    return size;
}
