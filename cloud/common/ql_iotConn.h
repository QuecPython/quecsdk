/*
 * @Author: your name
 * @Date: 2021-11-04 19:23:58
 * @LastEditTime: 2021-11-10 08:52:02
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \QuecCSDK\cloud\common\ql_iotConn.h
 */
#ifndef __QIOT_CONN_H__
#define __QIOT_CONN_H__
#include "Ql_iotApi.h"

#define QIOT_CONN_NET_ERRTIME_MAX 6 /* 连续网络异常次数，重启网络 */
#define QIOT_AUTH_ERR_STRING(X)                                                                               \
    (                                                                                                         \
        (X == QIOT_AUTH_SUCC) ? "AUTH_SUCC" : (X == QIOT_AUTH_ERR_REQDATA)       ? "AUTH_ERR_REQDATA"         \
                                          : (X == QIOT_AUTH_ERR_DONE)            ? "AUTH_ERR_DONE"            \
                                          : (X == QIOT_AUTH_ERR_PKPS_INVALID)    ? "AUTH_ERR_PKPS_INVALID"    \
                                          : (X == QIOT_AUTH_ERR_PAYLOAD_INVALID) ? "AUTH_ERR_PAYLOAD_INVALID" \
                                          : (X == QIOT_AUTH_ERR_SIGN_INVALID)    ? "AUTH_ERR_SIGN_INVALID"    \
                                          : (X == QIOT_AUTH_ERR_VERSION_INVALID) ? "AUTH_ERR_VERSION_INVALID" \
                                          : (X == QIOT_AUTH_ERR_HASH_INVALID)    ? "AUTH_ERR_HASH_INVALID"    \
                                          : (X == QIOT_AUTH_ERR_PK_CHANGE)       ? "AUTH_ERR_PK_CHANGE"       \
                                          : (X == QIOT_AUTH_ERR_DK_ILLEGAL)      ? "AUTH_ERR_DK_ILLEGAL"      \
                                          : (X == QIOT_AUTH_ERR_PK_VER_NOCOR)    ? "AUTH_ERR_PK_VER_NOCOR"    \
                                          : (X == QIOT_AUTH_ERR_DEVICE_INSIDE)   ? "AUTH_ERR_DEVICE_INSIDE"   \
                                          : (X == QIOT_AUTH_ERR_SERVER_NOTFOUND) ? "AUTH_ERR_SERVER_NOTFOUND" \
                                          : (X == QIOT_AUTH_ERR_FAIL)            ? "AUTH_ERR_FAIL"            \
                                          : (X == QIOT_AUTH_ERR_UNKNOWN)         ? "AUTH_ERR_UNKNOWN"         \
                                                                                 : "Unknown")

#define QIOT_CONN_ERR_STRING(X)                                                                                   \
    (                                                                                                             \
        (X == QIOT_CONN_SUCC) ? "CONN_SUCC" : (X == QIOT_CONN_ERR_DS_INVALID)     ? "CONN_ERR_DS_INVALID"         \
                                          : (X == QIOT_CONN_ERR_DEVICE_FORBID)    ? "CONN_ERR_DEVICE_FORBID"      \
                                          : (X == QIOT_CONN_ERR_DEVICE_INSIDE)    ? "CONN_ERR_DEVICE_INSIDE"      \
                                          : (X == QIOT_CONN_ERR_VERSION_NOTFOUND) ? "CONN_ERR_VERSION_NOTFOUND"   \
                                          : (X == QIOT_CONN_ERR_PING)             ? "CONN_ERR_PING"               \
                                          : (X == QIOT_CONN_ERR_NET)              ? "CONN_ERR_NET"                \
                                          : (X == QIOT_CONN_ERR_SERVER_CHANGE)    ? "QIOT_CONN_ERR_SERVER_CHANGE" \
                                          : (X == QIOT_CONN_ERR_UNKNOW)           ? "CONN_ERR_UNKNOW"             \
                                                                                  : "Unknown")

#define QIOT_SUBCRIBE_ERR_STRING(X)                                                             \
    (                                                                                           \
        (X == QIOT_SUBCRIBE_SUCC) ? "SUBCRIBE_SUCC" : (X == QIOT_SUBCRIBE_ERR) ? "SUBCRIBE_ERR" \
                                                                               : "Unknown")

#define QIOT_LOGOUT_ERR_STRING(X) \
    (                             \
        (X == QIOT_LOGOUT_SUCC) ? "LOGOUT_SUCC" : "Unknown")

#define QIOT_CONN_MODE_STRING(X)                                                                 \
    (                                                                                            \
        (X == QIOT_CONNMODE_IDLE) ? "CONNMODE_IDLE" : (X == QIOT_CONNMODE_REQ) ? "CONNMODE_REQ"  \
                                                  : (X == QIOT_CONNMODE_AUTO)  ? "CONNMODE_AUTO" \
                                                                               : "Unknown")
void Ql_iotConnInit(void);
qbool Ql_iotConnSend(const char *endPoint, quint16_t mode, quint16_t cmd, quint16_t srcpkgId, quint16_t pkgId, const quint8_t *payload, quint32_t payloadLen, socketRecvNodeCb_f recvCB);
#ifdef QUEC_ENABLE_GATEWAY
#endif
#endif