/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : ϵͳʱ��
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
***************************************************************************/
#include "quos_sysTick.h"
#include "Quos_kernel.h"
#include "Qhal_driver.h"
static qint32_t Timezone = 8 * 3600;
HAL_LOCK_DEF(static, lockId)
static Systick_T SysTickStart = {0, 0};
/**************************************************************************
** ����	@brief : ��ʼ��ϵͳʱ��
** ����	@param :
** ���	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_sysTickInit(void)
{
	Qhal_rtcInit();
	HAL_LOCK_INIT(lockId);
    Qhal_rtcGet(&SysTickStart.sec, &SysTickStart.ms);
}
/**************************************************************************
** ����	@brief : ����ϵͳʱ��
** ����	@param :
** ���	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_sysTickRectify(Systick_T sysTickNew)
{
	HAL_LOCK(lockId);
	Systick_T systickNow;
    Qhal_rtcGet(&systickNow.sec, &systickNow.ms);

	qint32_t diffVal = Quos_sysTickdiff(sysTickNew, systickNow, TRUE);
	if (diffVal < 0)
	{
		diffVal = 0 - diffVal;
		SysTickStart.sec -= diffVal / 1000;
		diffVal %= 1000;
		if (SysTickStart.ms < (quint32_t)diffVal)
		{
			SysTickStart.sec--;
			SysTickStart.ms += 1000 - diffVal;
		}
		else
		{
			SysTickStart.ms -= diffVal;
		}
	}
	else
	{
		SysTickStart.ms += diffVal %= 1000;
		SysTickStart.sec += diffVal / 1000;
		if (SysTickStart.ms >= 1000)
		{
			SysTickStart.sec += SysTickStart.ms / 1000;
			SysTickStart.ms %= SysTickStart.ms;
		}
	}
	HAL_UNLOCK(lockId);
}
/**************************************************************************
** ����	@brief : ��ȡ��ǰϵͳ����ʱ��
** ����	@param :
** ���	@retval:
***************************************************************************/
Systick_T FUNCTION_ATTR_ROM Quos_sysTickGet(void)
{
	HAL_LOCK(lockId);
	Systick_T SysTickNow;
    Qhal_rtcGet(&SysTickNow.sec, &SysTickNow.ms);
	SysTickNow.sec -= SysTickStart.sec;
	if (SysTickNow.ms < SysTickStart.ms)
	{
		SysTickNow.sec--;
		SysTickNow.ms += 1000;
	}
	SysTickNow.ms -= SysTickStart.ms;
	HAL_UNLOCK(lockId);
	return SysTickNow;
}
/**************************************************************************
** ����	@brief : ��ȡ��ǰϵͳ����ʱ��ms
** ����	@param :
** ���	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_sysTickGetMs(void)
{
	Systick_T now = Quos_sysTickGet();
	return (quint32_t)(now.sec * 1000 + now.ms);
}
/**************************************************************************
** ����	@brief : ��ȡ��ǰϵͳ����ʱ��s
** ����	@param :
** ���	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_sysTickGetS(void)
{
	return Quos_sysTickGet().sec;
}

/**************************************************************************
** ����	@brief : ����tick����ms��ʱ��
** ����	@param : 
** ���	@retval: 
***************************************************************************/
Systick_T FUNCTION_ATTR_ROM Quos_sysTickAddMs(Systick_T tick, quint32_t ms)
{
	tick.sec += ms / 1000;
	tick.ms += ms % 1000;
	tick.sec += tick.ms / 1000;
	tick.ms = tick.ms % 1000;
	return tick;
}

/**************************************************************************
** ����	@brief : �Ƚ�ʱ���С
** ����	@param : isMs:��ֵ��λTRUE:����,FALSE:��
** ���	@retval: ����ʱ���ֵ
***************************************************************************/
qint32_t FUNCTION_ATTR_ROM Quos_sysTickdiff(Systick_T time1, Systick_T time2, qbool isMs)
{
	qint32_t interval;
	if (time1.sec > time2.sec || (time1.sec == time2.sec && time1.ms >= time2.ms))
	{
		if (time1.ms < time2.ms)
		{
			time1.sec--;
			time1.ms += 1000;
		}
		interval = time1.sec - time2.sec;
		if (isMs)
			interval = interval * 1000 + (time1.ms - time2.ms);
	}
	else
	{
		if (time2.ms < time1.ms)
		{
			time2.sec--;
			time2.ms += 1000;
		}
		interval = time1.sec - time2.sec;
		if (isMs)
			interval = interval * 1000 - (time2.ms - time1.ms);
	}
	return interval;
}
/**************************************************************************
** ����	@brief : ����ʱ��
** ����	@param :         
** ���	@retval:        
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_timezoneSet(qint32_t tz)
{
	Timezone = tz;
}
/**************************************************************************
** ����	@brief : ��ȡʱ��
** ����	@param :         
** ���	@retval:        
***************************************************************************/
qint32_t Quos_timezoneGet(void)
{
	return Timezone;
}
/**************************************************************************
** ����	@brief : ʱ���ת��������ʱ����
** ����	@param :         
** ���	@retval:        
***************************************************************************/
static const quint8_t Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
Quos_tm_t FUNCTION_ATTR_RAM Quos_localtime(quint32_t time)
{
	Quos_tm_t tm;
	quint32_t Pass4year;
	quint32_t hours_per_year;
	time += Timezone;
	tm.tm_sec = time % 60; /*ȡ��ʱ�� */
	time /= 60;
	tm.tm_min = time % 60; /*ȡ����ʱ�� */
	time /= 60;
	Pass4year = time / (1461L * 24L);	  /*ȡ��ȥ���ٸ����꣬ÿ������ 1461*24 Сʱ */
	tm.tm_year = (Pass4year << 2) + 1970; /*������� */
	time %= 1461L * 24L;				  /*������ʣ�µ�Сʱ�� */
	/*У������Ӱ�����ݣ�����һ����ʣ�µ�Сʱ�� */
	for (;;)
	{
		hours_per_year = 365 * 24; /*һ���Сʱ�� */
		if ((tm.tm_year & 3) == 0)
			hours_per_year += 24; /*�ж�����,һ�����24Сʱ����һ�� */
		if (time < hours_per_year)
			break;
		tm.tm_year++;
		time -= hours_per_year;
	}

	tm.tm_hour = time % 24; /*Сʱ�� */
	time /= 24;				/*һ����ʣ�µ����� */
	time++;					/*�ٶ�Ϊ���� */
	/*У��������������·ݣ����� */
	if ((tm.tm_year & 3) == 0)
	{
		if (time > 60)
			time--;
		else if (time == 60)
		{
			tm.tm_mon = 1;
			tm.tm_mday = 29;
			return tm;
		}
	}
	/*�������� */
	for (tm.tm_mon = 0; Days[tm.tm_mon] < time; tm.tm_mon++)
	{
		time -= Days[tm.tm_mon];
	}
	tm.tm_mon += 1;
	tm.tm_mday = time;
	return tm;
}
/**************************************************************************
** ����	@brief : ������ʱ����ת��ʱ���
** ����	@param :         
** ���	@retval:        
***************************************************************************/
static quint32_t mon_yday[2][12] =
	{
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

static qbool isleap(quint32_t year)
{
	return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

quint32_t FUNCTION_ATTR_RAM Quos_mktime(Quos_tm_t tm)
{
	/* ��ƽ��ʱ���������� */
	quint32_t result = (tm.tm_year - 1970) * 365 * 24 * 3600 +
					  (mon_yday[isleap(tm.tm_year) ? 1 : 0][tm.tm_mon - 1] + tm.tm_mday - 1) * 24 * 3600 +
					  tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
	/* ������������� */
	quint32_t i;
	for (i = 1970; i < tm.tm_year; i++)
	{
		if (isleap(i))
			result += 24 * 3600;
	}

	return result - Timezone;
}