/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 软件定时器
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_swTimer.h"
#include "Quos_kernel.h"

#if (SDK_ENABLE_TIMER == 1)

typedef struct
{
	TWLLHead_T head;
	char *name;
	Systick_T endTime;
	quint32_t timeout; /* 定时时长，单位ms */
	quint32_t repeat;  /* 重复次数 */
	void *parm;
	void **self;
	void (*timeoutCb)(void *swTimer);
} SWTimer_T;

#define SWT_FOREVER			((quint32_t)-1)
static TWLLHead_T *SWTimerHead = NULL;
HAL_LOCK_DEF(static, lockId)
/**************************************************************************
** 功能	@brief : 打印定时器
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_swTimerPrintf(void)
{
	Systick_T now = Quos_sysTickGet();
	TWLLHead_T *temp, *next;
	Quos_logPrintf(LSDK_TIMER, LL_INFO, "**list pointer*********endTime*****timeout**repeat*******timerCB***********param****name******(" TIME_SEC2UTC ":%03d)***", TIME_SEC2UTC_(now.sec), now.ms);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		Quos_logPrintf(LSDK_TIMER, LL_INFO, "%-16p " TIME_SEC2UTC ":%03d %8d %6d %16p %16p %s", timer, TIME_SEC2UTC_(timer->endTime.sec), timer->endTime.ms, timer->timeout, timer->repeat, timer->timeoutCb, timer->parm, timer->name);
	}
}
/**************************************************************************
** 功能	@brief : 初始化定时器
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_swTimerInit(void)
{
	HAL_LOCK_INIT(lockId);
}
/**************************************************************************
** 功能	@brief : 启动一个定时器
** 输入	@param : timeout 定时时长；repeat 定时次数，0为不断重复
timeoutCb 定时时间到的回调处理函数，此函数需尽量简短,
timeoverCb 定是次数结束调用，为NULL时则无需通知
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_swTimerStart(void **swTimerP, char *name, quint32_t timeout, quint32_t repeat, const SWTimerCB timeoutCb, void *parm)
{
	SWTimer_T **swTimer = (SWTimer_T **)swTimerP;
	Quos_logPrintf(LSDK_TIMER, LL_INFO, "swTimer[%p] *swTimer[%p] name[%s] timeout[%u] repeat[%u] timeoutCb[%p] parm[%p]", swTimer, *swTimer, name, timeout, repeat, timeoutCb, parm);

	if (NULL == timeoutCb && SWT_SUSPEND != timeout)
	{
		Quos_logPrintf(LSDK_TIMER, LL_ERR, "parm invaild");
		return FALSE;
	}

	SWTimer_T *listNew = NULL;
	HAL_LOCK(lockId);
	if (swTimer && *swTimer)
	{
		TWLLHead_T *temp, *next;
		TWLIST_FOR_DATA(SWTimerHead, temp, next)
		{
			SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
			if (timer == *swTimer)
			{
				Quos_logPrintf(LSDK_TIMER, LL_DBG, "swTimer has in list");
				Quos_twllHeadDelete(&SWTimerHead, temp);
				listNew = timer;
				break;
			}
		}
	}
	if (NULL == listNew)
	{
		listNew = (SWTimer_T *)HAL_MALLOC(sizeof(SWTimer_T));
		if (NULL == listNew)
		{
			Quos_logPrintf(LSDK_TIMER, LL_ERR, "mcf newTimer");
			if (swTimer)
			{
				*swTimer = NULL;
			}
			HAL_UNLOCK(lockId);
			return FALSE;
		}
		HAL_MEMSET(listNew, 0, sizeof(sizeof(SWTimer_T)));
		listNew->self = (void **)swTimer;
		if (swTimer)
		{
			*swTimer = (void *)listNew;
		}
	}

	listNew->name = name;
	listNew->endTime = Quos_sysTickAddMs(Quos_sysTickGet(), timeout);
	listNew->timeout = timeout;
	listNew->repeat = (repeat == 0) ? SWT_FOREVER : repeat;
	listNew->parm = parm;
	listNew->timeoutCb = timeoutCb;
	Quos_twllHeadAdd(&SWTimerHead, &listNew->head);
	Quos_kernelResume();
	quos_swTimerPrintf();
	HAL_UNLOCK(lockId);
	return TRUE;
}
/**************************************************************************
** 功能	@brief : 删除指定编号定时器
** 输入	@param : 定时器编号
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_swTimerDelete(void *swTimerP)
{
	SWTimer_T *swTimer = (SWTimer_T *)swTimerP;
	Quos_logPrintf(LSDK_TIMER, LL_INFO, "swTimer[%p]:%s", swTimer, (swTimer && swTimer->name) ? swTimer->name : "");
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if (timer == swTimer)
		{
			Quos_logPrintf(LSDK_TIMER, LL_DBG, "timer is del repeat[%u]", timer->repeat);
			if (timer->self)
			{
				*timer->self = NULL;
				timer->self = NULL;
			}
			Quos_twllHeadDelete(&SWTimerHead, temp);
			HAL_FREE(timer);
			Quos_kernelResume();
			quos_swTimerPrintf();
			break;
		}
	}
	HAL_UNLOCK(lockId);
}

/**************************************************************************
** 功能	@brief : 变更定时器超时时间
** 输入	@param : timeout：超时时间
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_swTimerTimeoutSet(void *swTimerP, quint32_t timeout)
{
	SWTimer_T *swTimer = (SWTimer_T *)swTimerP;
	Quos_logPrintf(LSDK_TIMER, LL_DBG, "timeout:%u", timeout);
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if (timer == swTimer)
		{
			timer->timeout = timeout;
			timer->endTime = Quos_sysTickAddMs(Quos_sysTickGet(), timer->timeout);
			Quos_kernelResume();
			break;
		}
	}
	HAL_UNLOCK(lockId);
}
/**************************************************************************
** 功能	@brief : 获取定时器超时时间
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_swTimerTimeoutGet(void *swTimerP)
{
	quint32_t timeout = SWT_SUSPEND;
	SWTimer_T *swTimer = (SWTimer_T *)swTimerP;
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if (timer == swTimer)
		{
			timeout = timer->timeout;
			break;
		}
	}
	HAL_UNLOCK(lockId);
	return timeout;
}
/**************************************************************************
** 功能	@brief : 变更定时器重复次数
** 输入	@param : repeat:0为永久
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_swTimerRepeatSet(void *swTimerP, quint32_t repeat)
{
	SWTimer_T *swTimer = (SWTimer_T *)swTimerP;
	Quos_logPrintf(LSDK_TIMER, LL_DBG, "repeat:%u", repeat);
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if (timer == swTimer)
		{
			timer->repeat = (repeat == 0) ? SWT_FOREVER : repeat;
			Quos_kernelResume();
			break;
		}
	}
	HAL_UNLOCK(lockId);
}
/**************************************************************************
** 功能	@brief : 获取定时器重复次数
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_swTimerRepeatGet(void *swTimerP)
{
	quint32_t repeat = 0;
	SWTimer_T *swTimer = (SWTimer_T *)swTimerP;
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if (timer == swTimer)
		{
			repeat = timer->repeat;
			break;
		}
	}
	HAL_UNLOCK(lockId);
	return repeat;
}
/**************************************************************************
** 功能	@brief : 获取定时器外参
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM *Quos_swTimerParmGet(void *swTimerP)
{
	void *param = NULL;
	SWTimer_T *swTimer = (SWTimer_T *)swTimerP;
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if (timer == swTimer)
		{
			param = timer->parm;
		}
	}
	HAL_UNLOCK(lockId);
	return param;
}
/**************************************************************************
** 功能	@brief : 变更定时器回调函数
** 输入	@param : repeat:0为永久
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_swTimerCBSet(void *swTimerP, const SWTimerCB timeoutCb)
{
	SWTimer_T *swTimer = (SWTimer_T *)swTimerP;
	Quos_logPrintf(LSDK_TIMER, LL_DBG, "timeoutCb:%p", timeoutCb);
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);
	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if (timer == swTimer)
		{
			timer->timeoutCb = timeoutCb;
			break;
		}
	}
	HAL_UNLOCK(lockId);
}
/**************************************************************************
** 功能	@brief : 检查定时器时间是否已到
** 输入	@param :
** 输出	@retval: 距离下次响应时长，-1：无定时器
***************************************************************************/
quint32_t FUNCTION_ATTR_RAM Quos_swTimerTask(void)
{
	SWTimer_T *listMinTime = NULL;
	TWLLHead_T *temp, *next;
	HAL_LOCK(lockId);

	TWLIST_FOR_DATA(SWTimerHead, temp, next)
	{
		SWTimer_T *timer = __GET_STRUCT_BY_ELEMENT(temp, SWTimer_T, head);
		if(0 == timer->repeat)
		{
			if(timer->self)
			{
				*timer->self = NULL;
				timer->self = NULL;
			}						
			Quos_twllHeadDelete(&SWTimerHead, temp);
			HAL_FREE(timer);
			quos_swTimerPrintf();
		}
		else if (SWT_SUSPEND == timer->timeout) /* timeout为-1时为无效定时器 */
		{
			// DO NOT
		}
		else if (NULL == listMinTime || Quos_sysTickdiff(timer->endTime, listMinTime->endTime, TRUE) < 0)
		{
			listMinTime = timer;
		}
	}
	if(NULL == listMinTime)
	{
		HAL_UNLOCK(lockId);
		Quos_logPrintf(LSDK_TIMER, LL_DBG, "suspend");
		return SWT_SUSPEND;
	}

	qint32_t interval = Quos_sysTickdiff(listMinTime->endTime, Quos_sysTickGet(), TRUE);
	if(interval > 0)
	{
		HAL_UNLOCK(lockId);
		Quos_logPrintf(LSDK_TIMER, LL_DBG, "interval:%d", interval);
		return interval;
	}
	Quos_logPrintf(LSDK_TIMER, LL_INFO, "%s:%p  timeout:%u repeat:%u", listMinTime->name, listMinTime, listMinTime->timeout, listMinTime->repeat);
	if (listMinTime->repeat != SWT_FOREVER)
	{
		listMinTime->repeat--;
	}
	listMinTime->endTime = Quos_sysTickAddMs(Quos_sysTickGet(), listMinTime->timeout);
	HAL_UNLOCK(lockId);
	Quos_logPrintf(LSDK_TIMER, LL_DBG, "exec cb[%s]",listMinTime->name);
	listMinTime->timeoutCb((void *)listMinTime);
	Quos_logPrintf(LSDK_TIMER, LL_DBG, "exec cb[%s] finish",listMinTime->name);
	return 0;
}
#endif
