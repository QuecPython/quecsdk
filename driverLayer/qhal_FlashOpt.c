/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 
** 硬件   @hardware：Linux平台
** 其他   @other   ：
***************************************************************************/
#include "qhal_FlashOpt.h"

#include "Qhal_types.h"
#include "Ql_iotApi.h"

#include "helios_fs.h"
/**************************************************************************
** 功能	@brief : 打开文件
** 输入	@param : 
** 输出	@retval:
***************************************************************************/
pointer_t FUNCTION_ATTR_ROM Qhal_fileOpen(const char *filename, qbool onlyRead)
{    
    HeliosFILE *fp = NULL;
    if(NULL == filename || 0 == HAL_STRLEN(filename))
    {
        Quos_logPrintf(HAL_FLASH,LL_ERR,"filename invalid");
        return SOCKET_FD_INVALID;
    }
    if(onlyRead)
    {
        fp = Helios_fopen((const char*)filename,"rb");
#if defined (PLAT_Unisoc)
        if((int)fp < 0)
#elif defined (PLAT_ASR)
        if(fp == NULL)
#else
        if(fp == NULL)
#endif
        {
            Quos_logPrintf(HAL_FLASH,LL_ERR,"open %s fail",filename);
            return SOCKET_FD_INVALID;
        }
    }
    else
    {
        fp = Helios_fopen((const char*)filename,"rb+");
#if defined (PLAT_Unisoc)
        if((int)fp < 0)
#elif defined (PLAT_ASR)
        if(fp == NULL)
#else
        if(fp == NULL)
#endif
        {
            fp = Helios_fopen((const char*)filename,"wb+");
#if defined (PLAT_Unisoc)
        if((int)fp < 0)
#elif defined (PLAT_ASR)
        if(fp == NULL)
#else
        if(fp == NULL)
#endif
            {
                Quos_logPrintf(HAL_FLASH,LL_ERR,"open %s fail",filename);
                return SOCKET_FD_INVALID;
            }
        }
    }
    return (pointer_t)fp;
}
/**************************************************************************
** 功能	@brief : Flash写入数据
** 输入	@param : 
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Qhal_fileWrite(pointer_t sockFd, quint32_t offset, void *buf, quint16_t bufLen)
{
    HeliosFILE *fp = (HeliosFILE *)sockFd;
    if(Helios_ftell(fp) != (int)offset)
    {
        Helios_fseek(fp, (long)offset, 0);
    }
    
    if(bufLen != Helios_fwrite(buf,  bufLen, 1, fp))
    {
        Quos_logPrintf(HAL_FLASH,LL_ERR,"len is err");
        return FALSE;
    }
    Quos_logPrintf(HAL_FLASH,LL_DBG,"success");    
    return TRUE;
}
/**************************************************************************
** 功能	@brief : Flash读取数据
** 输入	@param : 
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Qhal_fileRead(pointer_t sockFd, quint32_t offset, void *buf, quint16_t bufLen)
{
    HeliosFILE *fp = (HeliosFILE *)sockFd;
    if(Helios_ftell(fp) != (int)offset)
    {
        Helios_fseek(fp, (long)offset, 0);
    }
    quint32_t rLen =  Helios_fread(buf,  bufLen, 1, fp);  
    if(rLen <=0)
    {
        Quos_logPrintf(HAL_FLASH,LL_ERR,"len is err");
        return 0;
    }
    else
    {
        Quos_logHexDump(HAL_FLASH,LL_DUMP,"buf",buf,rLen);
        return rLen;
    }
}
/**************************************************************************
** 功能	@brief : 关闭文件
** 输入	@param : 
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Qhal_fileClose(pointer_t sockFd)
{
#if defined (PLAT_Unisoc)
    if((int)sockFd < 0)
#elif defined (PLAT_ASR)
    if(sockFd == 0)
#else
    if(sockFd == 0)
#endif
    {
        return;
    }
    Helios_fclose((HeliosFILE *)sockFd);
}
/**************************************************************************
** 功能	@brief : Flash擦除扇区数据
** 输入	@param : 
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Qhal_fileErase(const char *filename)
{
    if(NULL == filename || 0 == HAL_STRLEN(filename))
    {
        Quos_logPrintf(HAL_FLASH,LL_ERR,"filename invalid");
        return FALSE;
    }
    Quos_logPrintf(HAL_FLASH,LL_DBG,"filename:%s",filename);
    Helios_remove(filename);
    return TRUE;
}
