/*
 * @Author: your name
 * @Date: 2021-11-04 19:23:58
 * @LastEditTime: 2021-11-05 11:56:19
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \QuecCSDK\cloud\common\ql_iotCmdLan.c
 */
#include "ql_iotCmdLan.h"
#include "ql_iotConfig.h"
#include "ql_iotDp.h"
#include "Qhal_driver.h"

#ifdef QUEC_ENABLE_LAN

#ifndef QIOT_LAN_TIMEOUT
#define QIOT_LAN_TIMEOUT 5 * SWT_ONE_SECOND
#endif

static void *lanChl = NULL;
/**************************************************************************
** 功能	@brief : 设备上报PK、MAC内容
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM ql_iotLanDevDiscover(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(payload);
    UNUSED(payloadLen);
    void *ttlvHead = NULL;
    Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_LAN_MAC, QIot_userdata.deviceInfo.deviceKey);
    Ql_iotTtlvIdAddString(&ttlvHead, QIOT_DPID_LAN_PK, QIot_userdata.productInfo.productKey);
    Ql_iotDpSendTtlvRsp(app, endPoint, QIOT_DPCMD_LAN_DISCOVER_RSP, pkgId, ttlvHead);
    Ql_iotTtlvFree(&ttlvHead);
}


/**************************************************************************
** 功能	@brief : LAN接收数据
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM ql_iotLanRecvData(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData)
{
    UNUSED(chlFd);
    UNUSED(peer);
    UNUSED(peerSize);
    /* 在此缺少将peer加入通信总线管理 */
    if (recvData)
    {
        Ql_iotDpHandle(QIOT_DPAPP_LANPHONE, NULL, recvData->Buf, recvData->bufLen);
    }
    return TRUE;
}

/**************************************************************************
** 功能	@brief : LAN初始化
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdLanInit(void)
{
    Quos_socketChlInfoNode_t chlInfo;
    HAL_MEMSET(&chlInfo, 0, sizeof(Quos_socketChlInfoNode_t));
    chlInfo.sockFd = Qhal_udpInit(&chlInfo.type, QIOT_LAN_PORT, NULL, 0, NULL);
    Quos_logPrintf(QUEC_LAN, LL_DBG, "LANFd:" PRINTF_FD, chlInfo.sockFd);
    if (SOCKET_FD_INVALID == chlInfo.sockFd)
    {
        Quos_logPrintf(QUEC_LAN, LL_ERR, "Listening on LAN port failed");
        return FALSE;
    }
    chlInfo.io.send = Qhal_sockWrite;
    chlInfo.send.txCnt = 1;
    chlInfo.send.timeout = QIOT_LAN_TIMEOUT;
    chlInfo.recvDataFunc = ql_iotLanRecvData;
    chlInfo.io.close = Qhal_sockClose;
    lanChl = Quos_socketChannelAdd(NULL, chlInfo);
    if (NULL == lanChl)
    {
        Quos_logPrintf(QUEC_LAN, LL_ERR, "add socket Channel fail");
        return FALSE;
    }
    return TRUE;
}
#endif
