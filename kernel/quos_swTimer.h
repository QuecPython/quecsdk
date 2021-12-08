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

typedef void (*SWTimerCB)(void *swTimerP);
void Quos_swTimerInit(void);
qbool Quos_swTimerStart(void **swTimerP, char *name, quint32_t timeout, quint32_t repeat, const SWTimerCB timeoutCb, void *parm);
void Quos_swTimerDelete(void *SWTimerP);
void Quos_swTimerTimeoutSet(void *swTimerP, quint32_t timeout);
quint32_t Quos_swTimerTimeoutGet(void *swTimerP);
void Quos_swTimerRepeatSet(void *swTimerP, quint32_t repeat);
quint32_t Quos_swTimerRepeatGet(void *swTimerP);
void *Quos_swTimerParmGet(void *swTimerP);
void Quos_swTimerCBSet(void *swTimerP, const SWTimerCB timeoutCb);
quint32_t Quos_swTimerTask(void);

#endif
#endif