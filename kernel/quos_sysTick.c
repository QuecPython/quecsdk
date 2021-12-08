/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 系统时钟
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_sysTick.h"
#include "Quos_kernel.h"
#include "Qhal_driver.h"
static qint32_t Timezone = 8 * 3600;
HAL_LOCK_DEF(static, lockId)
static Systick_T SysTickStart = {0, 0};
/**************************************************************************
** 功能	@brief : 初始化系统时间
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_sysTickInit(void)
{
	Qhal_rtcInit();
	HAL_LOCK_INIT(lockId);
    Qhal_rtcGet(&SysTickStart.sec, &SysTickStart.ms);
}
/**************************************************************************
** 功能	@brief : 纠正系统时间
** 输入	@param :
** 输出	@retval:
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
** 功能	@brief : 获取当前系统运行时间
** 输入	@param :
** 输出	@retval:
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
** 功能	@brief : 获取当前系统运行时间ms
** 输入	@param :
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_sysTickGetMs(void)
{
	Systick_T now = Quos_sysTickGet();
	return (quint32_t)(now.sec * 1000 + now.ms);
}
/**************************************************************************
** 功能	@brief : 获取当前系统运行时间s
** 输入	@param :
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_sysTickGetS(void)
{
	return Quos_sysTickGet().sec;
}

/**************************************************************************
** 功能	@brief : 计算tick加上ms后时间
** 输入	@param : 
** 输出	@retval: 
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
** 功能	@brief : 比较时间大小
** 输入	@param : isMs:差值单位TRUE:毫秒,FALSE:秒
** 输出	@retval: 返回时间差值
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
** 功能	@brief : 设置时区
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_timezoneSet(qint32_t tz)
{
	Timezone = tz;
}
/**************************************************************************
** 功能	@brief : 获取时区
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
qint32_t Quos_timezoneGet(void)
{
	return Timezone;
}
/**************************************************************************
** 功能	@brief : 时间戳转成年月日时分秒
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
static const quint8_t Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
Quos_tm_t FUNCTION_ATTR_RAM Quos_localtime(quint32_t time)
{
	Quos_tm_t tm;
	quint32_t Pass4year;
	quint32_t hours_per_year;
	time += Timezone;
	tm.tm_sec = time % 60; /*取秒时间 */
	time /= 60;
	tm.tm_min = time % 60; /*取分钟时间 */
	time /= 60;
	Pass4year = time / (1461L * 24L);	  /*取过去多少个四年，每四年有 1461*24 小时 */
	tm.tm_year = (Pass4year << 2) + 1970; /*计算年份 */
	time %= 1461L * 24L;				  /*四年中剩下的小时数 */
	/*校正闰年影响的年份，计算一年中剩下的小时数 */
	for (;;)
	{
		hours_per_year = 365 * 24; /*一年的小时数 */
		if ((tm.tm_year & 3) == 0)
			hours_per_year += 24; /*判断闰年,一年则多24小时，即一天 */
		if (time < hours_per_year)
			break;
		tm.tm_year++;
		time -= hours_per_year;
	}

	tm.tm_hour = time % 24; /*小时数 */
	time /= 24;				/*一年中剩下的天数 */
	time++;					/*假定为闰年 */
	/*校正闰年的误差，计算月份，日期 */
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
	/*计算月日 */
	for (tm.tm_mon = 0; Days[tm.tm_mon] < time; tm.tm_mon++)
	{
		time -= Days[tm.tm_mon];
	}
	tm.tm_mon += 1;
	tm.tm_mday = time;
	return tm;
}
/**************************************************************************
** 功能	@brief : 年月日时分秒转成时间戳
** 输入	@param :         
** 输出	@retval:        
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
	/* 以平年时间计算的秒数 */
	quint32_t result = (tm.tm_year - 1970) * 365 * 24 * 3600 +
					  (mon_yday[isleap(tm.tm_year) ? 1 : 0][tm.tm_mon - 1] + tm.tm_mday - 1) * 24 * 3600 +
					  tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
	/* 加上闰年的秒数 */
	quint32_t i;
	for (i = 1970; i < tm.tm_year; i++)
	{
		if (isleap(i))
			result += 24 * 3600;
	}

	return result - Timezone;
}