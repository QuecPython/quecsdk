/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : Httpͨ�Ź���
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
***************************************************************************/
#include "quos_http.h"
#if (SDK_ENABLE_HTTP == 1)
#include "Quos_kernel.h"
#include "Qhal_driver.h"
#ifndef QUOS_HTTP_TIMEOUT
#define QUOS_HTTP_TIMEOUT 10 * SWT_ONE_SECOND
#endif
#ifndef QUOS_HTTP_FILE_PIECE
#define QUOS_HTTP_FILE_PIECE 1024
#endif
#define HTTP_DEFAULT_HEADER "%s /%s HTTP/1.1\r\nHost:%s:%d\r\n%s"

enum
{
    HTTP_BODY_TYPE_INVALID = -3,
    HTTP_BODY_TYPE_NOMASK = -2,
    HTTP_BODY_TYPE_CHUNKED = -1,
    HTTP_BODY_TYPE_CONTENTLENGHT,
};
typedef struct
{
    httpEventCB_f eventCB;
    char *sendFilename;
    char *recvFilename;
    qint32_t httpCode;
    qint32_t bodyType;
    char *retHeader;
    quint32_t bodyLen;
    quint32_t bodyOffset;
    quint32_t chunkedBlockLen;
    pointer_t fileFd;
} HttpSocket_t;

/**************************************************************************
** ����	@brief : http socket connet���
** ����	@param :
** ���	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_httpSockConnctCB(void *chlFd, qbool result)
{
    Quos_logPrintf(LSDK_HTTP, LL_DBG, "chlFd[%p] result:%s", chlFd, _BOOL2STR(result));
    if (result == FALSE && NULL != chlFd)
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        ((HttpSocket_t *)(chlNode->param))->eventCB(QUOS_HTTP_CODE_ERR_NET, NULL, NULL, 0);
    }
    return TRUE;
}
/**************************************************************************
** ����	@brief : HTTP send CB
** ����	@param :
** ���	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_httpSendCB(void *chlFd, const void *sendData, const void *recvData)
{
    UNUSED(sendData);
    if (NULL == recvData)
    {
        Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
        HttpSocket_t *httpSocket = (HttpSocket_t *)chlNode->param;
        httpSocket->eventCB(QUOS_HTTP_CODE_ERR_NET, NULL, NULL, 0);
        Quos_socketChannelDel((void *)chlNode);
    }
}
/**************************************************************************
** ����	@brief : HTTP ʹ��TCP����
** ����	@param :
** ���	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_httpSendResult(void *chlFd, const void *sendData, qbool result)
{
    Quos_logPrintf(LSDK_HTTP, LL_DBG, "chlFd[%p] result:%s", chlFd, _BOOL2STR(result));
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    Quos_socketSendDataNode_t *sendNode = (Quos_socketSendDataNode_t *)sendData;
    HttpSocket_t *httpSocket = (HttpSocket_t *)chlNode->param;
    if (TRUE == result)
    {
        if (SOCKET_FD_INVALID == httpSocket->fileFd)
        {
            httpSocket->fileFd = Qhal_fileOpen(httpSocket->sendFilename, TRUE);
            if (SOCKET_FD_INVALID == httpSocket->fileFd)
            {
                Quos_socketChannelDel((void *)chlNode);
                return FALSE;
            }
        }
        if (httpSocket->bodyLen > httpSocket->bodyOffset)
        {
            quint32_t len = httpSocket->bodyLen - httpSocket->bodyOffset;
            len = len > QUOS_HTTP_FILE_PIECE ? QUOS_HTTP_FILE_PIECE : len;
            quint8_t *buf = HAL_MALLOC(len);
            if (NULL == buf)
            {
                Quos_logPrintf(LSDK_HTTP, LL_ERR, "mcf buf to file piece len:%u", len);
            }
            else if (Qhal_fileRead(httpSocket->fileFd, httpSocket->bodyOffset, buf, len) != len)
            {
                HAL_FREE(buf);
                Quos_logPrintf(LSDK_HTTP, LL_ERR, "read file piece len fail");
            }
            else if (FALSE == Quos_socketTx(chlFd, NULL, sendNode->sendCnt, sendNode->sendTimeout, (socketsendNodeCb_f)sendNode->sendCB, (socketRecvNodeCb_f)sendNode->recvCB, sendNode->pkgId, buf, len, NULL))
            {
                Quos_logPrintf(LSDK_HTTP, LL_ERR, "add file piece to socket sendlist fail");
            }
            else
            {
                httpSocket->bodyOffset += len;
                Quos_logPrintf(LSDK_HTTP, LL_DBG, "sendFile:%u/%u", httpSocket->bodyOffset, httpSocket->bodyLen);
                return TRUE;
            }
        }
        else
        {
            Qhal_fileClose(httpSocket->fileFd);
            httpSocket->fileFd = SOCKET_FD_INVALID;
            quint8_t *buf = HAL_MALLOC(sizeof(QUOS_HTTP_MULTIPART_NODE_END));
            if (NULL == buf)
            {
                Quos_logPrintf(LSDK_HTTP, LL_ERR, "mcf buf to multipart end mask");
                return FALSE;
            }
            HAL_SPRINTF((char *)buf, "%s", QUOS_HTTP_MULTIPART_NODE_END);
            if (FALSE == Quos_socketTx(chlFd, NULL, sendNode->sendCnt, sendNode->sendTimeout, NULL, (socketRecvNodeCb_f)sendNode->recvCB, sendNode->pkgId, buf, HAL_STRLEN(buf), NULL))
            {
                Quos_logPrintf(LSDK_HTTP, LL_ERR, "add multipart end mask to socket sendlist fail");
            }
            else
            {
                Quos_logPrintf(LSDK_HTTP, LL_INFO, "send multipart end mask");
                return TRUE;
            }
        }
    }
    Quos_socketChannelDel(chlFd);
    return FALSE;
}
/**************************************************************************
** ����	@brief : http��������
** ����	@param :
** ���	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_httpRecvData(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData)
{
    UNUSED(peer);
    UNUSED(peerSize);
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    HttpSocket_t *httpSocket = (HttpSocket_t *)chlNode->param;
    /* socket���Զ˶Ͽ�������bufLen=0 */
    if (NULL == recvData)
    {
        /* ���body���Ͳ�ָ���Ҳ��ñ���Ϊ�ļ�,����socket�Ͽ���Ϊ������ɱ�ʶ */
        Quos_logPrintf(LSDK_HTTP, LL_ERR, "recv buf len= 0");
        if (NULL == httpSocket->recvFilename && HTTP_BODY_TYPE_NOMASK == httpSocket->bodyType && httpSocket->bodyLen > 0)
        {
            httpSocket->eventCB(httpSocket->httpCode, httpSocket->retHeader, chlNode->unformTemp.buf, httpSocket->bodyLen);
        }

        /* ����������ļ���������Ͽ�����֪ͨӦ�ò������쳣���ѽ��յ����ֽ���������Ӧ�ò����ϵ����� */
        else if (NULL != httpSocket->recvFilename && HTTP_BODY_TYPE_INVALID != httpSocket->bodyType && httpSocket->bodyLen > 0)
        {
            httpSocket->eventCB(QUOS_HTTP_CODE_ERR_NET, httpSocket->retHeader, (quint8_t *)httpSocket->recvFilename, httpSocket->bodyLen);
        }
        else
        {
            httpSocket->eventCB(QUOS_HTTP_CODE_ERR_NET, httpSocket->retHeader, NULL, 0);
        }
        if (SOCKET_FD_INVALID != httpSocket->fileFd)
        {
            Qhal_fileClose(httpSocket->fileFd);
            httpSocket->fileFd = SOCKET_FD_INVALID;
        }
        return FALSE;
    }
    Quos_logHexDump(LSDK_HTTP, LL_DUMP, "RECV", recvData->Buf, recvData->bufLen);
    /* �Ƚ��������ݷ��뻺������ */
    if (recvData->bufLen + chlNode->unformTemp.offset + 1 > chlNode->unformTemp.bufLen)
    {
        quint8_t *tempBuf = HAL_MALLOC(recvData->bufLen + chlNode->unformTemp.offset + 1);
        if (NULL == tempBuf)
        {
            httpEventCB_f cb = httpSocket->eventCB;
            Quos_socketChannelDel(chlFd);
            cb(QUOS_HTTP_CODE_ERR_RAM, NULL, NULL, 0);
            return FALSE;
        }
        HAL_MEMCPY(tempBuf, chlNode->unformTemp.buf, chlNode->unformTemp.offset);
        HAL_FREE(chlNode->unformTemp.buf);
        chlNode->unformTemp.buf = tempBuf;
        chlNode->unformTemp.bufLen = recvData->bufLen + chlNode->unformTemp.offset + 1;
    }
    HAL_MEMCPY(chlNode->unformTemp.buf + chlNode->unformTemp.offset, recvData->Buf, recvData->bufLen);
    chlNode->unformTemp.offset += recvData->bufLen;
    chlNode->unformTemp.buf[chlNode->unformTemp.offset] = 0;

    /* ����HTTP ����HEADER */
    if (HTTP_BODY_TYPE_INVALID == httpSocket->bodyType)
    {
        char *headSplit = HAL_STRSTR(chlNode->unformTemp.buf, "\r\n\r\n");
        if (NULL == headSplit)
        {
            Quos_logPrintf(LSDK_HTTP, LL_DBG, "http head find invalid");
            return FALSE;
        }
        headSplit+=2;
        HAL_MEMSET(headSplit, 0, HAL_STRLEN("\r\n"));

        if (0 != HAL_STRNCMP(chlNode->unformTemp.buf, "HTTP/1.1 ", HAL_STRLEN("HTTP/1.1 ")))
        {
            httpEventCB_f cb = httpSocket->eventCB;
            char *retHeader = httpSocket->retHeader;
            httpSocket->retHeader = NULL;
            Quos_socketChannelDel(chlFd);
            cb(QUOS_HTTP_CODE_ERR_DATA, retHeader, NULL, 0);
            HAL_FREE(retHeader);
            return FALSE;
        }

        httpSocket->bodyLen = 0;
        httpSocket->chunkedBlockLen = 0;
        httpSocket->httpCode = HAL_ATOI((char *)chlNode->unformTemp.buf + HAL_STRLEN("HTTP/1.1 "));

        /* ����body��ʽ���� */
        char *pdata;
        if ((pdata = HAL_STRSTR(chlNode->unformTemp.buf, "Content-Length: ")) != NULL)
        {
            httpSocket->bodyType = (qint32_t)HAL_ATOI(pdata + HAL_STRLEN("Content-Length: "));
        }
        else if ((pdata = HAL_STRSTR(chlNode->unformTemp.buf, "Transfer-Encoding: chunked\r\n")) != NULL)
        {
            httpSocket->bodyType = HTTP_BODY_TYPE_CHUNKED;
        }
        else
        {
            httpSocket->bodyType = HTTP_BODY_TYPE_NOMASK;
        }

        /*if ((pdata = HAL_STRSTR(chlNode->unformTemp.buf, "Content-Range: bytes ")) != NULL)
        {
            httpSocket->bodyOffset = HAL_ATOI(pdata + HAL_STRLEN("Content-Range: bytes "));
        }*/
        /* ��http ����header���ظ�Ӧ�ò� */
        quint32_t headerLen = headSplit - (char *)chlNode->unformTemp.buf;
        Quos_logPrintf(LSDK_HTTP, LL_DBG, "find http head ok,code:%d bodyType:%d bodyOffset:%u header len:%u", httpSocket->httpCode, httpSocket->bodyType, httpSocket->bodyOffset, headerLen);
        Quos_logPrintf(LSDK_HTTP, LL_DBG, "%s", chlNode->unformTemp.buf);
        httpSocket->retHeader = HAL_MALLOC(headerLen + 1);
        if (NULL == httpSocket->retHeader)
        {
            Quos_socketChannelDel(chlFd);
            return TRUE;
        }
        /* ɾ����������header���� */
        HAL_MEMCPY(httpSocket->retHeader, chlNode->unformTemp.buf, headerLen);
        httpSocket->retHeader[headerLen] = 0;
        chlNode->unformTemp.offset -= headerLen + HAL_STRLEN("\r\n");
        HAL_MEMMOVE(chlNode->unformTemp.buf, headSplit + HAL_STRLEN("\r\n"), chlNode->unformTemp.offset);
        chlNode->unformTemp.buf[chlNode->unformTemp.offset] = 0;
        if (NULL != httpSocket->recvFilename)
        {
            if (FALSE == httpSocket->eventCB(httpSocket->httpCode, httpSocket->retHeader, (quint8_t *)httpSocket->recvFilename, httpSocket->bodyLen))
            {
                Quos_socketChannelDel(chlFd);
                return TRUE;
            }
            else if ((httpSocket->fileFd = Qhal_fileOpen(httpSocket->recvFilename, FALSE)) == SOCKET_FD_INVALID)
            {
                Quos_socketChannelDel(chlFd);
                return TRUE;
            }
        }
    }

    if (HTTP_BODY_TYPE_NOMASK == httpSocket->bodyType)
    {
        if (NULL != httpSocket->recvFilename)
        {
            Qhal_fileWrite(httpSocket->fileFd, httpSocket->bodyOffset + httpSocket->bodyLen, chlNode->unformTemp.buf, chlNode->unformTemp.offset);
            httpSocket->bodyLen += chlNode->unformTemp.offset;
            chlNode->unformTemp.offset = 0;
        }
        else
        {
            httpSocket->bodyLen = chlNode->unformTemp.offset;
        }
    }
    else if (HTTP_BODY_TYPE_CHUNKED == httpSocket->bodyType)
    {
        if (NULL != httpSocket->recvFilename)
        {
            char *temp = (char *)chlNode->unformTemp.buf;
            while (1)
            {
                if (httpSocket->chunkedBlockLen == 0)
                {
                    if (temp[0] == '\r')
                        temp++;
                    if (temp[0] == '\n')
                        temp++;
                    if (HAL_STRSTR(temp, "\r\n") == NULL)
                    {
                        Quos_logPrintf(LSDK_HTTP, LL_DBG, "no found block len head");
                        break;
                    }
                    httpSocket->chunkedBlockLen = HAL_STRTOUL(temp, NULL, 16);
                    Quos_logPrintf(LSDK_HTTP, LL_DBG, "chunkedBlockLen:%u", httpSocket->chunkedBlockLen);
                    if (0 == httpSocket->chunkedBlockLen)
                    {
                        HttpSocket_t temp;
                        temp.eventCB = httpSocket->eventCB;
                        temp.httpCode = httpSocket->httpCode;
                        temp.retHeader = httpSocket->retHeader;
                        temp.recvFilename = httpSocket->recvFilename;
                        temp.bodyLen = httpSocket->bodyLen;
                        httpSocket->retHeader = NULL;
                        httpSocket->recvFilename = NULL;
                        if (SOCKET_FD_INVALID != httpSocket->fileFd)
                        {
                            Qhal_fileClose(httpSocket->fileFd);
                            httpSocket->fileFd = SOCKET_FD_INVALID;
                        }
                        Quos_socketChannelDel(chlFd);
                        temp.eventCB(temp.httpCode, temp.retHeader, (quint8_t *)temp.recvFilename, temp.bodyLen);
                        HAL_FREE(temp.retHeader);
                        HAL_FREE(temp.recvFilename);
                        return TRUE;
                    }
                    temp = HAL_STRSTR(temp, "\r\n") + HAL_STRLEN("\r\n");
                }

                quint32_t validLen = chlNode->unformTemp.offset - (temp - (char *)chlNode->unformTemp.buf) > httpSocket->chunkedBlockLen ? httpSocket->chunkedBlockLen : chlNode->unformTemp.offset - (temp - (char *)chlNode->unformTemp.buf);
                if (validLen == 0)
                {
                    break;
                }
                Qhal_fileWrite(httpSocket->fileFd, httpSocket->bodyOffset + httpSocket->bodyLen, temp, validLen);
                httpSocket->bodyLen += validLen;
                httpSocket->chunkedBlockLen -= validLen;
                temp += validLen;
            }
            if (temp != (char *)chlNode->unformTemp.buf)
            {
                chlNode->unformTemp.offset = chlNode->unformTemp.offset - (temp - (char *)chlNode->unformTemp.buf);
                HAL_MEMMOVE(chlNode->unformTemp.buf, temp, chlNode->unformTemp.offset);
                chlNode->unformTemp.buf[chlNode->unformTemp.offset] = 0;
            }
            return FALSE;
        }
        else
        {
            Quos_logPrintf(LSDK_HTTP, LL_DBG, "body:%u\n%s ", chlNode->unformTemp.offset, chlNode->unformTemp.buf);
            if (HAL_STRSTR(chlNode->unformTemp.buf, "\r\n0\r\n\r\n"))
            {
                quint32_t pieceLen;
                char *temp = (char *)chlNode->unformTemp.buf;

                while ((pieceLen = HAL_STRTOUL(temp, NULL, 16)) > 0)
                {
                    Quos_logPrintf(LSDK_HTTP, LL_DBG, "pieceLen:%u bodyLen=%u", pieceLen, httpSocket->bodyLen);
                    temp = HAL_STRSTR(temp, "\r\n") + HAL_STRLEN("\r\n");
                    HAL_MEMMOVE(chlNode->unformTemp.buf + httpSocket->bodyLen, temp, pieceLen);
                    httpSocket->bodyLen += pieceLen;
                    chlNode->unformTemp.buf[httpSocket->bodyLen] = 0;
                    temp += pieceLen + HAL_STRLEN("\r\n");
                }

                httpSocket->eventCB(httpSocket->httpCode, httpSocket->retHeader, chlNode->unformTemp.buf, httpSocket->bodyLen);
                Quos_socketChannelDel(chlFd);
                return TRUE;
            }
        }
    }
    else if (HTTP_BODY_TYPE_CONTENTLENGHT <= httpSocket->bodyType)
    {
        if (NULL != httpSocket->recvFilename)
        {
            Qhal_fileWrite(httpSocket->fileFd, httpSocket->bodyOffset + httpSocket->bodyLen, chlNode->unformTemp.buf, chlNode->unformTemp.offset);
            httpSocket->bodyLen += chlNode->unformTemp.offset;
            Quos_logPrintf(LSDK_HTTP, LL_DBG, "recv piece:%u len:%u", chlNode->unformTemp.offset, httpSocket->bodyLen);
            chlNode->unformTemp.offset = 0;
            if (httpSocket->bodyLen >= (quint32_t)httpSocket->bodyType)
            {
                if (SOCKET_FD_INVALID != httpSocket->fileFd)
                {
                    Qhal_fileClose(httpSocket->fileFd);
                    httpSocket->fileFd = SOCKET_FD_INVALID;
                }
                httpSocket->eventCB(httpSocket->httpCode, httpSocket->retHeader, (quint8_t *)httpSocket->recvFilename, httpSocket->bodyLen);
                Quos_socketChannelDel(chlFd);
                return TRUE;
            }
        }
        else
        {
            httpSocket->bodyLen = chlNode->unformTemp.offset;
            if (httpSocket->bodyLen >= (quint32_t)httpSocket->bodyType)
            {
                httpSocket->eventCB(httpSocket->httpCode, httpSocket->retHeader, chlNode->unformTemp.buf, httpSocket->bodyLen);
                Quos_socketChannelDel(chlFd);
                return TRUE;
            }
        }
    }
    return TRUE;
}
/**************************************************************************
** ����	@brief : HTTP������Դ�ͷ�
** ����	@param :
** ���	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_httpParamFree(void *param)
{
    HttpSocket_t *httpSock = (HttpSocket_t *)param;
    if (SOCKET_FD_INVALID != httpSock->fileFd)
    {
        Qhal_fileClose(httpSock->fileFd);
        httpSock->fileFd = SOCKET_FD_INVALID;
    }
    HAL_FREE(httpSock->sendFilename);
    HAL_FREE(httpSock->recvFilename);
    HAL_FREE(httpSock->retHeader);
    HAL_FREE(httpSock);
}
/**************************************************************************
** ����	@brief : http request
** ����	@param :
** ���	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_httpRequest(void **httpFd, const char *opt, const char *url, httpEventCB_f eventCB, const HttpReqData_t *reqData, const HttpReqFile_t *reqFile)
{
    urlAnalyze_t urlA;
    if (NULL == opt || NULL == url || NULL == eventCB || (NULL != reqData && 0 != reqData->payloadLen && NULL == reqData->payload))
    {
        Quos_logPrintf(LSDK_HTTP, LL_ERR, "opt[%s] url[%p] eventCB[%p] payload[%u:%p]", opt, url, eventCB, reqData->payloadLen, reqData->payload);
        return FALSE;
    }
    if (FALSE == (Quos_urlAnalyze(url, &urlA)))
    {
        Quos_logPrintf(LSDK_HTTP, LL_ERR, "url invliad");
        return FALSE;
    }
    urlA.port = urlA.port ? urlA.port : (urlA.isSecure ? 443 : 80);

    HttpSocket_t *httpSock = HAL_MALLOC(sizeof(HttpSocket_t));
    if (NULL == httpSock)
    {
        Quos_logPrintf(LSDK_HTTP, LL_ERR, "mcf httpSock");
        return FALSE;
    }
    HAL_MEMSET(httpSock, 0, sizeof(HttpSocket_t));
    httpSock->fileFd = SOCKET_FD_INVALID;
    httpSock->eventCB = eventCB;
    httpSock->bodyType = HTTP_BODY_TYPE_INVALID;
    if (reqFile)
    {
        httpSock->bodyOffset = reqFile->offset;
        httpSock->bodyLen = reqFile->size;
        if (reqFile->txName)
        {
            httpSock->sendFilename = HAL_STRDUP(reqFile->txName);
            if (NULL == httpSock->sendFilename)
            {
                Quos_logPrintf(LSDK_HTTP, LL_ERR, "mcf sendFilename");
                HAL_FREE(httpSock);
                return FALSE;
            }
        }
        if (reqFile->rxName)
        {
            httpSock->recvFilename = HAL_STRDUP(reqFile->rxName);
            if (NULL == httpSock->recvFilename)
            {
                Quos_logPrintf(LSDK_HTTP, LL_ERR, "mcf recvFilename");
                HAL_FREE(httpSock->sendFilename);
                HAL_FREE(httpSock);
                return FALSE;
            }
        }
    }

    quint16_t len = sizeof(HTTP_DEFAULT_HEADER) + HAL_STRLEN(opt) + HAL_STRLEN(urlA.path) + HAL_STRLEN(urlA.hostname) + 5;
    if (reqData)
    {
        len += HAL_STRLEN(reqData->rawHeaders) + reqData->payloadLen;
    }

    if (NULL == reqFile && NULL != reqData && 0 != reqData->payloadLen)
    {
        len += sizeof(QUOS_HTTP_HEAD_CONTENT_LENGHT) + 5;
    }
    len += HAL_STRLEN("\r\n");
    quint8_t *buf = HAL_MALLOC(len);
    if (NULL == buf)
    {
        HAL_FREE(httpSock->sendFilename);
        HAL_FREE(httpSock->recvFilename);
        HAL_FREE(httpSock);
        Quos_logPrintf(LSDK_HTTP, LL_ERR, "mcf tcpPayload,len:%u", len);
        return FALSE;
    }
    len = HAL_SPRINTF((char *)buf, HTTP_DEFAULT_HEADER, opt, (urlA.path ? urlA.path : ""), urlA.hostname, urlA.port, ((reqData && reqData->rawHeaders) ? reqData->rawHeaders : ""));
    if (NULL == reqFile && NULL != reqData && 0 != reqData->payloadLen)
    {
        len += HAL_SPRINTF((char *)buf + len, QUOS_HTTP_HEAD_CONTENT_LENGHT, reqData->payloadLen);
    }
    len += HAL_SPRINTF((char *)buf + len, "\r\n");
    Quos_logPrintf(LSDK_HTTP, LL_DBG, "%s", buf);
    if (reqData && 0 != reqData->payloadLen)
    {
        HAL_MEMCPY(buf + len, reqData->payload, reqData->payloadLen);
        len += reqData->payloadLen;
    }

    Quos_socketChlInfoNode_t chlInfo;
    HAL_MEMSET(&chlInfo, 0, sizeof(Quos_socketChlInfoNode_t));
    if (urlA.isSecure)
    {
#if (SDK_ENABLE_TLS == 1)
        chlInfo.sockFd = Qhal_tcpSslClientInit(&chlInfo.type, urlA.hostname, urlA.port, &chlInfo.conn.timeout);
#else
        chlInfo.sockFd = SOCKET_FD_INVALID;
#endif
    }
    else
    {
        chlInfo.sockFd = Qhal_tcpClientInit(&chlInfo.type, urlA.hostname, urlA.port, &chlInfo.conn.timeout);
    }
    if (chlInfo.conn.timeout)
    {
        chlInfo.conn.notify = quos_httpSockConnctCB;
    }

    if (SOCKET_FD_INVALID == chlInfo.sockFd)
    {
        HAL_FREE(httpSock->sendFilename);
        HAL_FREE(httpSock->recvFilename);
        HAL_FREE(httpSock->retHeader);
        HAL_FREE(httpSock);
        Quos_logPrintf(LSDK_HTTP, LL_ERR, "conn fail:%s[%u]", urlA.hostname, urlA.port);
        return FALSE;
    }

    chlInfo.io.send = Qhal_sockWrite;
    chlInfo.send.txCnt = 1;
    chlInfo.send.timeout = QUOS_HTTP_TIMEOUT;
    chlInfo.recvDataFunc = quos_httpRecvData;
    chlInfo.io.close = Qhal_sockClose;
    chlInfo.paramFree = quos_httpParamFree;
    chlInfo.param = httpSock;

    void *chlFd = Quos_socketChannelAdd(httpFd, chlInfo);
    if (NULL == chlFd)
    {
        Quos_logPrintf(LSDK_HTTP, LL_ERR, "add socket Channel fail");
        Qhal_sockClose(chlInfo.sockFd, chlInfo.type);
        HAL_FREE(httpSock->sendFilename);
        HAL_FREE(httpSock->recvFilename);
        HAL_FREE(httpSock->retHeader);
        HAL_FREE(httpSock);
        return FALSE;
    }

    if (FALSE == Quos_socketTx(chlFd, NULL, 0, 0, ((NULL == reqFile || FALSE == reqFile->isPostForm) ? NULL : quos_httpSendResult), quos_httpSendCB, 0, buf, len, NULL))
    {
        Quos_socketChannelDel(chlFd);
        return FALSE;
    }
    Quos_logPrintf(LSDK_HTTP, LL_DBG, "http socket create ok");
    return TRUE;
}
/**************************************************************************
** ����	@brief : http get�����ļ�
** ����	@param : 
** ���	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_httpGetDownload(void **httpFd, const char *url, httpEventCB_f eventCB, const HttpReqData_t *reqData, const char *filename, quint32_t offset)
{
    HttpReqFile_t reqFile;
    HAL_MEMSET(&reqFile, 0, sizeof(HttpReqFile_t));
    reqFile.rxName = (char *)filename;
    reqFile.offset = offset;
    return Quos_httpRequest(httpFd, "GET", url, eventCB, reqData, &reqFile);
}
/**************************************************************************
** ����	@brief : http post��
** ����	@param : reqData.rawHeaders:����Content-Type��Content-Length�������Զ����header
                 reqData.payload:���ı�ͷ����
                 reqData.payloadLen:���ı�ͷ���ݳ���
** ���	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_httpPostForm(void **httpFd, const char *url, httpEventCB_f eventCB, const HttpReqData_t *reqData, const char *filename, quint32_t fileSize)
{
    HttpReqFile_t reqFile;
    HAL_MEMSET(&reqFile, 0, sizeof(HttpReqFile_t));
    HttpReqData_t newReqData = *reqData;
    reqFile.txName = (char *)filename;
    reqFile.size = fileSize;
    reqFile.isPostForm = TRUE;

    quint32_t rawHeaderLen = HAL_STRLEN(reqData->rawHeaders);
    rawHeaderLen += sizeof(QUOS_HTTP_CONTENT_TYPE_KEY) + HAL_STRLEN(QUOS_HTTP_CONTENT_TYPE_VALUE_MULTIPART);
    rawHeaderLen += sizeof(QUOS_HTTP_HEAD_CONTENT_LENGHT) + 5;
    newReqData.rawHeaders = HAL_MALLOC(rawHeaderLen);
    if (NULL == newReqData.rawHeaders)
    {
        return FALSE;
    }
    HAL_SPRINTF(newReqData.rawHeaders, "%s" QUOS_HTTP_CONTENT_TYPE_KEY QUOS_HTTP_CONTENT_TYPE_VALUE_MULTIPART QUOS_HTTP_HEAD_CONTENT_LENGHT, reqData->rawHeaders, reqData->payloadLen + fileSize + (quint32_t)HAL_STRLEN(QUOS_HTTP_MULTIPART_NODE_END));
    qbool ret = Quos_httpRequest(httpFd, "POST", url, eventCB, &newReqData, &reqFile);
    HAL_FREE(newReqData.rawHeaders);
    return ret;
}
/**************************************************************************
** ����	@brief : ��ȡ��ǰ�ļ�����/�ϴ�����
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_httpFileRate(void *httpFd, quint32_t *bodyLen, qint32_t *bodyType, char **retHeader)
{
    qbool ret = FALSE;
    if(Quos_socketGetSockFdType(httpFd, NULL, NULL))
    {
        Quos_socketChlInfoNode_t *sockNode = (Quos_socketChlInfoNode_t*)httpFd;
        if(sockNode->param)
        {
            HttpSocket_t *httpSock = (HttpSocket_t *)sockNode->param;
            if(bodyLen)
            {
                *bodyLen = httpSock->bodyLen;
            }
            if(bodyType)
            {
                *bodyType = httpSock->bodyType;
            }
            if(retHeader)
            {
                *retHeader = httpSock->retHeader;
            }
            ret = TRUE;
        }
    }
    return ret;
}
#endif