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
		quint8_t tm_sec;   /* �� - ȡֵ����Ϊ[0,59] */
		quint8_t tm_min;   /* �� - ȡֵ����Ϊ[0,59] */
		quint8_t tm_hour;  /* ʱ - ȡֵ����Ϊ[0,23] */
		quint8_t tm_mday;  /* һ�����е����� - ȡֵ����Ϊ[1,31] */
		quint8_t tm_mon;   /* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
		quint32_t tm_year; /* ʵ����ݣ���С��1970 */
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