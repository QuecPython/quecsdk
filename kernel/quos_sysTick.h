#ifndef __QUOS_SYSTICK_H__
#define __QUOS_SYSTICK_H__
#include "quos_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		quint32_t ms;  /* millisecond */
		quint32_t sec; /*  second,support 136years */
	} Systick_T;

	typedef struct
	{
		quint8_t tm_sec;   /* 秒 - 取值区间为[0,59] */
		quint8_t tm_min;   /* 分 - 取值区间为[0,59] */
		quint8_t tm_hour;  /* 时 - 取值区间为[0,23] */
		quint8_t tm_mday;  /* 一个月中的日期 - 取值区间为[1,31] */
		quint8_t tm_mon;   /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
		quint32_t tm_year; /* 实际年份，不小于1970 */
	} Quos_tm_t;

	void Quos_sysTickInit(void);
	void Quos_sysTickRectify(Systick_T sysTickNew);
	Systick_T Quos_sysTickGet(void);
	quint32_t Quos_sysTickGetMs(void);
	quint32_t Quos_sysTickGetS(void);
	Systick_T Quos_sysTickAddMs(Systick_T tick, quint32_t ms);
	qint32_t Quos_sysTickdiff(Systick_T time1, Systick_T time2, qbool isMs);
	void Quos_timezoneSet(qint32_t tz);
	qint32_t Quos_timezoneGet(void);
	Quos_tm_t Quos_localtime(quint32_t time);
	quint32_t Quos_mktime(Quos_tm_t tm);

#ifdef __cplusplus
}
#endif
#endif