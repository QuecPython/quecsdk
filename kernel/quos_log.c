/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : ͳһ��־��ӡAPI
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
***************************************************************************/
#include "quos_log.h"
HAL_LOCK_DEF(,lockLogId)

/**************************************************************************
** ����	@brief : HEXDUMP����
** ����	@param :
** ���	@retval:
***************************************************************************/
void FUNCTION_ATTR_RAM Quos_logHexDumpData(const void *dat, quint16_t len)
{
	quint16_t i;
	for (i = 0; i < len; i++)
	{
		if (i % 20 == 0)
		{
			HAL_PRINTF("\r\n");
		}
		HAL_PRINTF("%02X ", ((quint8_t *)dat)[i]);
	}
	HAL_PRINTF("\r\n");
}
