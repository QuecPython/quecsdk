#ifndef __QUOS_SIGNAL_H__
#define __QUOS_SIGNAL_H__
#include "quos_config.h"
#if (SDK_ENABLE_SIGNAL == 1)
typedef void (*SignalCB_f)(const void *arg, quint32_t argLen);
void Quos_signalInit(void);
qbool Quos_signalSet(void *arg, quint32_t len, SignalCB_f eventCB);
qbool Quos_signalTask(void);
#endif
#endif