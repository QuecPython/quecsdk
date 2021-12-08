/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    : 2020-12-25
** ����   @brief   : dmpҵ��ָ��,����DMP2.2.0
** Ӳ��   @hardware��
** ����   @other   ��
***************************************************************************/
#include "ql_iotCmdBus.h"
#include "ql_iotCmdSys.h"
#include "ql_iotCmdLoc.h"
#include "ql_iotDp.h"
#include "ql_iotTtlv.h"
#include "ql_iotConfig.h"
#include "Qhal_driver.h"

Ql_iotCmdBusInfo_t QIot_busInfo;

/**************************************************************************
** ����	@brief : ͸�����ݷ��ͽ��
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotCmdBusPassTransSendCB(void *chlFd, const void *sendData, const void *recvData)
{
    UNUSED(chlFd);
    UNUSED(sendData);
    Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_SEND, recvData ? QIOT_SEND_SUCC_TRANS : QIOT_SEND_ERR_TRANS, NULL, 0);
}
/**************************************************************************
** ����	@brief : ͸�����ݷ���
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBusPassTransSend(quint16_t mode, quint8_t *payload, quint32_t len)
{
    if (0 == len)
    {
        return FALSE;
    }
    return Ql_iotDpSendCommonReq(QIOT_DPAPP_M2M | QIOT_DPAPP_LANPHONE,NULL, 0, mode, QIOT_DPCMD_PASS_EVENT, payload, len, ql_iotCmdBusPassTransSendCB);
}
/**************************************************************************
** ����	@brief : ͸�����ݽ���
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdBusPassTransRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(app);
    UNUSED(endPoint);
    UNUSED(pkgId);
    QIot_buffer_t *recvBuf = HAL_MALLOC(sizeof(QIot_buffer_t) + payloadLen);
    if (NULL == recvBuf)
    {
        Quos_logPrintf(QUEC_BUS, LL_ERR, "mcf recvBuf");
        return;
    }
    recvBuf->type = QIOT_RECV_SUCC_TRANS;
    recvBuf->val.len = payloadLen;
    HAL_MEMCPY(recvBuf->val.buf, payload, payloadLen);
    if (FALSE == Quos_eventPost(QIOT_ATEVENT_TYPE_RECV, recvBuf))
    {
        HAL_FREE(recvBuf);
    }
}
/**************************************************************************
** ����	@brief : ��ģ�����ݷ��ͽ��
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotCmdBusPhymodelReportCB(void *chlFd, const void *sendData, const void *recvData)
{
    UNUSED(chlFd);
    UNUSED(sendData);
    Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_SEND, recvData ? QIOT_SEND_SUCC_PHYMODEL : QIOT_SEND_ERR_PHYMODEL, NULL, 0);
}
/**************************************************************************
** ����	@brief : ��ģ�������ϱ�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBusPhymodelReport(quint16_t mode, const void *ttlvHead)
{
    if (NULL == ttlvHead)
    {
        return FALSE;
    }
    return Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M|QIOT_DPAPP_LANPHONE,NULL, 0, mode, QIOT_DPCMD_TSL_EVENT, ttlvHead, ql_iotCmdBusPhymodelReportCB);
}
/**************************************************************************
** ����	@brief : ��ģ������Ӧ��
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBusPhymodelAck(quint16_t mode, quint16_t pkgId, const void *ttlvHead)
{
    UNUSED(mode);
    return Ql_iotDpSendTtlvRsp(QIOT_DPAPP_M2M,NULL, QIOT_DPCMD_TSL_RSP, pkgId, ttlvHead);
}
/**************************************************************************
** ����	@brief : ��ģ�������ϱ�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBusPhymodelReportHex(quint16_t mode, quint8_t *buf, quint32_t len)
{
    return Ql_iotDpSendCommonReq(QIOT_DPAPP_M2M|QIOT_DPAPP_LANPHONE, NULL, 0, mode, QIOT_DPCMD_TSL_EVENT, buf, len, ql_iotCmdBusPhymodelReportCB);
}
/**************************************************************************
** ����	@brief : ��ģ������Ӧ��
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBusPhymodelAckHex(quint16_t mode, quint16_t pkgId, quint8_t *buf, quint32_t len)
{
    UNUSED(mode);
    return Ql_iotDpSendCommonRsp(QIOT_DPAPP_M2M,NULL, QIOT_DPCMD_TSL_RSP, pkgId, buf, len);
}
/**************************************************************************
** ����	@brief : ��ģ�����ݽ���
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Ql_iotCmdBusPhymodelWriteRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(app);
    UNUSED(endPoint);
    void *ttlvHead = Ql_iotTtlvUnformat(payload, payloadLen);
    if (NULL == ttlvHead)
    {
        Ql_iotCmdSysExceReport(app,endPoint, QIOT_SERVER_ERRCODE_UNFORMAT_FAIL, pkgId);
        return;
    }
    Ql_iotTtlvFree(&ttlvHead);

    QIot_buffer_t *recvBuf = NULL;
    recvBuf = HAL_MALLOC(sizeof(QIot_buffer_t) + payloadLen);
    if (NULL == recvBuf)
    {
        Quos_logPrintf(QUEC_BUS, LL_ERR, "mcf recvBuf");
        return;
    }
    HAL_MEMCPY(recvBuf->val.buf, payload, payloadLen);
    recvBuf->val.len = payloadLen;
    recvBuf->type = QIOT_RECV_SUCC_PHYMODEL_RECV;
    if (FALSE == Quos_eventPost(QIOT_ATEVENT_TYPE_RECV, recvBuf))
    {
        HAL_FREE(recvBuf);
    }
}
/**************************************************************************
** ����	@brief : ��ģ����������
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Ql_iotCmdBusPhymodelReqRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    UNUSED(app);
    UNUSED(endPoint);
    quint16_t *ids = (quint16_t *)HAL_MALLOC(sizeof(quint16_t) * (payloadLen / 2 + 1));
    if (ids)
    {
        quint32_t idNum;
        ids[0] = pkgId;
        for (idNum = 0; idNum < payloadLen / 2; idNum++)
        {
            ids[idNum + 1] = _ARRAY01_U16(payload + idNum * 2);
        }
        Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_RECV, QIOT_RECV_SUCC_PHYMODEL_REQ, ids, idNum);
        HAL_FREE(ids);
    }
}
/**************************************************************************
** ����	@brief : ҵ���¼�����
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static void ql_iotCmdBusEventNotify(qint32_t event, void *arg)
{
    switch (event)
    {
    case QIOT_ATEVENT_TYPE_RECV:
    {
        QIot_buffer_t *recvBuf = (QIot_buffer_t *)arg;
        #ifdef QUEC_ENABLE_AT
        if (QIot_busInfo.recvIsBuffer)
        {
            if (QIOT_RECV_SUCC_TRANS == recvBuf->type && Quos_twllHeadGetNodeCount(QIot_busInfo.recvTransData) < QIOT_RECV_NODE_TRANS_MAX)
            {
                Quos_twllHeadAdd(&QIot_busInfo.recvTransData, &recvBuf->head);
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_RECV, QIOT_RECV_SUCC_TRANS, NULL, 0);
            }
            else if (QIOT_RECV_SUCC_PHYMODEL_RECV == recvBuf->type && Quos_twllHeadGetNodeCount(QIot_busInfo.recvPhymodelData) < QIOT_RECV_NODE_MODEL_MAX)
            {
                if(QIot_busInfo.dataFormat)
                {
                    void *ttlvHead = Ql_iotTtlvUnformat(recvBuf->val.buf, recvBuf->val.len);
                    if (NULL == ttlvHead)
                    {
                        Quos_logPrintf(QUEC_BUS, LL_ERR, "not ttlv format");
                        return;
                    }
                    cJSON *root = Ql_iotTtlv2Json(ttlvHead);
                    Ql_iotTtlvFree(&ttlvHead);
                    if (NULL == root)
                    {
                        Quos_logPrintf(QUEC_BUS, LL_ERR, "ttlv to json error");
                        return;
                    }
                    char *jsonData = cJSON_PrintUnformatted(root);
                    int jsonLen = HAL_STRLEN(jsonData);
                    QIot_buffer_t *jsonBuf = HAL_MALLOC(sizeof(QIot_buffer_t) + jsonLen);
                    if (NULL == jsonBuf)
                    {
                        Quos_logPrintf(QUEC_BUS, LL_ERR, "mcf jsonBuf");
                        cJSON_Delete(root);
                        return;
                    }
                    HAL_MEMCPY(jsonBuf->val.buf, jsonData, jsonLen);
                    jsonBuf->val.len = jsonLen;
                    jsonBuf->type = QIOT_RECV_SUCC_PHYMODEL_RECV;
                    cJSON_Delete(root);
                    HAL_FREE(recvBuf);
                    Quos_twllHeadAdd(&QIot_busInfo.recvPhymodelData, &jsonBuf->head);
                }
                else
                {
                    Quos_twllHeadAdd(&QIot_busInfo.recvPhymodelData, &recvBuf->head);
                }
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_RECV, QIOT_RECV_SUCC_PHYMODEL_RECV, NULL, 0);
            }
            else
            {
                HAL_FREE(recvBuf);
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_RECV, QIOT_RECV_ERR_LIMIT, NULL, 0);
            }
        }
        else
        #endif
        {
            if (QIOT_RECV_SUCC_TRANS == recvBuf->type)
            {
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_RECV, QIOT_RECV_SUCC_TRANS, recvBuf->val.buf, recvBuf->val.len);
            }
            else if (QIOT_RECV_SUCC_PHYMODEL_RECV == recvBuf->type)
            {
                #ifdef QUEC_ENABLE_AT
                Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_RECV, QIOT_RECV_SUCC_PHYMODEL_RECV, recvBuf->val.buf, recvBuf->val.len);
                #else
                void *ttlv = Ql_iotTtlvUnformat(recvBuf->val.buf, recvBuf->val.len);
                if (ttlv)
                {
                    Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_RECV, QIOT_RECV_SUCC_PHYMODEL_RECV, ttlv, 0);
                    Ql_iotTtlvFree(&ttlv);
                }
                #endif
            }
            HAL_FREE(recvBuf);
        }
        break;
    }
    default:
        break;
    }
}
/**************************************************************************
** ����	@brief : ҵ�������ʼ��
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdBusInit(void)
{
    HAL_MEMSET(&QIot_busInfo, 0, sizeof(QIot_busInfo));
    qint32_t event[] = {QIOT_ATEVENT_TYPE_RECV};
    Quos_eventCbReg(event, sizeof(event) / sizeof(event[0]), ql_iotCmdBusEventNotify);
}