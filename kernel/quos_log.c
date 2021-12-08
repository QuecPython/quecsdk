/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 统一日志打印API
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_log.h"
HAL_LOCK_DEF(,lockLogId)

/**************************************************************************
** 功能	@brief : HEXDUMP数据
** 输入	@param :
** 输出	@retval:
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
