/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : 
** Ӳ��   @hardware��Linuxƽ̨
** ����   @other   ��
***************************************************************************/
#include "qhal_FlashOpt.h"

#include "Qhal_types.h"
#include "Ql_iotApi.h"

#include "helios_fs.h"
/**************************************************************************
** ����	@brief : ���ļ�
** ����	@param : 
** ���	@retval:
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
** ����	@brief : Flashд������
** ����	@param : 
** ���	@retval:
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
** ����	@brief : Flash��ȡ����
** ����	@param : 
** ���	@retval:
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
** ����	@brief : �ر��ļ�
** ����	@param : 
** ���	@retval:
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
** ����	@brief : Flash������������
** ����	@param : 
** ���	@retval:
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
