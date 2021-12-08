/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "Ql_iotApi.h"
#include "ql_iotSecure.h"
#include "Qhal_driver.h"
#include "ql_iotDp.h"
void *coapFd = NULL;
quint8_t token19_0_0[8];

#define DEV_UUID "868626047808496"
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void CoapSocketRecvNodeCb(void *chlFd, const void *sendData, const void *recvData)
{
    printf("CoapSocketRecvNodeCb recvData:%p\n", recvData);
    if (recvData)
    {
        Coap_Message_t *msg = (Coap_Message_t *)recvData;
        printf("code:%s\n", COAP_HEAD_CODE_STRING(msg->head.code));
    }
    UNUSED(chlFd);
    UNUSED(sendData);
}
/**************************************************************************
** 功能	@brief : coap协议测试
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool CoapAuthRecvHandle(void *chlFd, const Coap_Message_t *coapMsg, Coap_Message_t *retCoapMsg)
{
    UNUSED(chlFd);
    UNUSED(coapMsg);
    printf("CoapAuthRecvHandle coapMsg:%p\n", coapMsg);
    if (COAP_HCODE_DELETE == coapMsg->head.code)
    {
        printf("CoapRecvNotify:%s\n", COAP_HEAD_CODE_STRING(COAP_HCODE_DELETE));
        Quos_coapHeadSet(retCoapMsg, COAP_HTYPE_ACK, COAP_HCODE_DELETED_202, coapMsg->head.mid, coapMsg->head.tokenLen, coapMsg->token);
        return TRUE;
    }
    else if (COAP_HCODE_PUT == coapMsg->head.code)
    {
        char *uri = Quos_coapOptionGetPath(coapMsg);
        printf("CoapRecvNotify:%s url[%s]\n", COAP_HEAD_CODE_STRING(COAP_HCODE_PUT), uri);
        HAL_FREE(uri);
        Quos_coapHeadSet(retCoapMsg, COAP_HTYPE_ACK, COAP_HCODE_CHANGED_204, coapMsg->head.mid, coapMsg->head.tokenLen, coapMsg->token);
        return TRUE;
    }
    else if (COAP_HCODE_POST == coapMsg->head.code)
    {
        printf("CoapRecvNotify:%s\n", COAP_HEAD_CODE_STRING(COAP_HCODE_POST));
        Quos_coapHeadSet(retCoapMsg, COAP_HTYPE_ACK, COAP_HCODE_EMPTY, coapMsg->head.mid, 0, NULL);
        Coap_Message_t bsEndMsg;
        HAL_MEMSET(&bsEndMsg, 0, sizeof(Coap_Message_t));
        Quos_coapHeadSet(&bsEndMsg, COAP_HTYPE_CON, COAP_HCODE_CHANGED_204, 0, coapMsg->head.tokenLen, coapMsg->token);
        Quos_coapMsgSend(chlFd, NULL, &bsEndMsg, CoapSocketRecvNodeCb, FALSE);
        return TRUE;
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : coap协议测试
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool CoapConnRecvHandle(void *chlFd, const Coap_Message_t *coapMsg, Coap_Message_t *retCoapMsg)
{
    UNUSED(chlFd);
    UNUSED(coapMsg);
    printf("CoapConnRecvHandle coapMsg:%p\n", coapMsg);

    if (COAP_HCODE_GET == coapMsg->head.code)
    {
        char *path = Quos_coapOptionGetPath(coapMsg);
        printf("path[%s]\n", path);
        if (0 == HAL_STRCMP(path, "/19/0/0"))
        {
            HAL_MEMCPY(token19_0_0, coapMsg->token, coapMsg->head.tokenLen);
            printf("save observe(19/0/0) token\n");
        }
        Quos_coapHeadSet(retCoapMsg, COAP_HTYPE_ACK, COAP_HCODE_CONTENT_205, coapMsg->head.mid, coapMsg->head.tokenLen, coapMsg->token);
        return TRUE;
    }
    else if (COAP_HCODE_PUT == coapMsg->head.code)
    {
        char *path = Quos_coapOptionGetPath(coapMsg);
        printf("path[%s]\n", path);
        if (0 == HAL_STRCMP(path, "/19/1/0"))
        {
            printf("recv bus data\n");
            Quos_logHexDumpData(coapMsg->payload.val, coapMsg->payload.len);
            Quos_coapHeadSet(retCoapMsg, COAP_HTYPE_ACK, COAP_HCODE_CHANGED_204, coapMsg->head.mid, coapMsg->head.tokenLen, coapMsg->token);
            return TRUE;
        }
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
extern quint32_t ql_iotDpFormat(quint8_t **buf, quint16_t pId, quint16_t cmd, const quint8_t *payload, quint32_t payloadLen);
void CoapTestDataNotify(void)
{
    static quint16_t id = 0;
    char *srcData = "hello world,quecthing";

    Coap_Message_t msg;
    HAL_MEMSET(&msg, 0, sizeof(Coap_Message_t));
    Quos_coapHeadSet(&msg, COAP_HTYPE_NON, COAP_HCODE_CONTENT_205, 0, 8, token19_0_0);
    Quos_coapOptionSetNumber(&msg, COAP_OTYPE_OBSERVE, 0);
    Quos_coapOptionSetNumber(&msg, COAP_OTYPE_CONTENT_TYPE, COAP_OCTYPE_APP_OCTET_STREAM);
    quint8_t *pkg = NULL;
    quint32_t pkgLen = ql_iotDpFormat(&pkg, ++id, 0x0024, (quint8_t *)srcData, HAL_STRLEN(srcData));
    if (pkg)
    {
        Quos_coapPayloadSet(&msg, pkg, pkgLen);
        Quos_coapMsgSend(coapFd, NULL, &msg, NULL, FALSE);
    }
}
/**************************************************************************
** 功能	@brief : 程序初始化入口
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
int main(void)
{
    /* 初始化quecsdk */
    Ql_iotInit();

#if 1
    if (FALSE == Quos_coapInit(&coapFd, "220.180.239.212:8258", CoapAuthRecvHandle))
    {
        printf("Quos_coapInit fail\n");
        return -1;
    }

    Coap_Message_t coapMsg;
    Quos_coapHeadSet(&coapMsg, COAP_HTYPE_CON, COAP_HCODE_POST, 0, 8, NULL);
    char *payload = Ql_iotSecureGenCoapAuthPayload("p1145q", "WjJRSEp1c2x1ckU2", DEV_UUID);
    Quos_coapPayloadSet(&coapMsg, payload, HAL_STRLEN(payload));
    Quos_coapMsgSend(coapFd, "/bs", &coapMsg, CoapSocketRecvNodeCb, FALSE);
#elif 0
    if (FALSE == Quos_coapInit(&coapFd, "220.180.239.212:8257", CoapConnRecvHandle))
    {
        printf("Quos_coapInit fail\n");
        return -1;
    }

    char *data = "LZ3oAdAUpizTf0YXID7XWMeYYuNPgqNxByo/ISFiJRQCuV9om2ApZwE4ZLEIfQe7";
    char *ds = Ql_iotSecureDecodeDs("WjJRSEp1c2x1ckU2", (quint8_t *)data, HAL_STRLEN(data));
    printf("ds:%s\n", ds);
    char *ep = Ql_iotSecureGenCoapConnEndpoint("p1145q", "WjJRSEp1c2x1ckU2", DEV_UUID, ds);
    HAL_FREE(ds);
    printf("ep:%s\n", ep);
    char *option = HAL_MALLOC(HAL_STRLEN("rd?b=U&lwm2m=1.0&lt=300000&ep=") + HAL_STRLEN(ep) + 1);
    HAL_SPRINTF(option, "%s%s", "rd?b=U&lwm2m=1.0&lt=300000&ep=", ep);
    HAL_FREE(ep);
    Coap_Message_t coapMsg;
    Quos_coapHeadSet(&coapMsg, COAP_HTYPE_CON, COAP_HCODE_POST, 0, 8, NULL);
    Quos_coapOptionSetPath(&coapMsg, option);
    Quos_coapPayloadSet(&coapMsg, NULL, 0);
    Quos_coapMsgSend(coapFd, NULL, &coapMsg, CoapSocketRecvNodeCb, FALSE);
#else
    if (FALSE == Quos_coapInit(&coapFd, "221.229.214.202:5683", CoapConnRecvHandle))
    {
        printf("Quos_coapInit fail\n");
        return -1;
    }

    char *payload = HAL_MALLOC(56);
    HAL_STRCPY(payload, "</>;rt=\"oma.lwm2m\",</1/0>,</3/0>,</4/0>,</5/0>,</19/0>");
    char *option = HAL_MALLOC(HAL_STRLEN("rd?b=U&lwm2m=1.0&lt=86400&ep=") + HAL_STRLEN(DEV_UUID) + 1);
    HAL_SPRINTF(option, "%s%s", "rd?b=U&lwm2m=1.0&lt=300000&ep=", DEV_UUID);
    Coap_Message_t coapMsg;
    Quos_coapHeadSet(&coapMsg, COAP_HTYPE_CON, COAP_HCODE_POST, 0, 8, NULL);
    Quos_coapOptionSetPath(&coapMsg, option);
    Quos_coapPayloadSet(&coapMsg, payload, HAL_STRLEN(payload));
    Quos_coapMsgSend(coapFd, NULL, &coapMsg, CoapSocketRecvNodeCb, FALSE);
#endif

    while (1)
    {
        QIot_state_e status = Ql_iotGetWorkState();
        printf("work status:%d\r\n", status);
        sleep(10);
        CoapTestDataNotify();
    }
}
