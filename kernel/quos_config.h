/*
 * @Author: your name
 * @Date: 2021-06-21 10:50:46
 * @LastEditTime: 2021-07-07 19:17:00
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \quecthing_pythonSDK\components\quecsdk\kernel\quos_config.h
 */
#ifndef __QUOS_CONFIG_H__
#define __QUOS_CONFIG_H__
#include "Qhal_types.h"

#define SOCKET_FD_INVALID                   ((pointer_t)-1)
#define QUOS_DNS_HOSTNANE_MAX_LENGHT        (64) /* DNS�涨�������ܳ���63�ַ�*/
#define QUOS_IP_ADDR_MAX_LEN                (46)

enum
{
    QUOS_SYSTEM_EVENT_NETCONNECTED = -1,
    QUOS_SYSTEM_EVENT_NETDISCONNECT = -2,
};
/* SDK ֧�ֹ������� */
#define SDK_ENABLE_TLS      1
#define SDK_ENABLE_HTTP     1
#define SDK_ENABLE_LWM2M    0
#define SDK_ENABLE_COAP     0
#define SDK_ENABLE_MQTT     1
#define SDK_ENABLE_SHA1     1
#define SDK_ENABLE_SHA256   1
#define SDK_ENABLE_MD5      1
#define SDK_ENABLE_BASE64   1
#define SDK_ENABLE_AES      1
#define SDK_ENABLE_EVENT    1
#define SDK_ENABLE_SIGNAL   1
#define SDK_ENABLE_TIMER    1
#define SDK_ENABLE_JSON     1
#define SDK_ENABLE_DATASAFE 1
#define SDK_ENABLE_LOGDUMP  0

/* LOGģ�� PRINTF�ȼ����� */
#define LSDK_STORE LL_ERR
#define LSDK_COAP  LL_ERR
#define LSDK_ENCRP LL_ERR
#define LSDK_EVENT LL_DBG
#define LSDK_HTTP  LL_DBG
#define LSDK_MQTT  LL_DBG
#define LSDK_SIG   LL_DBG
#define LSDK_SOCK  LL_DBG
#define LSDK_TIMER LL_DBG
#define LSDK_LWM2M LL_ERR
#define LSDK_NET   LL_ERR
#endif
