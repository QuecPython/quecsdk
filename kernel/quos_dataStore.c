/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 文件或FLASH安全备份保存
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_dataStore.h"
#include "Quos_kernel.h"
#include "Qhal_driver.h"
#if (SDK_ENABLE_DATASAFE == 1)

#define FILE_DSNAME_BAK_HEAD "_BAK"

#ifndef QUOS_FILE_NAME_MAXLENGHT
#define QUOS_FILE_NAME_MAXLENGHT 256 /* 文件名最大长度 */
#endif

typedef struct
{
    quint32_t index;
    quint8_t crc;
    quint16_t len;
    quint8_t buf[1]; /* 数据开始位置 */
} SafeFlashHeadData_t;

/**************************************************************************
** 功能	@brief : 安全写数据
** 输入	@param : bak
                buf:待写入的数据
                bufLen:待写入数据的长度
                aesKey必须是QUOS_AES_KEYLEN字节
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_dsWrite(const char *filename, const void *buf, quint16_t bufLen, const char *aesKey)
{
    Quos_logPrintf(LSDK_STORE, LL_DBG, "filename[%s] bufLen=%u", filename, bufLen);
    Quos_logHexDump(LSDK_STORE, LL_DUMP, "buf", buf, bufLen);

    if (NULL == filename || buf == NULL || 0 == bufLen)
    {
        return FALSE;
    }
    char *fileNew = (char *)filename;
    char filenameBak[QUOS_FILE_NAME_MAXLENGHT + sizeof(FILE_DSNAME_BAK_HEAD)];
    HAL_SPRINTF(filenameBak, "%s", filename);
    char *point = filenameBak + HAL_STRLEN(filenameBak);
    while (point != filenameBak && '.' != *point)
    {
        point--;
    }
    if (point == filenameBak)
    {
        HAL_SPRINTF(filenameBak + HAL_STRLEN(filenameBak), "%s", FILE_DSNAME_BAK_HEAD);
    }
    else
    {
        HAL_MEMMOVE(point + HAL_STRLEN(FILE_DSNAME_BAK_HEAD), point, HAL_STRLEN(FILE_DSNAME_BAK_HEAD) + 1);
        HAL_MEMCPY(point, FILE_DSNAME_BAK_HEAD, HAL_STRLEN(FILE_DSNAME_BAK_HEAD));
    }

    SafeFlashHeadData_t sfHD[2];
    HAL_MEMSET(sfHD, 0, sizeof(sfHD));
    pointer_t fileFd = Qhal_fileOpen(filename, TRUE);
    pointer_t fileBakFd = Qhal_fileOpen(filenameBak, TRUE);
    if (SOCKET_FD_INVALID != fileFd)
    {
        Qhal_fileRead(fileFd, 0, &sfHD[0], __GET_POS_ELEMENT(SafeFlashHeadData_t, buf));
    }
    if (SOCKET_FD_INVALID != fileBakFd)
    {
        Qhal_fileRead(fileBakFd, 0, &sfHD[1], __GET_POS_ELEMENT(SafeFlashHeadData_t, buf));
    }

    Quos_logPrintf(LSDK_STORE, LL_DBG, "filename[%s] Index:%u len:%u crc:0x%02X", filename, sfHD[0].index, sfHD[0].len, sfHD[0].crc);
    Quos_logPrintf(LSDK_STORE, LL_DBG, "filenameBak[%s] Index:%u len:%u crc:0x%02X", filenameBak, sfHD[1].index, sfHD[1].len, sfHD[1].crc);

    if (0xFFFFFFFF == sfHD[0].index || 0xFFFF == sfHD[0].len)
    {
        HAL_MEMSET(&sfHD[0], 0, sizeof(sfHD[0]));
    }
    if (0xFFFFFFFF == sfHD[1].index || 0xFFFF == sfHD[1].len)
    {
        HAL_MEMSET(&sfHD[1], 0, sizeof(sfHD[1]));
    }

    quint32_t maxLen = sfHD[0].len > sfHD[1].len ? sfHD[0].len : sfHD[1].len;
    maxLen = maxLen > (quint32_t)__BYTE_TO_ALIGN(bufLen + 1, QUOS_AES_BLOCKLEN) ? maxLen : (quint32_t)__BYTE_TO_ALIGN(bufLen + 1, QUOS_AES_BLOCKLEN);
    SafeFlashHeadData_t *newSfHD = (SafeFlashHeadData_t *)HAL_MALLOC(__GET_POS_ELEMENT(SafeFlashHeadData_t, buf) + maxLen);
    if (newSfHD)
    {
        if (sfHD[0].index <= sfHD[1].index)
        {
            if (sfHD[1].len == 0 ||
                sfHD[1].len != Qhal_fileRead(fileBakFd, __GET_POS_ELEMENT(SafeFlashHeadData_t, buf), newSfHD->buf, sfHD[1].len) ||
                sfHD[1].crc != (quint8_t)Quos_crcCalculate(0, newSfHD->buf, sfHD[1].len))
            {
                Quos_logPrintf(LSDK_STORE, LL_ERR, "newest data in flash1 is invaild");
                newSfHD->index = sfHD[0].index + 1;
                fileNew = filenameBak;
            }
            else
            {
                newSfHD->index = sfHD[1].index + 1;
            }
        }
        else
        {
            if (sfHD[0].len == 0 ||
                sfHD[0].len != Qhal_fileRead(fileFd, __GET_POS_ELEMENT(SafeFlashHeadData_t, buf), newSfHD->buf, sfHD[0].len) ||
                sfHD[0].crc != (quint8_t)Quos_crcCalculate(0, newSfHD->buf, sfHD[0].len))
            {
                Quos_logPrintf(LSDK_STORE, LL_ERR, "newest data in flash0 is invaild");
                newSfHD->index = sfHD[1].index + 1;
            }
            else
            {
                newSfHD->index = sfHD[0].index + 1;
                fileNew = filenameBak;
            }
        }
    }
    /*else
    {
        Quos_logPrintf(LSDK_STORE, LL_WARN, "malloc sfHD fail so cancel crc old data");
        newSfHD = (SafeFlashHeadData_t *)HAL_MALLOC(__GET_POS_ELEMENT(SafeFlashHeadData_t, buf) + __BYTE_TO_ALIGN(bufLen,QUOS_AES_BLOCKLEN));
        if (NULL == newSfHD)
        {
            Quos_logPrintf(LSDK_STORE, LL_ERR, "mcf sfHD");
            return FALSE;
        }
        if (sfHD[0].index <= sfHD[1].index)
        {
            newSfHD->index = sfHD[1].index + 1;
        }
        else
        {
            newSfHD->index = sfHD[0].index + 1;
            fileNew = filenameBak;
        }
    }*/
    if (SOCKET_FD_INVALID != fileFd)
    {
        Qhal_fileClose(fileFd);
    }
    if (SOCKET_FD_INVALID != fileBakFd)
    {
        Qhal_fileClose(fileBakFd);
    }
    if (NULL == newSfHD)
    {
        return FALSE;
    }

    HAL_MEMCPY(newSfHD->buf, buf, bufLen);
    newSfHD->len = bufLen;
    if (aesKey)
    {
        AES_ctx_t aes_ctx;
        Quos_aesInitCtx(&aes_ctx, aesKey);
        Quos_aesPadding(newSfHD->buf, newSfHD->buf, newSfHD->len);
        newSfHD->len = Quos_aesEcbEncrypt(&aes_ctx, (const quint8_t *)newSfHD->buf, __BYTE_TO_ALIGN(newSfHD->len + 1, QUOS_AES_BLOCKLEN));
    }

    newSfHD->crc = (quint8_t)Quos_crcCalculate(0, newSfHD->buf, newSfHD->len);
    Quos_logPrintf(LSDK_STORE, LL_DBG, "fileNew[%s] Index:%u len:%u crc:0x%02X", fileNew, newSfHD->index, newSfHD->len, newSfHD->crc);
    Qhal_fileErase(fileNew);

    qbool ret = FALSE;
    fileFd = Qhal_fileOpen(fileNew, FALSE);
    if (SOCKET_FD_INVALID != fileFd)
    {
        ret = Qhal_fileWrite(fileFd, 0, newSfHD, __GET_POS_ELEMENT(SafeFlashHeadData_t, buf) + newSfHD->len);
        Qhal_fileClose(fileFd);
    }
    else
    {
        Quos_logPrintf(LSDK_STORE, LL_ERR, "fileNew[%s] open fail", fileNew);
    }

    HAL_FREE(newSfHD);
    return ret;
}
/**************************************************************************
** 功能	@brief : 安全读数据
** 输入	@param : addr：保存数据的起始地址
                   buf:待读取的数据缓冲区
                   bufLen:待读取数据的长度
                   aesKey必须是QUOS_AES_KEYLEN字节
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_dsRead(const char *filename, void **buf, const char *aesKey)
{
    quint32_t ret = 0;
    Quos_logPrintf(LSDK_STORE, LL_DBG, "filename[%s]", filename);
    *buf = NULL;
    if (NULL == filename)
    {
        return 0;
    }
    char filenameBak[QUOS_FILE_NAME_MAXLENGHT + sizeof(FILE_DSNAME_BAK_HEAD)];
    HAL_SPRINTF(filenameBak, "%s", filename);
    char *point = filenameBak + HAL_STRLEN(filenameBak);
    while (point != filenameBak && '.' != *point)
    {
        point--;
    }
    if (point == filenameBak)
    {
        HAL_SPRINTF(filenameBak + HAL_STRLEN(filenameBak), "%s", FILE_DSNAME_BAK_HEAD);
    }
    else
    {
        HAL_MEMMOVE(point + HAL_STRLEN(FILE_DSNAME_BAK_HEAD), point, HAL_STRLEN(FILE_DSNAME_BAK_HEAD) + 1);
        HAL_MEMCPY(point, FILE_DSNAME_BAK_HEAD, HAL_STRLEN(FILE_DSNAME_BAK_HEAD));
    }

    SafeFlashHeadData_t sfHD[2];
    HAL_MEMSET(sfHD, 0, sizeof(sfHD));
    pointer_t fileFd = Qhal_fileOpen(filename, TRUE);
    pointer_t fileBakFd = Qhal_fileOpen(filenameBak, TRUE);
    if (SOCKET_FD_INVALID != fileFd)
    {
        Qhal_fileRead(fileFd, 0, &sfHD[0], __GET_POS_ELEMENT(SafeFlashHeadData_t, buf));
    }
    if (SOCKET_FD_INVALID != fileBakFd)
    {
        Qhal_fileRead(fileBakFd, 0, &sfHD[1], __GET_POS_ELEMENT(SafeFlashHeadData_t, buf));
    }
    Quos_logPrintf(LSDK_STORE, LL_DBG, "filename[%s] Index:%u len:%u crc:0x%02X", filename, sfHD[0].index, sfHD[0].len, sfHD[0].crc);
    Quos_logPrintf(LSDK_STORE, LL_DBG, "filenameBak[%s] Index:%u len:%u crc:0x%02X", filenameBak, sfHD[1].index, sfHD[1].len, sfHD[1].crc);

    if (0xFFFFFFFF == sfHD[0].index)
    {
        HAL_MEMSET(&sfHD[0], 0, sizeof(sfHD[0]));
    }
    if (0xFFFFFFFF == sfHD[1].index)
    {
        HAL_MEMSET(&sfHD[1], 0, sizeof(sfHD[1]));
    }
    if ((0 == sfHD[0].len && 0 == sfHD[1].len) || (*buf = HAL_MALLOC(sfHD[0].len > sfHD[1].len ? sfHD[0].len : sfHD[1].len)) == NULL)
    {
        Quos_logPrintf(LSDK_STORE, LL_ERR, "mcf buf %u:%u", sfHD[0].len, sfHD[1].len);
    }
    else if (sfHD[0].index > sfHD[1].index)
    {
        if (sfHD[0].len != 0 &&
            sfHD[0].len == Qhal_fileRead(fileFd, __GET_POS_ELEMENT(SafeFlashHeadData_t, buf), *buf, sfHD[0].len) &&
            sfHD[0].crc == (quint8_t)Quos_crcCalculate(0, *buf, sfHD[0].len))
        {
            ret = sfHD[0].len;
            Quos_logPrintf(LSDK_STORE, LL_DBG, "index[0] data is newest");
        }
        else if (sfHD[1].len != 0 &&
                 sfHD[1].len == Qhal_fileRead(fileBakFd, __GET_POS_ELEMENT(SafeFlashHeadData_t, buf), *buf, sfHD[1].len) &&
                 sfHD[1].crc == (quint8_t)Quos_crcCalculate(0, *buf, sfHD[1].len))
        {
            ret = sfHD[1].len;
            Quos_logPrintf(LSDK_STORE, LL_DBG, "index[1] data is newest because index[0] data was abnormal");
        }
    }
    else
    {
        if (sfHD[1].len != 0 &&
            sfHD[1].len == Qhal_fileRead(fileBakFd, __GET_POS_ELEMENT(SafeFlashHeadData_t, buf), *buf, sfHD[1].len) &&
            sfHD[1].crc == (quint8_t)Quos_crcCalculate(0, *buf, sfHD[1].len))
        {
            ret = sfHD[1].len;
            Quos_logPrintf(LSDK_STORE, LL_DBG, "index[1] data is newest");
        }
        else if (sfHD[0].len != 0 &&
                 sfHD[0].len == Qhal_fileRead(fileFd, __GET_POS_ELEMENT(SafeFlashHeadData_t, buf), *buf, sfHD[0].len) &&
                 sfHD[0].crc == (quint8_t)Quos_crcCalculate(0, *buf, sfHD[0].len))
        {
            ret = sfHD[0].len;
            Quos_logPrintf(LSDK_STORE, LL_DBG, "index[0] data is newest because index[1] data was abnormal");
        }
    }
    if (SOCKET_FD_INVALID != fileFd)
    {
        Qhal_fileClose(fileFd);
    }
    if (SOCKET_FD_INVALID != fileBakFd)
    {
        Qhal_fileClose(fileBakFd);
    }
    if (ret == 0 && NULL != *buf)
    {
        HAL_FREE(*buf);
        *buf = NULL;
    }
    if (aesKey)
    {
        AES_ctx_t aes_ctx;
        Quos_aesInitCtx(&aes_ctx, aesKey);
        Quos_aesEcbDecrypt(&aes_ctx, *buf, ret);
        ret = Quos_aesPaddingBack(*buf, ret);
    }
    return ret;
}
#endif

typedef struct
{
#define DATA_STORE_INFO_HEAD 0x56473830
    quint32_t head;
    quint8_t dat[1]; /* StoragerDataNode_t的元素组合 */
} dsKeyValueInfo_t;
/**************************************************************************
** 功能	@brief : 从Flash读取KEY-VALUE类数据
** 输入	@param : aesKey必须是QUOS_AES_KEYLEN字节
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_dsKvRead(const char *filename, const dsKeyValue_t keyValueNode[], const char *aesKey)
{
    dsKeyValueInfo_t *dataInfo = NULL;

    quint32_t len = Quos_dsRead(filename, (void **)&dataInfo, aesKey);
    if (NULL == dataInfo || dataInfo->head != DATA_STORE_INFO_HEAD || len <= sizeof(dataInfo->head))
    {
        HAL_FREE(dataInfo);
        Quos_logPrintf(LSDK_STORE, LL_ERR, "read store data fail");
        return FALSE;
    }
    len -= sizeof(dataInfo->head);
    quint16_t i;
    for (i = 0; i < len; i += 3)
    {
        quint16_t j = 0;
        while (keyValueNode[j].dat)
        {
            if (dataInfo->dat[i] == keyValueNode[j].id)
            {
                quint16_t nodeLen = _ARRAY01_U16(&dataInfo->dat[i + 1]);
                HAL_MEMSET(keyValueNode[j].dat, 0, keyValueNode[j].maxLen);
                if (nodeLen > 0 && nodeLen <= keyValueNode[j].maxLen)
                {
                    HAL_MEMCPY(keyValueNode[j].dat, &dataInfo->dat[i + 3], nodeLen);
                    Quos_logPrintf(LSDK_STORE, LL_DBG, "id[0x%02X] %.*s", keyValueNode[j].id, nodeLen, (char *)keyValueNode[j].dat);
                    Quos_logHexDump(LSDK_STORE, LL_DUMP, "", keyValueNode[j].dat, nodeLen);
                }

                i += nodeLen;
                break;
            }
            j++;
        }
    }
    HAL_FREE(dataInfo);
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 写入存储类数据到Flash
** 输入	@param : aesKey必须是QUOS_AES_KEYLEN字节
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_dsKvWrite(const char *filename, const dsKeyValue_t keyValueNode[], const char *aesKey)
{
    dsKeyValueInfo_t *dataInfo;
    quint16_t datLen = 0;
    quint8_t nodeCount = 0;
    while (keyValueNode[nodeCount].dat)
    {
        if (keyValueNode[nodeCount].isString)
        {
            datLen += 3 + HAL_STRLEN(keyValueNode[nodeCount].dat);
        }
        else
        {
            datLen += 3 + keyValueNode[nodeCount].maxLen;
        }
        nodeCount++;
    }
    Quos_logPrintf(LSDK_STORE, LL_ERR, "nodeCount[%u] datLen[%u]", nodeCount, datLen);
    dataInfo = (dsKeyValueInfo_t *)HAL_MALLOC(__GET_POS_ELEMENT(dsKeyValueInfo_t, dat) + datLen);
    if (NULL == dataInfo)
    {
        Quos_logPrintf(LSDK_STORE, LL_ERR, "mcf dataInfo");
        return FALSE;
    }
    dataInfo->head = DATA_STORE_INFO_HEAD;
    nodeCount = 0;
    datLen = 0;
    while (keyValueNode[nodeCount].dat)
    {
        quint16_t nodeLen;
        if (keyValueNode[nodeCount].isString)
        {
            nodeLen = HAL_STRLEN(keyValueNode[nodeCount].dat);
        }
        else
        {
            nodeLen = keyValueNode[nodeCount].maxLen;
        }

        dataInfo->dat[datLen++] = keyValueNode[nodeCount].id;
        _U16_ARRAY01(nodeLen, &dataInfo->dat[datLen]);
        datLen += 2;
        if (nodeLen > 0)
        {
            HAL_MEMCPY(&dataInfo->dat[datLen], keyValueNode[nodeCount].dat, nodeLen);
        }
        datLen += nodeLen;
        Quos_logPrintf(LSDK_STORE, LL_ERR, "id[0x%02X]", keyValueNode[nodeCount].id);
        Quos_logHexDump(LSDK_STORE, LL_DUMP, "", keyValueNode[nodeCount].dat, nodeLen);
        nodeCount++;
    }
    if (FALSE == Quos_dsWrite(filename, dataInfo, __GET_POS_ELEMENT(dsKeyValueInfo_t, dat) + datLen, aesKey))
    {
        HAL_FREE(dataInfo);
        return FALSE;
    }
    HAL_FREE(dataInfo);
    return TRUE;
}
