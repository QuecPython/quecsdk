#ifndef __QHAL_SOCKET_H__
#define __QHAL_SOCKET_H__
#include "Qhal_types.h"

enum
{
    SOCKET_TYPE_UART = 0,
    SOCKET_TYPE_UDP,        /* UDP */
    SOCKET_TYPE_TCP_LISTEN,
    SOCKET_TYPE_TCP_CLI,    /* TCP CLIENT */
    SOCKET_TYPE_TCP_SSL_CLI,/* TCP CLIENT + TLS */
    SOCKET_TYPE_UDP_SSL_CLI,/* UDP CLIENT + DTLS */
};

pointer_t Qhal_tcpServerInit(quint8_t *type, quint16_t l_port, quint8_t maxClient);
pointer_t Qhal_tcpClientInit(quint8_t *type, const char *hostname, quint16_t r_port, quint32_t *connectTimeout);
pointer_t Qhal_udpInit(quint8_t *type, quint16_t l_port, const char *hostname, quint16_t r_port, void **peer);
pointer_t Qhal_tcpSslClientInit(quint8_t *type, const char *hostname, quint16_t r_port, quint32_t *connectTimeout);
pointer_t Qhal_udpSslInit(quint8_t *type, quint16_t l_port, const char *hostname, quint16_t r_port);
qbool Qhal_sockWrite(pointer_t sockFd, quint8_t type, const void *peer, const quint8_t *buf, quint16_t bufLen, qbool *isSending);
void Qhal_sockClose(pointer_t sockFd, quint8_t type);

quint32_t FUNCTION_ATTR_ROM Qhal_dns2IPGet(const char *hostname, quint8_t **retAddr, quint32_t addrMax);
#endif
