#ifndef __QUOS_SOCKET_H__
#define __QUOS_SOCKET_H__
#include "quos_config.h"
#include "quos_twll.h"
#include "quos_swTimer.h"
typedef struct
{
    quint16_t bufLen;
    quint8_t *Buf;
} Quos_socketRecvDataNode_t; /* 接收包节点 */

typedef qbool (*socketsendNodeCb_f)(void *chlFd, const void *sendData, qbool result);
typedef void (*socketRecvNodeCb_f)(void *chlFd, const void *sendData, const void *recvData);
typedef struct
{
    TWLLHead_T head;
    quint8_t sendCnt;
    quint16_t bufLen;
    quint32_t sendTimeout;
    quint32_t pkgId;
    quint8_t *Buf;
    void *param;
    void *peer;
    socketsendNodeCb_f sendCB;
    socketRecvNodeCb_f recvCB; /* 等待接收超时或成功 */
} Quos_socketSendDataNode_t;   /* 发送包节点 */

typedef struct
{
    quint8_t temp;
    quint8_t *buf;
    quint32_t offset;
    quint32_t bufLen;
} Quos_socketTempData_t;

typedef qbool (*socketUnform_f)(const quint8_t *buf, quint32_t bufLen, quint32_t *offset, Quos_socketTempData_t *unformTemp);
typedef qbool (*socketRecvNotify_f)(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData);
typedef qbool (*socketConnNotify_f)(void *chlFd, qbool result);
typedef qbool (*socketSendIO_f)(pointer_t sockFd, quint8_t type, const void *peer, const quint8_t *buf, quint16_t bufLen, qbool *isSending);
typedef void (*socketCloseIO_f)(pointer_t sockFd, quint8_t type);
typedef void (*socketParamFree_f)(void *param);
typedef struct
{
    TWLLHead_T head;
    pointer_t sockFd;
    quint8_t type;
    qbool valid;
    void *param;
    void **self;
    socketRecvNotify_f recvDataFunc;
    socketUnform_f unformFunc;  /* 此回调API内不允许调用Quos_socket类API，否则会引起死锁 */
    Quos_socketTempData_t unformTemp;
    socketParamFree_f paramFree;
    struct
    {
        socketSendIO_f send;
        socketCloseIO_f close;
    } io; /* 此IO类CB里面不允许调用Quos_socket类API，否则会引起死锁 */
    struct
    {
        socketConnNotify_f notify;
        quint32_t timeout;
        void *timer;
    } conn;
    struct
    {
        quint8_t txCnt;
        quint8_t waitIoSendAck;
        qbool waitPkgAck;
        quint32_t timeout;
        quint32_t sendInterVal; /* 数据发送最小间隔，用于限速 */
        Systick_T beginTime;
        Systick_T recvTime;
        TWLLHead_T *orderList;    /* 有序发送列表 */
        TWLLHead_T *disorderList; /* 无序发送列表，必须是无需等待应答数据;有数据会优先于有序发送列表，即使有序发送列表还在发送中 */
    } send;
} Quos_socketChlInfoNode_t;

void Quos_socketInit(void);
void *Quos_socketChannelAdd(void **chlFdPoint, Quos_socketChlInfoNode_t chlInfo);
qbool Quos_socketChannelDel(void *chlFd);
void Quos_socketIOConnResult(pointer_t sockFd, quint8_t type, qbool result);
void Quos_socketIORx(pointer_t sockFd, quint8_t type, const void *peer, quint32_t peerSize, quint8_t *Buf, quint32_t bufLen);
void Quos_socketIOTxCb(pointer_t sockFd, quint8_t type);
qbool Quos_socketTx(const void *chlFd, void *peer, quint8_t sendCnt, quint32_t sendTimeout, socketsendNodeCb_f sendCB, socketRecvNodeCb_f recvCB,
                    quint32_t pkgId, quint8_t *buf, quint16_t bufLen, const void *param);
qbool Quos_socketTxDisorder(const void *chlFd, void *peer, quint8_t *buf, quint16_t bufLen);
qbool Quos_socketTxAck(void *chlFd, const void *peer, quint32_t pkgId, const void *recvData);
qbool Quos_socketGetSockFdType(void *chlFd, pointer_t *sockFd, quint8_t *type);
void *Quos_socketGetChlFd(pointer_t sockFd, quint8_t type);
quint32_t Quos_socketGetChlFdList(quint8_t type, const void *param, void *chlFd[], quint32_t maxSize);
qbool Quos_socketCheckChlFd(const void *chlFd);
TWLLHead_T *Quos_socketGetChlHead(void);
qbool Quos_socketInfoModity(void *chlFd, quint8_t sendCnt, quint32_t sendTimeout, socketRecvNotify_f recvDataFunc);
quint32_t Quos_socketSendDataByteSize(void *chlFd);
quint32_t Quos_socketTask(void);
#endif
