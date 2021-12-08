#ifndef __QUOS_LOG_H__
#define __QUOS_LOG_H__
#include "quos_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LL_OFF (0X07)  /* 关闭所有日志 */
#define LL_FAIL (0X06) /* 将导致程序退出的错误及其以上 */
#define LL_ERR (0X05)  /* 发生错误但不会导致程序退出及其以上 */
#define LL_WARN (0X04) /* 警告级别错误及其以上 */
#define LL_INFO (0X03) /* 粗粒度级别log及其以上 */
#define LL_DBG (0X02)  /* 所有log */
#define LL_DUMP (0X01) /* dump数据，仅用于Quos_logHexDump打印 */

#ifndef QUOS_LOGL
#define QUOS_LOGL LL_ERR
#endif

#define Quos_logPrintf(TYPE, LEVEL, format, ...)                                                                               \
	do                                                                                                                         \
	{                                                                                                                          \
		if (LEVEL >= TYPE && LEVEL >= QUOS_LOGL)                                                                               \
		{                                                                                                                      \
			HAL_LOCK(lockLogId);                                                                                               \
			HAL_PRINTF("%s<%-12s> %s[%d] " format "\r\n", Qhal_logHeadString(), #TYPE, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
			HAL_UNLOCK(lockLogId);                                                                                             \
		}                                                                                                                      \
	} while (0)

#define Quos_logHexDump(TYPE, LEVEL, HEAD, DAT, DATLEN)                                                       \
	do                                                                                                        \
	{                                                                                                         \
		if (LEVEL >= TYPE && LEVEL >= QUOS_LOGL)                                                              \
		{                                                                                                     \
			HAL_LOCK(lockLogId);                                                                              \
			HAL_PRINTF("%s<%-12s> %s[%d] %s\r\n", Qhal_logHeadString(), #TYPE, __FUNCTION__, __LINE__, HEAD); \
			HAL_PRINTF("*************************** %04d **************************", (quint16_t)(DATLEN));   \
			Quos_logHexDumpData(DAT, DATLEN);                                                                 \
			HAL_UNLOCK(lockLogId);                                                                            \
		}                                                                                                     \
	} while (0)

	void Quos_logHexDumpData(const void *dat, quint16_t len);

	HAL_LOCK_DEF(extern, lockLogId)
	extern char *Qhal_logHeadString(void);
#ifdef __cplusplus
}
#endif
#endif