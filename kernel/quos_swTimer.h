#ifndef __QUOS_SWTIMER_H__
#define __QUOS_SWTIMER_H__
#include "quos_config.h"
#include "quos_twll.h"
#include "quos_sysTick.h"

#if (SDK_ENABLE_TIMER == 1)

#define SWT_SUSPEND			((quint32_t)-1)
#define SWT_ONE_MSEC (1)
#define SWT_ONE_SECOND (1000 * SWT_ONE_MSEC)
#define SWT_ONE_MINUTE (60 * SWT_ONE_SECOND)
#define SWT_ONE_HOUR (60 * SWT_ONE_MINUTE)
#define SWT_ONE_DAY (24 * SWT_ONE_HOUR)

typedef struct __SWTimer
{
	TWLLHead_T head;
	char *name;
	Systick_T endTime;
	quint32_t timeout; /* 定时时长，单位ms */
	quint32_t repeat;  /* 重复次数 */
	void *parm;
	void **self;
	void (*timeoutCb)(struct __SWTimer *swTimer);
} SWTimer_T;

typedef void (*SWTimerCB)(SWTimer_T *swTimer);
qbool Quos_swTimerStart(SWTimer_T **swTimer, char *name, quint32_t timeout, quint32_t repeat, const SWTimerCB timeoutCb, void *parm);
void Quos_swTimerDelete(SWTimer_T *SWTimer);
void Quos_swTimerTimeoutSet(SWTimer_T *swTimer, quint32_t timeout);
void Quos_swTimerRepeatSet(SWTimer_T *swTimer, quint32_t repeat);
void Quos_swTimerCBSet(SWTimer_T *swTimer, const SWTimerCB timeoutCb);
quint32_t Quos_swTimerTask(void);

#endif
#endif