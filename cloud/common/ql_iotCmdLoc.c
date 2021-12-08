/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 
** 功能   @brief   : 
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "ql_iotCmdLoc.h"
#include "ql_iotCmdSys.h"
#include "ql_iotConfig.h"
#include "ql_iotDp.h"
#include "ql_iotTtlv.h"
#include "Qhal_driver.h"

#define QIOT_LOCATOR_SEPARATOR ";"

/**************************************************************************
** 功能	@brief : 判断组合是否冲突
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotLocatoTitleClashCheck(const void *ttlvHead)
{
    quint32_t titleCnt = 0, titleSum = Ql_iotTtlvCountGet(ttlvHead);
    for (titleCnt = 0; titleCnt < titleSum; titleCnt++)
    {
        char *value = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(ttlvHead, titleCnt, NULL, NULL));
        if(NULL == value)
        {
            Quos_logPrintf(QUEC_LOC, LL_ERR, "title is null");
            return FALSE;
        }
        if (0 == HAL_STRCMP(value, QIOT_LOC_SUPPORT_AUTO) && 1 != titleSum)
        {
            Quos_logPrintf(QUEC_LOC, LL_ERR, "title[%s] nonuniqueness", value);
            return FALSE;
        }
        else
        {
            quint32_t cnt1 = 0;
            for (cnt1 = 0; cnt1 < titleSum; cnt1++)
            {
                char *value1 = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(ttlvHead, cnt1, NULL, NULL));
                if(NULL == value1)
                {
                    Quos_logPrintf(QUEC_LOC, LL_ERR, "title is null");
                    return FALSE;
                }
                if (cnt1 != titleCnt && (HAL_STRSTR(value1, value) || HAL_STRSTR(value, value1)))
                {
                    Quos_logPrintf(QUEC_LOC, LL_ERR, "title[%d][%s] is clash with No[%d][%s]", titleCnt, value, cnt1, value1);
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 判断组合是否都支持
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotLocatoTitleSupportCheck(const void *ttlvHead)
{
    char *words[100];
    quint32_t listCnt = Qhal_propertyLocSupList(words, sizeof(words) / sizeof(words[0]));
    quint32_t titleCnt = 0, titleSum = Ql_iotTtlvCountGet(ttlvHead);
    for (titleCnt = 0; titleCnt < titleSum; titleCnt++)
    {
        char *title = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(ttlvHead, titleCnt, NULL, NULL));
        if (NULL == title)
        {
            Quos_logPrintf(QUEC_LOC, LL_ERR, "title is empty");
            return FALSE;
        }
        quint32_t count;
        for (count = 0; count < listCnt; count++)
        {
            if (0 == HAL_STRCMP(words[count], title))
            {
                break;
            }
        }
        if (count >= listCnt)
        {
            Quos_logPrintf(QUEC_LOC, LL_ERR, "title[%s] is no support", title);
            return FALSE;
        }
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotLocatorTtlv2String(const void *ttlv)
{
    quint32_t sum = Ql_iotTtlvCountGet(ttlv);
    quint32_t cnt;
    quint32_t len = 0;
    for (cnt = 0; cnt < sum; cnt++)
    {
        char *locType = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(ttlv, cnt, NULL, NULL));
        if (locType)
        {
            len += HAL_STRLEN(locType) + HAL_STRLEN(QIOT_LOCATOR_SEPARATOR);
        }
    }
    char *buf;
    if (0 == len || (buf = HAL_MALLOC(len + 1)) == NULL)
    {
        return NULL;
    }

    for (cnt = 0, len = 0; cnt < sum; cnt++)
    {
        char *locType = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(ttlv, cnt, NULL, NULL));
        if (locType)
        {
            len += HAL_SPRINTF(buf + len, "%s%s", locType, QIOT_LOCATOR_SEPARATOR);
        }
    }
    return buf;
}
/**************************************************************************
** 功能	@brief : 计算NMEA语句CRC
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static quint8_t FUNCTION_ATTR_ROM ql_iotLocNmeaCrc(char *str, quint32_t len)
{
    quint8_t crc = 0;
    if (NULL == str)
    {
        return 0;
    }
    while (len--)
    {
        crc ^= *str++;
    }
    return crc;
}
/**************************************************************************
** 功能	@brief : 获取LBS的模拟NMEA语句
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM ql_iotLocatorLbsNmeaRead(void **ttlv)
{
#define IOT_LBSNMEA_FORMAT "$LBS,%u,%02u,%u,%u,%d,0*"
    Qhal_propertyNet_t master, neibh[6];
    quint32_t count = 0;
    if (FALSE == Qhal_propertyNetGet(&master, &neibh, sizeof(neibh) / sizeof(neibh[0]), &count) ||
        count > sizeof(neibh) / sizeof(neibh[0]))
    {
        return FALSE;
    }

    char buf[100];
    quint32_t len = HAL_SPRINTF(buf, IOT_LBSNMEA_FORMAT, master.mcc, master.mnc, master.lac, master.cellid, master.rssi);
    len += HAL_SPRINTF(buf + len, "%02d", ql_iotLocNmeaCrc(buf + 1, len - 2));
    buf[len] = 0;
    Ql_iotTtlvIdAddString(ttlv, 0, buf);
    while (count--)
    {
        quint32_t len = HAL_SPRINTF(buf, IOT_LBSNMEA_FORMAT, neibh[count].mcc, neibh[count].mnc, neibh[count].lac, neibh[count].cellid, neibh[count].rssi);
        len += HAL_SPRINTF(buf + len, "%02d", ql_iotLocNmeaCrc(buf + 1, len - 2));
        buf[len] = 0;
        Ql_iotTtlvIdAddString(ttlv, 0, buf);
    }
    return TRUE;
}

/**************************************************************************
** 功能	@brief : 根据需求获取定位数据
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM *Ql_iotLocGetData(const void *titleTtlv)
{
    void *locDataTtlv = NULL;
    quint32_t titleCnt = 0;
    char *locType;
    if (FALSE == Ql_iotLocatoTitleClashCheck(titleTtlv) || FALSE == Ql_iotLocatoTitleSupportCheck(titleTtlv))
    {
        return NULL;
    }
    while ((locType = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(titleTtlv, titleCnt++, NULL, NULL))) != NULL)
    {
        if (0 == HAL_STRCMP(locType, QIOT_LOC_SUPPORT_AUTO))
        {
            if (FALSE == Qhal_propertyGnssRawDataRead(&locDataTtlv, "GGA"))
            {
                ql_iotLocatorLbsNmeaRead(&locDataTtlv);
            }
        }
        else if (0 == HAL_STRCMP(locType, QIOT_LOC_SUPPORT_LBS))
        {
            ql_iotLocatorLbsNmeaRead(&locDataTtlv);
        }
        else
        {
            Qhal_propertyGnssRawDataRead(&locDataTtlv, locType);
        }
    }
    return locDataTtlv;
}
/**************************************************************************
** 功能	@brief : 定位数据发送结果
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM ql_iotCmdBusLocReportCB(void *chlFd, const void *sendData, const void *recvData)
{
    UNUSED(chlFd);
    UNUSED(sendData);
    Ql_iotUrcEventCB(QIOT_ATEVENT_TYPE_SEND, recvData ? QIOT_SEND_SUCC_LOC : QIOT_SEND_ERR_FAIL_LOC, NULL, 0);
}

/**************************************************************************
** 功能	@brief : 主动上报模组定位数据
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBusLocReportInside(void *titleTtlv)
{
    qbool ret = FALSE;
    void *nmeaTtlv = Ql_iotLocGetData(titleTtlv);
    if (NULL == nmeaTtlv)
    {
        Quos_logPrintf(QUEC_BUS, LL_ERR, "nmeaTtlv is null");
        return FALSE;
    }
    char *buf = Ql_iotLocatorTtlv2String(nmeaTtlv);
    Ql_iotTtlvFree(&nmeaTtlv);

    void *reportTtlv = NULL;
    Ql_iotTtlvIdAddString(&reportTtlv, 2, buf);
    HAL_FREE(buf);
    ret = Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M,NULL, 0, 1, QIOT_DPCMD_LOC_REPORT, reportTtlv, ql_iotCmdBusLocReportCB);
    Ql_iotTtlvFree(&reportTtlv);
    return ret;
}
/**************************************************************************
** 功能	@brief : 主动上报外部定位数据
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Ql_iotCmdBusLocReportOutside(void *nmeaTtlv)
{
    qbool ret = FALSE;
    if (NULL == nmeaTtlv)
    {
        Quos_logPrintf(QUEC_BUS, LL_ERR, "nmeaTtlv is null");
        return FALSE;
    }
    char *buf = Ql_iotLocatorTtlv2String(nmeaTtlv);
    void *reportTtlv = NULL;
    Ql_iotTtlvIdAddString(&reportTtlv, 2, buf);
    HAL_FREE(buf);
    ret = Ql_iotDpSendTtlvReq(QIOT_DPAPP_M2M,NULL, 0, 1, QIOT_DPCMD_LOC_REPORT, reportTtlv, ql_iotCmdBusLocReportCB);
    Ql_iotTtlvFree(&reportTtlv);
    return ret;
}

/**************************************************************************
** 功能	@brief : 应答平台查询的实时内置定位数据
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotCmdLocDataReqRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen)
{
    void *titleTtlv = NULL;
    void *ttlvHead = Ql_iotTtlvUnformat(payload, payloadLen);
    char *cfgData = Ql_iotTtlvIdGetString(ttlvHead, 1);
    if (cfgData)
    {
        char *words[100];
        quint32_t count = Quos_stringSplit(cfgData,HAL_STRLEN(cfgData), words, sizeof(words) / sizeof(words[0]), QIOT_LOCATOR_SEPARATOR, FALSE);
        quint32_t i;
        for (i = 0; i < count; i++)
        {
            Ql_iotTtlvIdAddString(&titleTtlv, 0, words[i]);
        }
    }
    Ql_iotTtlvFree(&ttlvHead);
    if (NULL == titleTtlv || FALSE == Ql_iotLocatoTitleClashCheck(titleTtlv))
    {
        Ql_iotTtlvFree(&titleTtlv);
        Ql_iotCmdSysExceReport(app,endPoint,QIOT_SERVER_ERRCODE_UNFORMAT_FAIL, pkgId);
    }
    else
    {
        void *locDataTtlv = Ql_iotLocGetData(titleTtlv);
        Ql_iotTtlvFree(&titleTtlv);
        char *buf = Ql_iotLocatorTtlv2String(locDataTtlv);
        Ql_iotTtlvFree(&locDataTtlv);

        /* 以下locDataTtlv作用已变 */
        Ql_iotTtlvIdAddString(&locDataTtlv, 2, buf);
        HAL_FREE(buf);
        Ql_iotDpSendTtlvRsp(app,endPoint, QIOT_DPCMD_LOC_RSP, pkgId, locDataTtlv);
        Ql_iotTtlvFree(&locDataTtlv);
    }
}
/**************************************************************************
** 功能	@brief : 查询当前模组支持的内置定位至此类型
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM *Ql_iotLocGetSupList(void)
{
    char *words[100];
    quint32_t count = Qhal_propertyLocSupList(words, sizeof(words) / sizeof(words[0]));
    quint32_t num;
    void *titleTtlv = NULL;
    for (num = 0; num < count; num++)
    {
        Ql_iotTtlvIdAddString(&titleTtlv, 0, words[num]);
    }
    return titleTtlv;
}