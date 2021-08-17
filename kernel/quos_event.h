#ifndef __QUOS_EVENT_H__
#define __QUOS_EVENT_H__
#include "quos_config.h"
#if (SDK_ENABLE_EVENT == 1) 
typedef void (*EventCB_f)(qint32_t event, void *arg);
void Quos_eventCbReg(const qint32_t eventArray[], quint32_t eventCnt, EventCB_f eventCb);
qbool Quos_eventPost(qint32_t event,void *arg);
#endif
#endif