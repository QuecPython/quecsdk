#include "ql_iotDp.h"
#include "ql_iotTtlv.h"
#include "ql_iotConfig.h"
#include "ql_iotCmdBus.h"
#include "ql_iotCmdOTA.h"
#include "ql_iotCmdSys.h"
#include "ql_iotCmdLan.h"
#include "ql_iotCmdLoc.h"
#include "ql_iotConn.h"
#ifdef QUEC_ENABLE_GATEWAY
#include "ql_iotGwDev.h"
#endif

/* Э������ */
#define DP_POS_VER_1 0
#define DP_POS_VER_2 1
#define DP_POS_LEN_1 2
#define DP_POS_LEN_2 3
#define DP_POS_SUM 4
#define DP_POS_PID_1 5
#define DP_POS_PID_2 6
#define DP_POS_CMD_1 7
#define DP_POS_CMD_2 8
#define DP_POS_DATA 9

/* Э��汾 */
#define DP_VER_HEADER 0XAA  /* Э��汾�̶�ͷ�� */
#define DP_VER_CURRENT 0XAA /* ��ǰЭ��汾 */
#define DP_VER_ESC 0X55     /* Э��汾ת����� */

QIot_cmdTable_t QIot_cmdTableLocal[] =
    {
        {.cmd = QIOT_DPCMD_TSL_REQ, .cmdHandle = Ql_iotCmdBusPhymodelReqRecv},     /* ��ģ��״̬��ȡ */
        {.cmd = QIOT_DPCMD_TSL_WRITE, .cmdHandle = Ql_iotCmdBusPhymodelWriteRecv}, /* ��ģ�������·� */
        {.cmd = QIOT_DPCMD_PASS_WRITE, .cmdHandle = Ql_iotCmdBusPassTransRecv},    /* ͸�������·� */
        {.cmd = QIOT_DPCMD_LOC_REQ, .cmdHandle = Ql_iotCmdLocDataReqRecv},         /* ��ȡʵʱ��λ��Ϣ */
        {.cmd = QIOT_DPCMD_OTA_NOTIFY, .cmdHandle = Ql_iotCmdOtaNotify},           /* OTA��������֪ͨ */
        {.cmd = QIOT_DPCMD_OTA_FW_INFO, .cmdHandle = Ql_iotCmdOtaFwInfo},          /* �̼���Ϣ�·� */
        {.cmd = QIOT_DPCMD_STATUS_REQ, .cmdHandle = Ql_iotCmdSysStatusRecv},       /* �豸״̬��ȡ */
        {.cmd = QIOT_DPCMD_INFO_REQ, .cmdHandle = Ql_iotCmdSysDevInfoRecv},        /* ģ����Ϣ��ȡ */
        {.cmd = QIOT_DPCMD_EXCE_WRITE, .cmdHandle = Ql_iotCmdSysExceWrite},        /* ƽ̨�쳣֪ͨ���豸 */
        {.cmd = QIOT_DPCMD_DEV_CONFIG_WRITE, .cmdHandle = Ql_iotCmdSysTerManage},  /* �豸�������� */
#ifdef QUEC_ENABLE_GATEWAY
        {.cmd = QIOT_DPCMD_SUB_AUTH_RSP, .cmdHandle = Ql_gatewayDeviceSubAuthResponse},           /* ���豸��֤--ƽ̨�ظ� */
        {.cmd = QIOT_DPCMD_SUB_LOGIN_RSP, .cmdHandle = Ql_gatewayDeviceSubLoginResponse},         /* ���豸��½--ƽ̨�ظ� */
        {.cmd = QIOT_DPCMD_SUB_UNAUTH_EVENT_RSP, .cmdHandle = Ql_gatewayDeviceSubUnauthResponse}, /* ���豸ע��--ƽ̨�ظ� */
        {.cmd = QIOT_DPCMD_SUB_LOGOUT_RSP, .cmdHandle = Ql_gatewayDeviceSubLogoutResponse},       /* ���豸�ǳ�ƽ̨��Ӧ */
        {.cmd = QIOT_DPCMD_SUB_OFFLINE_EVENT, .cmdHandle = Ql_gatewayDeviceSubOfflineEvent},      /* ���豸�����¼� */
#endif
};
#ifdef QUEC_ENABLE_LAN
QIot_cmdTable_t QIot_cmdTableLan[] =
    {
        {.cmd = QIOT_DPCMD_LAN_DISCOVER_REQ, .cmdHandle = ql_iotLanDevDiscover}, /* ���־��������豸 */
                                                                                 //{.cmd = QIOT_DPCMD_LAN_AUTH_COND_REQ, .cmdHandle = NULL}, /* ��������֤ǰ���������� */
                                                                                 //{.cmd = QIOT_DPCMD_LAN_AUTH_SIGN_REQ, .cmdHandle = NULL}, /* ��������֤ǩ������ */
};
#endif
#ifdef QUEC_ENABLE_GATEWAY
QIot_cmdTable_t QIot_cmdTableSubDev[] =
    {
        {.cmd = QIOT_DPCMD_TSL_REQ, .cmdHandle = Ql_gatewayDeviceSubTslDataRead},    /* ��ģ��״̬��ȡ */
        {.cmd = QIOT_DPCMD_TSL_WRITE, .cmdHandle = Ql_gatewayDeviceSubTslDataWrite}, /* ��ģ�������·� */
        {.cmd = QIOT_DPCMD_PASS_WRITE, .cmdHandle = Ql_gatewayDeviceSubPassTransData},    /* ͸�������·� */
        //{.cmd = QIOT_DPCMD_LOC_REQ, .cmdHandle = NULL},    /* ��ȡʵʱ��λ��Ϣ */
        {.cmd = QIOT_DPCMD_STATUS_REQ, .cmdHandle = Ql_gatewayDeviceSubStateRead}, /* �豸״̬��ȡ */
        {.cmd = QIOT_DPCMD_INFO_REQ, .cmdHandle = Ql_gatewayDeviceSubInfoRead},    /* ģ����Ϣ��ȡ */

        //{.cmd = QIOT_DPCMD_STATUS_EVENT, .cmdHandle = Ql_gatewayDeviceSubStateReport}, /* ���豸״̬�ϱ� */
        //{.cmd = QIOT_DPCMD_INFO_EVENT, .cmdHandle = Ql_gatewayDeviceSubInfoReport},   /* ���豸ģ����Ϣ�ϱ� */
        //{.cmd = QIOT_GATEWAY_SUB_FIND_GATEWAY, .cmdHandle = Ql_gatewayDeviceSubReciveBroadMsg},   /* ���豸�㲥�������� */
        //{.cmd = QIOT_GATEWAY_SUB_CONN_GATEWAY, .cmdHandle = Ql_gatewayDeviceSubConnGatwayMsg},   /* ���豸�������� */
        //{.cmd = QIOT_GATEWAY_SUB_AUTH, .cmdHandle = Ql_gatewayDeviceSubAuthMsg},   /* ���豸������֤ */
        //{.cmd = QIOT_GATEWAY_SUB_LOGIN, .cmdHandle = Ql_gatewayDeviceSubConnMsg},   /* ���豸�����¼ƽ̨ */
        //{.cmd = QIOT_GATEWAY_SUB_LOGOUT_EVENT, .cmdHandle = Ql_gatewayDeviceSubDisconnMsg},   /* ���豸�������� */
        //{.cmd = QIOT_GATEWAY_SUB_UNAUTH_EVENT, .cmdHandle = Ql_gatewayDeviceSubUnauthMsg},   /* ���豸����ע�� */
        //{.cmd = QIOT_DPCMD_PASS_EVENT, .cmdHandle = Ql_gatewayDeviceSubPassTransReport},   /* ���豸�ϴ�͸������ */
        //{.cmd = QIOT_DPCMD_TSL_EVENT, .cmdHandle = Ql_gatewayDeviceSubPhymodelReport},   /* ���豸�ϱ���ģ������ */
        //{.cmd = QIOT_GATEWAY_SUB_READ_STATE, .cmdHandle = Ql_gatewayDeviceStateQuery},  /* ���豸��ѯ����״̬ */

};
#endif
QIot_dpAppSend_t QIot_dpAppSend[] =
    {
        {.app = QIOT_DPAPP_M2M, .send = Ql_iotConnSend},
#ifdef QUEC_ENABLE_GATEWAY
//{.app = QIOT_DPAPP_SUBDEV, .send = Ql_iotConnSubSendUart},
#endif
};
/**************************************************************************
** ����	@brief : �ϲ����ݰ���������0x55ת��
** ����	@param : 
** ���	@retval: ������ݳ���
** ��ע	@remark: ����������Ҫ��Ϊ�˼���malloc����
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Ql_iotDpFormat(quint8_t **buf, quint16_t pId, quint16_t cmd, const quint8_t *payload, quint32_t payloadLen)
{
    quint8_t head[DP_POS_DATA];
    head[DP_POS_VER_1] = DP_VER_HEADER;
    head[DP_POS_VER_2] = DP_VER_CURRENT;
    _U16_ARRAY01(payloadLen + DP_POS_DATA - DP_POS_SUM, &head[DP_POS_LEN_1]);
    _U16_ARRAY01(pId, &head[DP_POS_PID_1]);
    _U16_ARRAY01(cmd, &head[DP_POS_CMD_1]);
    head[DP_POS_SUM] = (quint8_t)Quos_crcCalculate(0, &head[DP_POS_PID_1], DP_POS_DATA - DP_POS_PID_1);
    head[DP_POS_SUM] = (quint8_t)Quos_crcCalculate(head[DP_POS_SUM], payload, payloadLen);
    quint32_t escCount = DP_POS_DATA;
    quint32_t i;
    for (i = DP_POS_VER_2; i < DP_POS_CMD_2; i++)
    {
        if (DP_VER_HEADER == head[i] && (DP_VER_CURRENT == head[i + 1] || DP_VER_ESC == head[i + 1]))
        {
            escCount++;
        }
    }
    if (DP_VER_HEADER == head[DP_POS_CMD_2] && (0 == payloadLen || DP_VER_ESC == payload[0]))
    {
        escCount++;
    }
    escCount += payloadLen;
    if (payloadLen > 0)
    {
        for (i = 0; i < payloadLen - 1; i++)
        {
            if (DP_VER_HEADER == payload[i] && (DP_VER_CURRENT == payload[i + 1] || DP_VER_ESC == payload[i + 1]))
            {
                escCount++;
            }
        }
        if (DP_VER_HEADER == payload[payloadLen - 1])
        {
            escCount++;
        }
    }

    quint8_t *pkgBuf = HAL_MALLOC(escCount);
    if (NULL == pkgBuf)
    {
        return 0;
    }
    escCount = 0;
    pkgBuf[escCount++] = DP_VER_HEADER;
    for (i = DP_POS_VER_2; i < DP_POS_CMD_2; i++)
    {
        pkgBuf[escCount++] = head[i];
        if (DP_VER_HEADER == head[i] && (DP_VER_CURRENT == head[i + 1] || DP_VER_ESC == head[i + 1]))
        {
            pkgBuf[escCount++] = DP_VER_ESC;
        }
    }
    pkgBuf[escCount++] = head[DP_POS_CMD_2];
    if (DP_VER_HEADER == head[DP_POS_CMD_2] && (0 == payloadLen || DP_VER_ESC == pkgBuf[0]))
    {
        pkgBuf[escCount++] = DP_VER_ESC;
    }
    if (payloadLen > 0)
    {
        for (i = 0; i < payloadLen - 1; i++)
        {
            pkgBuf[escCount++] = payload[i];
            if (DP_VER_HEADER == payload[i] && (DP_VER_CURRENT == payload[i + 1] || DP_VER_ESC == payload[i + 1]))
            {
                pkgBuf[escCount++] = DP_VER_ESC;
            }
        }
        pkgBuf[escCount++] = payload[payloadLen - 1];
        if (DP_VER_HEADER == payload[payloadLen - 1])
        {
            pkgBuf[escCount++] = DP_VER_ESC;
        }
    }
    *buf = pkgBuf;
    Quos_logHexDump(QUEC_DP, LL_DUMP, "send pkg", pkgBuf, escCount);
    return escCount;
}
/**************************************************************************
** ����	@brief : ��DP������ת��DP�ṹ��
** ����	@param : 
** ���	@retval: 
***************************************************************************/
QIot_dpPackage_t FUNCTION_ATTR_ROM Ql_iotDpRaw2Package(const quint8_t *buf, quint32_t len)
{
    QIot_dpPackage_t pkg;
    pkg.head = _ARRAY01_U16(&buf[DP_POS_VER_1]);
    pkg.sum = buf[DP_POS_SUM];
    pkg.pkgId = _ARRAY01_U16(&buf[DP_POS_PID_1]);
    pkg.cmd = _ARRAY01_U16(&buf[DP_POS_CMD_1]);
    pkg.payloadLen = len - DP_POS_DATA;
    if (pkg.payloadLen > 0)
    {
        pkg.payload = (quint8_t *)&buf[DP_POS_DATA];
    }
    else
    {
        pkg.payload = NULL;
    }

    return pkg;
}
/**************************************************************************
** ����	@brief : ��ԭʼ�������ݰ�����ȡЭ���,������buf���ݿ��ܻ�ı�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotDpRawDataPickup(quint8_t *buf, quint32_t len, QIot_dpPackage_t *pkg)
{
    if (len < DP_POS_DATA || DP_VER_HEADER != buf[0] || DP_VER_CURRENT != buf[1])
    {
        return FALSE;
    }
    quint16_t pkgLen = 0, offset = DP_POS_LEN_1;
    quint32_t i;
    for (i = offset; i < len; i++)
    {
        if (DP_VER_HEADER != buf[i - 1] || DP_VER_ESC != buf[i])
        {
            buf[offset++] = buf[i];
        }
        if (DP_POS_SUM == offset)
        {
            pkgLen = _ARRAY01_U16(&buf[DP_POS_LEN_1]);
        }
        else if (pkgLen + DP_POS_SUM == offset)
        {
            quint16_t pkgId = _ARRAY01_U16(&buf[DP_POS_PID_1]);
            if (0x0000 == pkgId || 0xFFFF == pkgId)
            {
                quint16_t cmd = _ARRAY01_U16(&buf[DP_POS_CMD_1]);
                Quos_logPrintf(QUEC_DP, LL_ERR, "pkgid[0x%04X] cmd[0x%04X]", pkgId, cmd);
            }
            else if ((quint8_t)Quos_crcCalculate(0, buf + DP_POS_PID_1, offset - DP_POS_PID_1) == buf[DP_POS_SUM])
            {
                Quos_logHexDump(QUEC_DP, LL_DUMP, "packet", buf, offset);
                if (pkg)
                {
                    *pkg = Ql_iotDpRaw2Package(buf, offset);
                }
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}
/**************************************************************************
** ����	@brief : ����������Ƿ���֧���б�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
QIot_dpCmdHandle_f FUNCTION_ATTR_ROM Ql_iotDpCmdIn(quint16_t cmd, QIot_cmdTable_t table[], quint32_t size)
{
    quint32_t i;
    for (i = 0; i < size; i++)
    {
        if (cmd == table[i].cmd)
        {
            return table[i].cmdHandle;
        }
    }
    return NULL;
}
/**************************************************************************
** ����	@brief : ���ݽ��մ���
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotDpHandle(QIot_dpAppType_e app, const char *endPoint, quint8_t *payload, quint32_t payloadLen)
{
    Quos_logPrintf(QUEC_DP, LL_DBG, "app:%d ep:%s", app, endPoint);
    char clientId[QIOT_M2M_CLIENTID_MAXSIZE + 1];
    HAL_SPRINTF(clientId, "qd%s%s", QIot_userdata.connectProduct->productKey, QIot_userdata.deviceInfo.deviceKey);
    QIot_dpCmdHandle_f cmdHandle = NULL;
    QIot_dpPackage_t dpPkg;

    if (NULL == payload || FALSE == Ql_iotDpRawDataPickup(payload, payloadLen, &dpPkg))
    {
        Quos_logPrintf(QUEC_CONN, LL_ERR, "data pickup fail: %.*s", payloadLen, payload);
        return;
    }
    if (NULL == endPoint || 0 == HAL_STRNCMP(endPoint, clientId, HAL_STRLEN(clientId))) // TODO
    {
        cmdHandle = Ql_iotDpCmdIn(dpPkg.cmd, QIot_cmdTableLocal, sizeof(QIot_cmdTableLocal) / sizeof(QIot_cmdTableLocal[0]));
    }
#ifdef QUEC_ENABLE_LAN
    else if (QIOT_DPAPP_LANPHONE == app)
    {
        cmdHandle = Ql_iotDpCmdIn(dpPkg.cmd, QIot_cmdTableLan, sizeof(QIot_cmdTableLan) / sizeof(QIot_cmdTableLan[0]));
    }
#endif
#ifdef QUEC_ENABLE_GATEWAY
    else
    {
        cmdHandle = Ql_iotDpCmdIn(dpPkg.cmd, QIot_cmdTableSubDev, sizeof(QIot_cmdTableSubDev) / sizeof(QIot_cmdTableSubDev[0]));
    }
#endif
    if (cmdHandle)
    {
        cmdHandle(app, endPoint, dpPkg.pkgId, dpPkg.payload, dpPkg.payloadLen);
    }
}

/**************************************************************************
** ����	@brief : ����pkgid
** ����	@param : 
** ���	@retval: 
***************************************************************************/
quint16_t FUNCTION_ATTR_ROM Ql_iotDpPkgIdUpdate(quint16_t *pkgId)
{
    (*pkgId)++;
    if (0 == (*pkgId) || 0xFFFF == (*pkgId))
    {
        *pkgId = 1;
    }
    return *pkgId;
}

/**************************************************************************
** ����	@brief : ����ͨ����������
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM ql_iotDpSendCommon(QIot_dpAppType_e app, const char *endPoint, quint16_t mode, quint16_t cmd, quint16_t srcPkgId, quint16_t pkgId, const quint8_t *payload, quint32_t payloadLen, socketRecvNodeCb_f recvCB)
{
    UNUSED(srcPkgId);
    qbool ret = FALSE;
    quint32_t i;
    for (i = 0; i < sizeof(QIot_dpAppSend) / sizeof(QIot_dpAppSend[0]); i++)
    {
        if (app & QIot_dpAppSend[i].app)
        {
            if ((QIot_dpAppSend[i].send)(endPoint, mode, cmd, srcPkgId, pkgId, payload, payloadLen, recvCB))
            {
                ret = TRUE;
            }
        }
    }
    return ret;
}
/**************************************************************************
** ����	@brief : ����TTLV����
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM ql_iotDpSendTtlv(QIot_dpAppType_e app, const char *endPoint, quint16_t mode, quint16_t cmd, quint16_t srcPkgId, quint16_t pkgId, const void *ttlvHead, socketRecvNodeCb_f recvCB)
{
    qbool ret = FALSE;
    quint32_t len = Ql_iotTtlvFormatLen(ttlvHead);
    quint8_t *buf = NULL;
    if (len)
    {
        if ((buf = HAL_MALLOC(len)) == NULL)
        {
            return FALSE;
        }
        len = Ql_iotTtlvFormat(ttlvHead, buf);
    }
    ret = ql_iotDpSendCommon(app, endPoint, mode, cmd, srcPkgId, pkgId, buf, len, recvCB);
    if (buf)
    {
        HAL_FREE(buf);
    }
    return ret;
}
/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotDpSendCommonReq(QIot_dpAppType_e app, const char *endPoint, quint16_t srcPkgId, quint16_t mode, quint16_t cmd, const quint8_t *payload, quint32_t payloadLen, socketRecvNodeCb_f recvCB)
{
    return ql_iotDpSendCommon(app, endPoint, mode, cmd, srcPkgId, 0, payload, payloadLen, recvCB);
}
/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool Ql_iotDpSendCommonRsp(QIot_dpAppType_e app, const char *endPoint, quint16_t cmd, quint16_t pkgId, const quint8_t *payload, quint32_t payloadLen)
{
    return ql_iotDpSendCommon(app, endPoint, 0, cmd, 0, pkgId, payload, payloadLen, NULL);
}

/**************************************************************************
** ����	@brief : ����TTLV��������
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotDpSendTtlvReq(QIot_dpAppType_e app, const char *endPoint, quint16_t srcPkgId, quint16_t mode, quint16_t cmd, const void *ttlvHead, socketRecvNodeCb_f recvCB)
{
    return ql_iotDpSendTtlv(app, endPoint, mode, cmd, srcPkgId, 0, ttlvHead, recvCB);
}
/**************************************************************************
** ����	@brief : ����TTLVӦ������
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotDpSendTtlvRsp(QIot_dpAppType_e app, const char *endPoint, quint16_t cmd, quint16_t pkgId, const void *ttlvHead)
{
    return ql_iotDpSendTtlv(app, endPoint, 0, cmd, 0, pkgId, ttlvHead, NULL);
}
