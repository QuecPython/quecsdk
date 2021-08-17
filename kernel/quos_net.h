#ifndef __QUOS_NET_H__
#define __QUOS_NET_H__
#include "quos_config.h"
void Quos_netOpen(void);
void Quos_netClose(void);
void Quos_netIOStatusNotify(qbool result);
#endif