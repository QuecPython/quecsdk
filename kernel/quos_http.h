#ifndef __QUOS_HTTP_H__
#define __QUOS_HTTP_H__
#include "quos_config.h"
#if (SDK_ENABLE_HTTP==1)
#define QUOS_HTTP_MULTIPART_BOUNDARY              "450d2e46-73fc11eaad264b91df3ae910"
#define QUOS_HTTP_MULTIPART_NODE_START            "--" QUOS_HTTP_MULTIPART_BOUNDARY "\r\n"
#define QUOS_HTTP_MULTIPART_NODE_END              "\r\n--" QUOS_HTTP_MULTIPART_BOUNDARY "--\r\n"

#define QUOS_HTTP_HEAD_CONTENT_LENGHT             "Content-Length: %u\r\n"

#define QUOS_HTTP_CONTENT_TYPE_KEY                "Content-Type: "
#define QUOS_HTTP_CONTENT_TYPE_VALUE_JSON         "application/json\r\n"
#define QUOS_HTTP_CONTENT_TYPE_VALUE_MULTIPART    "multipart/form-data; boundary=" QUOS_HTTP_MULTIPART_BOUNDARY "\r\n"
#define QUOS_HTTP_CONTENT_TYPE_VALUE_OCTET_STREAM "application/octet-stream\r\n"

#define QUOS_HTTP_CONTENT_DISPOSITION_KEY         "Content-Disposition: "

enum
{
    QUOS_HTTP_CODE_ERR_DATA = -255,
    QUOS_HTTP_CODE_ERR_RAM,
    QUOS_HTTP_CODE_ERR_NET,
};

typedef struct
{
    char *rawHeaders; /* ÿ����"\r\n"���� */
    char *payload;
    quint16_t payloadLen;
} HttpReqData_t;

typedef struct
{
    char *txName;
    char *rxName;
    quint32_t size;
    quint32_t offset;
    qbool isPostForm;
} HttpReqFile_t;

/* �ڷ������ļ�ʱ��ֻ���������HTTP��CBһ��
   �������ļ�ʱ��������ȡ��header����ʱ��CBһ�Σ�recvLen=0,
   ����CB���ؽ���ж��Ƿ������ΪTRUEʱ����������ļ���
   HTTP��������CBһ�Σ���ʱrecvLenΪ���ص����ļ���С */
typedef qbool (*httpEventCB_f)(qint32_t httpCode, char *retHeader, quint8_t *recvBuf, quint32_t recvLen);

qbool Quos_httpRequest(const char *opt, const char *url, httpEventCB_f eventCB, const HttpReqData_t *reqData, const HttpReqFile_t *reqFile);
qbool Quos_httpGetDownload(const char *url, httpEventCB_f eventCB, const HttpReqData_t *reqData, const char *filename, quint32_t offset);
qbool Quos_httpPostForm(const char *url, httpEventCB_f eventCB, const HttpReqData_t *reqData, const char *filename, quint32_t fileSize);

#define Quos_httpGet(URL, EVENTCB, REQDATA) Quos_httpRequest("GET", URL, EVENTCB, REQDATA, NULL)
#define Quos_httpPost(URL, EVENTCB, REQDATA) Quos_httpRequest("POST", URL, EVENTCB, REQDATA, NULL)
#define Quos_httpPut(URL, EVENTCB, REQDATA) Quos_httpRequest("PUT", URL, EVENTCB, REQDATA, NULL)
#define Quos_httpDelete(URL, EVENTCB, REQDATA) Quos_httpRequest("DELETE", URL, EVENTCB, REQDATA, NULL)
#endif
#endif
