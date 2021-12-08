/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : base64加解密
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_base64.h"
#include "Quos_kernel.h"
#if (SDK_ENABLE_BASE64 == 1)
static char baseCode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**************************************************************************
** 功能	@brief : base64加密
** 输入	@param : dstData:加密后数据，长度必须为原数据长度的4/3+2
** 输出	@retval:
***************************************************************************/
quint16_t FUNCTION_ATTR_ROM Quos_base64Encrypt(const quint8_t *srcData, quint16_t srcLen, quint8_t *dstData)
{
    quint16_t i = 0;
    Quos_logHexDump(LSDK_ENCRP, LL_DUMP, "base64 encrypt src data", srcData, srcLen);
    while (srcLen)
    {
        dstData[i++] = baseCode[(srcData[0] >> 2) & 0x3F];
        if (srcLen > 2)
        {
            dstData[i++] = baseCode[((srcData[0] & 0x03) << 4) | (srcData[1] >> 4)];
            dstData[i++] = baseCode[((srcData[1] & 0x0F) << 2) | (srcData[2] >> 6)];
            dstData[i++] = baseCode[srcData[2] & 0x3F];
            srcLen -= 3;
        }
        else if (1 == srcLen)
        {
            dstData[i++] = baseCode[(srcData[0] & 3) << 4];
            dstData[i++] = '=';
            dstData[i++] = '=';
            srcLen = 0;
        }
        else if (2 == srcLen)
        {
            dstData[i++] = baseCode[((srcData[0] & 3) << 4) | (srcData[1] >> 4)];
            dstData[i++] = baseCode[(srcData[1] & 0x0F) << 2];
            dstData[i++] = '=';
            srcLen = 0;
        }
        srcData += 3;
    }
    dstData[i] = 0;
    Quos_logPrintf(LSDK_ENCRP, LL_DBG, "base64 encrypt dst data:%s", dstData);
    return i;
}
/**************************************************************************
** 功能	@brief : base64解密
** 输入	@param : 
** 输出	@retval:
***************************************************************************/
quint16_t FUNCTION_ATTR_ROM Quos_base64Decrypt(const quint8_t *srcData, quint16_t srcLen, quint8_t *dstData)
{
    quint16_t i;
    quint16_t dstLen = 0;
    Quos_logPrintf(LSDK_ENCRP, LL_DBG, "base64 decrypt src data:%s", srcData);
    if (NULL == srcData || NULL == dstData || 0 == srcLen)
    {
        return 0;
    }
    for (i = 0; i < srcLen; i += 4)
    {
        quint8_t k;
        quint8_t baseData[4];
        HAL_MEMSET(baseData, 0, sizeof(baseData));
        for (k = 0; k < 64; k++)
        {
            if (srcData[i] == baseCode[k])
            {
                baseData[0] = k;
                break;
            }
        }
        for (k = 0; k < 64; k++)
        {
            if (srcData[i + 1] == baseCode[k])
            {
                baseData[1] = k;
                break;
            }
        }
        for (k = 0; k < 64; k++)
        {
            if (srcData[i + 2] == baseCode[k])
            {
                baseData[2] = k;
                break;
            }
        }
        for (k = 0; k < 64; k++)
        {
            if (srcData[i + 3] == baseCode[k])
            {
                baseData[3] = k;
                break;
            }
        }
        dstData[dstLen++] = ((baseData[0] << 2) & 0xFC) | ((baseData[1] >> 4) & 0x03);
        if (srcData[i + 2] == '=')
            break;
        dstData[dstLen++] = ((baseData[1] << 4) & 0xF0) | ((baseData[2] >> 2) & 0x0F);
        if (srcData[i + 3] == '=')
            break;
        dstData[dstLen++] = ((baseData[2] << 6) & 0xF0) | (baseData[3] & 0x3F);
    }
    Quos_logHexDump(LSDK_ENCRP, LL_DUMP, "base64 decrypt dst data", dstData, dstLen);
    return dstLen;
}
#endif