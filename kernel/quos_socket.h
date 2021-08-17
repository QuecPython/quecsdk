#ifndef __QUOS_SOCKET_H__
#define __QUOS_SOCKET_H__
#include "quos_config.h"
#include "quos_twll.h"
#include "quos_swTimer.h"
typedef struct
{
    quint16_t bufLen;
    quint8_t *Buf;
    void *peer;
} Quos_socketRecvDataNode_t; /* ���հ��ڵ� */

typedef struct
{
    TWLLHead_T head;
    quint8_t sendCnt;
    quint32_t sendTimeout;
    qbool (*sendCB)(void *chlFd, const void *sendData, qbool result);
    void (*recvCB)(void *chlFd, const void *sendData, const Quos_socketRecvDataNode_t *recvData); /* �ȴ����ճ�ʱ��ɹ� */
    void *param;
    quint32_t pkgId;
    quint16_t bufLen;
    quint8_t *Buf;
} Quos_socketSendDataNode_t; /* ���Ͱ��ڵ� */
typedef qbool (*socketsendNodeCb_f)(void *chlFd, const void *sendData, qbool result);
typedef void (*socketRecvNodeCb_f)(void *chlFd, const void *sendData, const Quos_socketRecvDataNode_t *recvData);

typedef struct
{
    quint8_t *buf;
    quint32_t offset;
    quint32_t bufLen;
} Quos_socketTempData_t;

typedef qbool (*socketUnform_f)(quint8_t *buf, quint32_t bufLen, quint32_t *offset, Quos_socketTempData_t *unformTemp);
typedef qbool (*socketRecvNotify_f)(void *chlFd, Quos_socketRecvDataNode_t *recvData);
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
    socketUnform_f unformFunc;
    Quos_socketTempData_t unformTemp;
    socketParamFree_f paramFree;
    struct
    {
        socketSendIO_f send;
        socketCloseIO_f close;
        void *peer;
    } io; /* ��IO��CB���治�������Quos_socket��API��������������� */
    struct
    {
        socketConnNotify_f notify;
        quint32_t timeout;
        SWTimer_T *timer;
    } conn;
    struct
    {
        quint8_t txCnt;
        quint32_t timeout;
        quint32_t sendInterVal; /* ���ݷ�����С������������� */
        Systick_T beginTime;
        Systick_T recvTime;
        quint8_t waitIoSendAck;
        qbool waitPkgAck;
        TWLLHead_T *orderList;    /* �������б� */
        TWLLHead_T *disorderList; /* �������б�����������ȴ�Ӧ������;�����ݻ��������������б���ʹ�������б��ڷ�����,���ڲ��Դ�����������������������ʹ�ô����� */
    } send;
} Quos_socketChlInfoNode_t;

void Quos_socketInit(void);
void *Quos_socketChannelAdd(void **chlFdPoint, Quos_socketChlInfoNode_t chlInfo);
qbool Quos_socketChannelDel(void *chlFd);
void Quos_socketIOConnResult(pointer_t sockFd, quint8_t type, qbool result);
void Quos_socketIORx(pointer_t sockFd, quint8_t type, const void *peer, quint8_t *Buf, quint32_t bufLen);
void Quos_socketIOTxCb(pointer_t sockFd, quint8_t type);
qbool Quos_socketTx(const void *chlFd, quint8_t sendCnt, quint32_t sendTimeout, socketsendNodeCb_f sendCB, socketRecvNodeCb_f recvCB,
                    quint32_t pkgId, quint8_t *buf, quint16_t bufLen, const void *param);
qbool Quos_socketTxDisorder(const void *chlFd, quint8_t *buf, quint16_t bufLen);
qbool Quos_socketTxAck(void *chlFd, quint32_t pkgId, const void *recvData);
qbool Quos_socketGetSockFdType(void *chlFd, pointer_t *sockFd, quint8_t *type);
void *Quos_socketGetChlFd(pointer_t sockFd, quint8_t type);
TWLLHead_T *Quos_socketGetChlHead(void);
qbool Quos_socketInfoModity(void *chlFd, quint8_t sendCnt, quint32_t sendTimeout, socketRecvNotify_f recvDataFunc);
quint32_t Quos_socketSendDataByteSize(void *chlFd);
quint32_t Quos_socketTask(void);
#endif
