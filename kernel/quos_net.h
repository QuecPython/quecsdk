#ifndef __QUOS_NET_H__
#define __QUOS_NET_H__
#include "quos_config.h"
void Quos_netOpen(void);
void Quos_netClose(void);
void Quos_netIOStatusNotify(qbool result);

#define DNS_IP_FROM_HOSTNAME_MAX_NUM        (3)			/* 从DNS中获取ip个数 */

void Quos_netHostnameSetDefault(const char *hostname, const char *ip);
quint8_t *Quos_netGetIpFromHostname(const char *hostname);
void Quos_netHostnameValidIpNumSet(const char *hostname);
qbool Quos_netHostnameValidIpGet(const char *hostname, char *ip);
void Quos_netHostnameDnsEnable(const char *hostname);
void Quos_netHostnameDelete(const char *hostname);
#endif