#ifndef __QIOT_CMDBUS_H__
#define __QIOT_CMDBUS_H__
#include "Ql_iotApi.h"

#define QIOT_RECV_NODE_TRANS_MAX 10
#define QIOT_RECV_NODE_MODEL_MAX 10

#define QIOT_SEND_ERR_STRING(X)                                                                                 \
    (                                                                                                           \
        (X == QIOT_SEND_SUCC_TRANS) ? "SEND_SUCC_TRANS" : (X == QIOT_SEND_SUCC_PHYMODEL) ? "SEND_SUCC_PHYMODEL" \
                                                      : (X == QIOT_SEND_ERR_TRANS)       ? "SEND_ERR_TRANS"     \
                                                      : (X == QIOT_SEND_ERR_PHYMODEL)    ? "SEND_ERR_PHYMODEL"  \
                                                                                         : "Unknown")
#define QIOT_RECV_ERR_STRING(X)                                                                                           \
    (                                                                                                                     \
        (X == QIOT_RECV_SUCC_TRANS) ? "RECV_SUCC_TRANS" : (X == QIOT_RECV_SUCC_PHYMODEL_RECV) ? "RECV_SUCC_PHYMODEL_RECV" \
                                                      : (X == QIOT_RECV_SUCC_PHYMODEL_REQ)    ? "RECV_SUCC_PHYMODEL_REQ"  \
                                                      : (X == QIOT_RECV_ERR_BUFFER)           ? "RECV_ERR_BUFFER"         \
                                                      : (X == QIOT_RECV_ERR_LIMIT)            ? "RECV_ERR_LIMIT"          \
                                                                                              : "Unknown")

typedef struct
{
    TWLLHead_T head;
    int type;
    struct
    {
        quint32_t len;
        quint8_t buf[1];
    } val;
} QIot_buffer_t;

typedef struct
{
    qbool recvIsBuffer;
    TWLLHead_T *recvTransData;
    TWLLHead_T *recvPhymodelData;
    qbool dataFormat;
} Ql_iotCmdBusInfo_t;
extern Ql_iotCmdBusInfo_t QIot_busInfo;
void Ql_iotCmdBusInit(void);

void Ql_iotCmdBusPassTransRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
void Ql_iotCmdBusPhymodelWriteRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
void Ql_iotCmdBusPhymodelReqRecv(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);

qbool Ql_iotCmdBusPhymodelReportHex(quint16_t mode, quint8_t *buf, quint32_t len);
qbool Ql_iotCmdBusPhymodelAckHex(quint16_t mode, quint16_t pkgId, quint8_t *buf, quint32_t len);
#endif