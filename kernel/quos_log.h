#ifndef __QUOS_LOG_H__
#define __QUOS_LOG_H__
#include "quos_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LL_OFF (0X07)  /* �ر�������־ */
#define LL_FAIL (0X06) /* �����³����˳��Ĵ��������� */
#define LL_ERR (0X05)  /* �������󵫲��ᵼ�³����˳��������� */
#define LL_WARN (0X04) /* ���漶����������� */
#define LL_INFO (0X03) /* �����ȼ���log�������� */
#define LL_DBG (0X02)  /* ����log */
#define LL_DUMP (0X01) /* dump���ݣ�������Quos_logHexDump��ӡ */

#ifndef QUOS_LOGL
#define QUOS_LOGL LL_DBG
#endif

#define Quos_logPrintf(TYPE, LEVEL, format, ...)                                                                          \
	if (LEVEL >= TYPE && LEVEL >= QUOS_LOGL)                                                                              \
	{                                                                                                                     \
		HAL_LOCK(lockLogId);                                                                                              \
		HAL_PRINTF("%s<%s\t> %s[%d] " format "\r\n", Qhal_logHeadString(), #TYPE, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
		HAL_UNLOCK(lockLogId);                                                                                            \
	}

#if (SDK_ENABLE_LOGDUMP ==1 )
#define Quos_logHexDump(TYPE, LEVEL, HEAD, DAT, DATLEN)                                                  \
	if (LEVEL >= TYPE && LEVEL >= QUOS_LOGL)                                                             \
	{                                                                                                    \
		HAL_LOCK(lockLogId);                                                                             \
		HAL_PRINTF("%s<%s\t> %s[%d] %s\r\n", Qhal_logHeadString(), #TYPE, __FUNCTION__, __LINE__, HEAD); \
		HAL_PRINTF("*************************** %04d **************************", (quint16_t)(DATLEN));   \
		Quos_logHexDumpData(DAT, DATLEN);                                                                \
		HAL_UNLOCK(lockLogId);                                                                           \
	}
	void Quos_logHexDumpData(void *dat, quint16_t len);
#else
	#define Quos_logHexDump(TYPE, LEVEL, HEAD, DAT, DATLEN)
#endif
	HAL_LOCK_DEF(extern, lockLogId)
	extern char *Qhal_logHeadString(void);
#ifdef __cplusplus
}
#endif
#endif