#ifndef __QUOS_CONFIG_H__
#define __QUOS_CONFIG_H__
#include "Qhal_types.h"

#define SOCKET_FD_INVALID                   ((pointer_t)-1)
#define QUOS_DNS_HOSTNANE_MAX_LENGHT        (64) /* DNS规定域名不能超过63字符*/
#define QUOS_IP_ADDR_MAX_LEN                (46)
enum
{
    QUOS_SYSTEM_EVENT_NETWORK = -1,
};
enum
{
    QUOS_SEVENT_NET_CONNECTED = 0,
    QUOS_SEVENT_NET_DISCONNECT = 1,
    QUOS_SEVENT_NET_CONNTIMEOUT = 2,
};
/* SDK 支持功能配置 */
#ifndef SDK_ENABLE_TLS
#define SDK_ENABLE_TLS      1
#endif
#ifndef SDK_ENABLE_HTTP
#define SDK_ENABLE_HTTP     1
#endif
#ifndef SDK_ENABLE_LWM2M
#define SDK_ENABLE_LWM2M    0
#endif
#ifndef SDK_ENABLE_COAP
#define SDK_ENABLE_COAP     0
#endif
#ifndef SDK_ENABLE_MQTT
#define SDK_ENABLE_MQTT     1
#endif
#ifndef SDK_ENABLE_SHA1
#define SDK_ENABLE_SHA1     0
#endif
#ifndef SDK_ENABLE_SHA256
#define SDK_ENABLE_SHA256   1
#endif
#ifndef SDK_ENABLE_MD5
#define SDK_ENABLE_MD5      1
#endif
#ifndef SDK_ENABLE_BASE64
#define SDK_ENABLE_BASE64   1
#endif
#ifndef SDK_ENABLE_AES
#define SDK_ENABLE_AES      1
#endif
#ifndef SDK_ENABLE_EVENT
#define SDK_ENABLE_EVENT    1
#endif
#ifndef SDK_ENABLE_FIFO
#define SDK_ENABLE_FIFO     0
#endif
#ifndef SDK_ENABLE_SIGNAL
#define SDK_ENABLE_SIGNAL   1
#endif
#ifndef SDK_ENABLE_TIMER
#define SDK_ENABLE_TIMER    1
#endif
#ifndef SDK_ENABLE_JSON
#define SDK_ENABLE_JSON     1
#endif
#ifndef SDK_ENABLE_DATASAFE
#define SDK_ENABLE_DATASAFE 1
#endif
#ifndef SDK_ENABLE_TWLL
#define SDK_ENABLE_TWLL     1
#endif

/* LOG模块 PRINTF等级配置 */
#define LSDK_STORE LL_DBG
#define LSDK_COAP  LL_DBG
#define LSDK_ENCRP LL_DBG
#define LSDK_EVENT LL_DBG
#define LSDK_HTTP  LL_DBG
#define LSDK_MQTT  LL_DBG
#define LSDK_SIG   LL_DBG
#define LSDK_SOCK  LL_DBG
#define LSDK_TIMER LL_DBG
#define LSDK_LWM2M LL_DBG
#define LSDK_NET   LL_DBG
#endif
