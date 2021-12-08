/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : coap通信管理
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_coap.h"
#if (SDK_ENABLE_COAP == 1)
#include "Quos_kernel.h"
#include "Qhal_driver.h"

#ifndef QUOS_COAP_SEND_TIMEOUT
#define QUOS_COAP_SEND_TIMEOUT 5 * SWT_ONE_SECOND
#endif
#ifndef QUOS_COAP_RESEND_TIME
#define QUOS_COAP_RESEND_TIME 3
#endif
#define QUOS_COAP_VERSION 0x01

typedef struct
{
    quint16_t mid;
    coapRecvNotify_f notifyCB;
    void *peer;
} coapSock_t;
typedef struct
{
    TWLLHead_T head;
    coapOptionType_t type;
    quint16_t len;
    quint8_t val[1];
} coap_optionNode_t;
/**************************************************************************
** 功能	@brief : coap message 头设置
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_coapHeadSet(Coap_Message_t *coapMsg, coapHeadType_t type, coapHeadCode_t code, quint16_t mid, quint32_t tokenLen, const quint8_t *token)
{
    coapMsg->head.ver = QUOS_COAP_VERSION;
    coapMsg->head.type = type;
    coapMsg->head.tokenLen = tokenLen > sizeof(coapMsg->token) ? sizeof(coapMsg->token) : tokenLen;
    coapMsg->head.code = code;
    coapMsg->head.mid = mid;
    if (coapMsg->head.tokenLen)
    {
        if (NULL == token)
        {
            Systick_T tm = Quos_sysTickGet();
            _U16_ARRAY01(mid, coapMsg->token);
            _U32_ARRAY0123(tm.sec, coapMsg->token + 2);
            _U16_ARRAY01(tm.ms, coapMsg->token + 6);
        }
        else
        {
            HAL_MEMCPY(coapMsg->token, token, coapMsg->head.tokenLen);
        }
    }
}

/**************************************************************************
** 功能	@brief : 找出options数组的元素id比参考id大但差值最小的节点
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static coap_optionNode_t FUNCTION_ATTR_ROM *quos_coapOptionIdDeltaMinGet(const TWLLHead_T *optionsHead, const coap_optionNode_t *referNode)
{
    qbool findNext = FALSE;
    coap_optionNode_t *option = NULL;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)optionsHead, temp, next)
    {
        coap_optionNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, coap_optionNode_t, head);
        if (NULL == referNode)
        {
            if (NULL == option)
            {
                option = node;
            }
            else if (option->type > node->type)
            {
                option = node;
            }
        }
        else if (node->type == referNode->type)
        {
            if (node == referNode)
            {
                findNext = TRUE;
            }
            else if (findNext)
            {
                return node;
            }
        }
        else if (node->type > referNode->type && (NULL == option || node->type < option->type))
        {
            option = node;
        }
    }
    return option;
}
/**************************************************************************
** 功能	@brief : coap message option 不透明类型数据添加
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_coapOptionSetOpaque(Coap_Message_t *coapMsg, coapOptionType_t type, const void *val, quint16_t valLen)
{
    coap_optionNode_t *node = HAL_MALLOC(__GET_POS_ELEMENT(coap_optionNode_t, val) + valLen);
    if (NULL == node)
    {
        return FALSE;
    }
    HAL_MEMSET(node, 0, sizeof(coap_optionNode_t));
    node->type = type;
    HAL_MEMCPY(node->val, (quint8_t *)val, valLen);
    node->len = valLen;
    Quos_twllHeadAdd((TWLLHead_T **)&coapMsg->optionsHead, &node->head);
    return TRUE;
}
/**************************************************************************
** 功能	@brief : coap message option 不透明类型数据获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_coapOptionGetOpaque(Coap_Message_t *coapMsg, coapOptionType_t type, const void **val, quint16_t *valLen)
{
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)coapMsg->optionsHead, temp, next)
    {
        coap_optionNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, coap_optionNode_t, head);
        if (type == node->type)
        {
            if (val)
            {
                *val = node->val;
            }
            if (valLen)
            {
                *valLen = node->len;
            }
            return TRUE;
        }
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : coap message option 数值类型数据添加
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_coapOptionSetNumber(Coap_Message_t *coapMsg, coapOptionType_t type, quint32_t number)
{
    quint16_t valLen = 0;
    quint32_t temp = number;
    do
    {
        valLen++;
        temp >>= 8;
    } while (temp > 0);

    coap_optionNode_t *node = HAL_MALLOC(__GET_POS_ELEMENT(coap_optionNode_t, val) + valLen);
    if (NULL == node)
    {
        return FALSE;
    }
    HAL_MEMSET(node, 0, sizeof(coap_optionNode_t));
    node->type = type;
    node->len = valLen;
    while (valLen--)
    {
        node->val[valLen] = number & 0xFF;
        number >>= 8;
    }
    Quos_twllHeadAdd((TWLLHead_T **)&coapMsg->optionsHead, &node->head);
    return TRUE;
}
/**************************************************************************
** 功能	@brief : coap message option 数值类型数据获取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_coapOptionGetNumber(Coap_Message_t *coapMsg, coapOptionType_t type, quint32_t *number)
{
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)coapMsg->optionsHead, temp, next)
    {
        coap_optionNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, coap_optionNode_t, head);
        if (type == node->type)
        {
            if (number)
            {
                quint16_t i;
                *number = 0;
                for (i = 0; i < node->len; i++)
                {
                    *number |= (quint32_t)node->val[i] << (i * 8);
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 设置payload
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_coapPayloadSet(Coap_Message_t *coapMsg, const void *val, quint16_t valLen)
{
    coapMsg->payload.val = (void *)val;
    coapMsg->payload.len = valLen;
}
/**************************************************************************
** 功能	@brief : PATH 转成option结构体
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_coapOptionSetPath(Coap_Message_t *coapMsg, const char *path)
{
    char *tmpPath = (char *)path;
    if (NULL == path)
    {
        return TRUE;
    }
    while ('\0' != tmpPath[0] && '?' != tmpPath[0])
    {
        if (tmpPath[0] == '/')
            tmpPath++;
        else
        {
            quint32_t i = 0;
            while ('\0' != tmpPath[i] && tmpPath[i] != '/' && tmpPath[i] != '?')
                i++;
            if (FALSE == Quos_coapOptionSetOpaque(coapMsg, COAP_OTYPE_URI_PATH, tmpPath, i))
            {
                return FALSE;
            }
            tmpPath += i;
        }
    }
    if ('?' == tmpPath[0])
        tmpPath++;
    while ('\0' != tmpPath[0])
    {
        if (tmpPath[0] == '&')
            tmpPath++;
        else
        {
            quint32_t i = 0;
            while (tmpPath[i] != 0 && tmpPath[i] != '&')
                i++;
            if (FALSE == Quos_coapOptionSetOpaque(coapMsg, COAP_OTYPE_URI_QUERY, tmpPath, i))
            {
                return FALSE;
            }
            tmpPath += i;
        }
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 转成option结构体转成path
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
char FUNCTION_ATTR_ROM *Quos_coapOptionGetPath(const Coap_Message_t *coapMsg)
{
    quint16_t len = 0;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)coapMsg->optionsHead, temp, next)
    {
        coap_optionNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, coap_optionNode_t, head);
        if (COAP_OTYPE_URI_PATH == node->type || COAP_OTYPE_URI_QUERY == node->type)
        {
            len += node->len + 1;
        }
    }
    char *path = NULL;
    if (0 == len || (path = HAL_MALLOC(len + 1)) == NULL)
    {
        return NULL;
    }
    len = 0;
    TWLIST_FOR_DATA((TWLLHead_T *)coapMsg->optionsHead, temp, next)
    {
        coap_optionNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, coap_optionNode_t, head);
        if (COAP_OTYPE_URI_PATH == node->type)
        {
            HAL_SPRINTF(path + len, "/%.*s", node->len, node->val);
            len += node->len + 1;
        }
    }
    qbool isFirst = TRUE;
    TWLIST_FOR_DATA((TWLLHead_T *)coapMsg->optionsHead, temp, next)
    {
        coap_optionNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, coap_optionNode_t, head);
        if (COAP_OTYPE_URI_QUERY == node->type)
        {
            if (isFirst)
            {
                HAL_SPRINTF(path + len, "?%.*s", node->len, node->val);
                isFirst = FALSE;
            }
            else
            {
                HAL_SPRINTF(path + len, "&%.*s", node->len, node->val);
            }
            len += node->len + 1;
        }
    }
    return path;
}
/**************************************************************************
** 功能	@brief : coap option原型转十六进制流
** 输入	@param : buffer:非空时将option格式化成16进制到buffer,buffer必须足够大防止溢出
** 输出	@retval: 返回option格式化成16进制字节长度
***************************************************************************/
static quint16_t FUNCTION_ATTR_ROM quos_coapoptionFormat(const coap_optionNode_t *opt, coap_optionNode_t *referNode, quint8_t *buffer)
{
    quint16_t len = 0;
    quint16_t delta = opt->type - (referNode ? referNode->type : 0);
    if (delta > 268)
    {
        if (buffer)
        {
            buffer[0] = 14 << 4;
            buffer[1] = (delta - 269) >> 8;
            buffer[2] = (delta - 269) & 0xFF;
        }
        len = 3;
    }
    else if (delta > 12)
    {
        if (buffer)
        {
            buffer[0] = 13 << 4;
            buffer[1] = delta - 13;
        }
        len = 2;
    }
    else
    {
        if (buffer)
        {
            buffer[0] = delta << 4;
        }
        len = 1;
    }
    if (opt->len > 268)
    {
        if (buffer)
        {
            buffer[0] |= 14;
            buffer[len] = (opt->len - 269) >> 8;
            buffer[len + 1] = (opt->len - 269) & 0xFF;
        }
        len += 2;
    }
    else if (opt->len > 12)
    {
        if (buffer)
        {
            buffer[0] |= 13;
            buffer[len] = opt->len - 13;
        }
        len += 1;
    }
    else
    {
        if (buffer)
        {
            buffer[0] |= opt->len;
        }
    }
    if (buffer)
    {
        HAL_MEMCPY(buffer + len, opt->val, opt->len);
    }
    len += opt->len;
    Quos_logPrintf(LSDK_COAP, LL_DBG, "coap option format len:%u", len);
    if (buffer)
    {
        Quos_logHexDump(LSDK_COAP, LL_DUMP, "option head buf", buffer, len - opt->len);
    }
    return len;
}
/**************************************************************************
** 功能	@brief : 将十六进制流解析成option结构体
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static quint16_t FUNCTION_ATTR_ROM quos_coapoptionUnformat(const quint8_t *buffer, quint16_t bufLen, coap_optionNode_t **option, coap_optionNode_t *referNode)
{
    if (NULL == buffer || 0 == bufLen)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "param fail");
        return 0;
    }
    quint16_t i = 0;
    quint16_t delta = (buffer[i] >> 4) & 0x0F;
    quint16_t optLen = buffer[i] & 0x0F;
    if (15 == delta || 15 == optLen)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "delta[%u] or optLen[%u] is invalid", delta, optLen);
        return 0;
    }
    i++;

    if (i + (delta > 12 ? (delta - 12) : 0) + (optLen > 12 ? (optLen - 12) : 0) > bufLen)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "buflen isn't enough for delta and len");
        return 0;
    }
    if (13 == delta)
    {
        delta = 13 + buffer[i++];
    }
    else if (14 == delta)
    {
        delta = 269 + buffer[i] * 256 + buffer[i + 1];
        i += 2;
    }
    if (13 == optLen)
    {
        optLen = 13 + buffer[i++];
    }
    else if (14 == optLen)
    {
        optLen = 269 + buffer[i] * 256 + buffer[i + 1];
        i += 2;
    }
    if (i + optLen > bufLen)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "buflen isn't enough for option value");
        return 0;
    }
    if (option)
    {
        *option = HAL_MALLOC(__GET_POS_ELEMENT(coap_optionNode_t, val) + optLen);
        if (*option)
        {
            (*option)->type = delta + (referNode ? referNode->type : 0);
            (*option)->len = optLen;
            HAL_MEMCPY((*option)->val, &buffer[i], optLen);
        }
    }
    return i + optLen;
}
/**************************************************************************
** 功能	@brief : 释放Coap_Message_t内存
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_coapMessageFree(Coap_Message_t *coapMsg)
{
    while (coapMsg->optionsHead)
    {
        coap_optionNode_t *node = __GET_STRUCT_BY_ELEMENT(coapMsg->optionsHead, coap_optionNode_t, head);
        Quos_twllHeadDelete((TWLLHead_T **)&coapMsg->optionsHead, coapMsg->optionsHead);
        HAL_FREE(node);
    }
    HAL_FREE(coapMsg->payload.val);
    HAL_MEMSET(coapMsg, 0, sizeof(Coap_Message_t));
}
/**************************************************************************
** 功能	@brief : 计算coap数据包长度
** 输入	@param : coapMsg:coap消息结构体
** 输出	@retval: 
***************************************************************************/
static quint16_t FUNCTION_ATTR_ROM quos_coapMessageFormat_len(const Coap_Message_t *coapMsg)
{
    quint16_t pkgLen = sizeof(coapMsg->head) + coapMsg->head.tokenLen;
    coap_optionNode_t *referNode = NULL;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)coapMsg->optionsHead, temp, next)
    {
        coap_optionNode_t *node = quos_coapOptionIdDeltaMinGet(coapMsg->optionsHead, referNode);
        if (NULL == node)
        {
            break;
        }
        pkgLen += quos_coapoptionFormat(node, referNode, NULL);
        referNode = node;
    }
    if (coapMsg->payload.len)
    {
        pkgLen += 1 + coapMsg->payload.len;
    }
    Quos_logPrintf(LSDK_COAP, LL_DBG, "coap msg format len:%u", pkgLen);
    return pkgLen;
}

/**************************************************************************
** 功能	@brief : 将coap消息结构体格式化成十六进制字节流
** 输入	@param : coapMsg:coap消息结构体,buffer:内部分配空间缓存格式化后的字节流
** 输出	@retval: 格式化后字节流长度
***************************************************************************/
static quint16_t FUNCTION_ATTR_ROM quos_coapMessageFormat(const Coap_Message_t *coapMsg, quint8_t **buffer)
{
    quint16_t len = quos_coapMessageFormat_len(coapMsg);
    quint8_t *buf = HAL_MALLOC(len);
    if (NULL == buf)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "format malloc fail");
        return 0;
    }
    *buffer = buf;
    len = 0;

    buf[len++] = (coapMsg->head.ver << 6) | (coapMsg->head.type << 4) | coapMsg->head.tokenLen;
    buf[len++] = coapMsg->head.code;
    buf[len++] = coapMsg->head.mid >> 8;
    buf[len++] = coapMsg->head.mid & 0xFF;

    HAL_MEMCPY(buf + len, coapMsg->token, coapMsg->head.tokenLen);
    len += coapMsg->head.tokenLen;

    coap_optionNode_t *refNode = NULL;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)coapMsg->optionsHead, temp, next)
    {
        coap_optionNode_t *node = quos_coapOptionIdDeltaMinGet(coapMsg->optionsHead, refNode);
        if (NULL == node)
        {
            break;
        }
        len += quos_coapoptionFormat(node, refNode, buf + len);
        refNode = node;
    }

    if (coapMsg->payload.len)
    {
        buf[len++] = 0xFF;
        HAL_MEMCPY(buf + len, coapMsg->payload.val, coapMsg->payload.len);
        len += coapMsg->payload.len;
    }
    return len;
}

/**************************************************************************
** 功能	@brief : 检查coap十六进制流是否时有效coap数据
** 输入	@param : buffer:coap十六进制流
** 输出	@retval: 返回option有效个数，<0时代表buffer非法coap数据
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_coapMessageCheck(const quint8_t *buffer, quint16_t len, quint16_t *optSize)
{
    quint16_t i = 4;
    if (optSize)
        *optSize = 0;
    if (NULL == buffer || 0 == len || len < (i + (buffer[0] & 0xF)))
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "param is invalid buffer:%p len:%u tokenLen:%u", buffer, len, buffer[0] & 0xF);
        return FALSE;
    }
    i += buffer[0] & 0xF;
    while (1)
    {
        if (i == len)
        {
            if (optSize)
            {
                Quos_logPrintf(LSDK_COAP, LL_DBG, "optsize:%u", *optSize);
            }
            return TRUE;
        }
        if (0xFF == buffer[i])
        {
            return i + 1 < len ? TRUE : FALSE;
        }
        quint16_t optlen = quos_coapoptionUnformat(buffer + i, len - i, NULL, NULL);
        if (0 == optlen)
        {
            return FALSE;
        }
        i += optlen;
        if (optSize)
            (*optSize) += 1;
    }
}
/**************************************************************************
** 功能	@brief : 将十六进制流转成coap结构体
** 输入	@param : buffer:源数据十六进制流，coap:转换后的coap结构体
** 输出	@retval: TRUE:成功 FLASE:无效coap数据
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_coapMessageUnformat(const quint8_t *buffer, quint16_t bufLen, Coap_Message_t *coapMsg)
{
    quint16_t optSize = 0;
    Quos_logHexDump(LSDK_COAP, LL_DUMP, "source", (void *)buffer, bufLen);
    if (NULL == coapMsg || FALSE == quos_coapMessageCheck(buffer, bufLen, &optSize))
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "param is invalid");
        return FALSE;
    }
    quint16_t offset = 1;
    HAL_MEMSET(coapMsg, 0, sizeof(Coap_Message_t));
    coapMsg->head.ver = (buffer[0] >> 6) & 0x03;
    coapMsg->head.type = (buffer[0] >> 4) & 0x03;
    coapMsg->head.tokenLen = (buffer[0] >> 0) & 0x0F;
    coapMsg->head.code = buffer[offset++];
    coapMsg->head.mid = ((quint16_t)buffer[offset] << 8) + (quint16_t)buffer[offset + 1];
    offset += 2;
    if (bufLen < offset + coapMsg->head.tokenLen)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "buflen[%u] isn't enough to token[%u+%u]", bufLen, offset, coapMsg->head.tokenLen);
        return FALSE;
    }
    HAL_MEMCPY(coapMsg->token, &buffer[offset], coapMsg->head.tokenLen);
    offset += coapMsg->head.tokenLen;

    Quos_logPrintf(LSDK_COAP, LL_DBG, "ver[%u] type[%s] code[%s] mid[%u]", coapMsg->head.ver, COAP_HEAD_TYPE_STRING(coapMsg->head.type), COAP_HEAD_CODE_STRING(coapMsg->head.code), coapMsg->head.mid);
    Quos_logHexDump(LSDK_COAP, LL_DUMP, "token", coapMsg->token, coapMsg->head.tokenLen);

    coap_optionNode_t *referNode = NULL;
    while (1)
    {
        quint16_t optLen;
        coap_optionNode_t *option;
        if (offset == bufLen)
        {
            return TRUE;
        }
        else if (0xFF == buffer[offset])
        {
            offset++;
            if (bufLen > offset)
            {
                coapMsg->payload.len = bufLen - offset;
                coapMsg->payload.val = HAL_MALLOC(coapMsg->payload.len);
                if (NULL == coapMsg->payload.val)
                {
                    break;
                }
                HAL_MEMCPY(coapMsg->payload.val, buffer + offset, coapMsg->payload.len);
                return TRUE;
            }
            else
            {
                break;
            }
        }
        else if ((optLen = quos_coapoptionUnformat(buffer + offset, bufLen - offset, &option, referNode)) == 0 && NULL == option)
        {
            break;
        }
        referNode = option;
        offset += optLen;
        Quos_twllHeadAdd((TWLLHead_T **)&coapMsg->optionsHead, &option->head);
        Quos_logPrintf(LSDK_COAP, LL_DBG, "option type[%s]", COAP_OPTION_TYPE_STRING(option->type));
        Quos_logHexDump(LSDK_COAP, LL_DUMP, "option val", option->val, option->len);
    }
    Quos_logPrintf(LSDK_COAP, LL_ERR, "unformat fail");
    Quos_coapMessageFree(coapMsg);
    return FALSE;
}
/**************************************************************************
** 功能	@brief : coap接收数据处理
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static qbool FUNCTION_ATTR_ROM quos_coapSocketRecv(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData)
{
    UNUSED(peer);
    UNUSED(peerSize);
    
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    coapSock_t *coapSock = (coapSock_t *)chlNode->param;
    if (NULL == recvData)
    {
        coapSock->notifyCB(chlFd, NULL, NULL);
        return FALSE;
    }
    Quos_logHexDump(LSDK_COAP, LL_DUMP, "coap recv", recvData->Buf, recvData->bufLen);
    Coap_Message_t coapMsg;
    HAL_MEMSET(&coapMsg, 0, sizeof(Coap_Message_t));
    if (FALSE == quos_coapMessageUnformat(recvData->Buf, recvData->bufLen, &coapMsg))
    {
        return FALSE;
    }
    if (COAP_HTYPE_ACK == coapMsg.head.type)
    {
        Quos_socketTxAck(chlFd, coapSock->peer, coapMsg.head.mid, &coapMsg);
    }
    else
    {
        Coap_Message_t retCoapMsg;
        HAL_MEMSET(&retCoapMsg, 0, sizeof(Coap_Message_t));
        Quos_coapHeadSet(&retCoapMsg, COAP_HTYPE_ACK, COAP_HCODE_PRECONDITION_FAILED_412, coapMsg.head.mid, coapMsg.head.tokenLen, coapMsg.token);
        if (TRUE == coapSock->notifyCB(chlFd, &coapMsg, &retCoapMsg))
        {
            Quos_coapMsgSend(chlFd, NULL, &retCoapMsg, NULL, TRUE);
        }
    }

    Quos_coapMessageFree(&coapMsg);
    return TRUE;
}
/**************************************************************************
** 功能	@brief : coap 资源释放
** 输入	@param :
** 输出	@retval:
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_coapSocketParamFree(void *param)
{
    coapSock_t *coapSock = (coapSock_t *)param;
    HAL_FREE(coapSock->peer);
    HAL_FREE(coapSock);
}
/**************************************************************************
** 功能	@brief : coap初始化
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_coapInit(void **chlFdPoint, const char *url, coapRecvNotify_f notifyCb)
{
    urlAnalyze_t urlA;
    if (NULL == url)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "param invalid:url[%p]", url);
        return FALSE;
    }
    if (FALSE == (Quos_urlAnalyze(url, &urlA)))
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "url analyze fail");
        return FALSE;
    }
    urlA.port = urlA.port ? urlA.port : (urlA.isSecure ? 5684 : 5683);

    coapSock_t *coapSock = HAL_MALLOC(sizeof(coapSock_t));
    if (NULL == coapSock)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "malloc coapSock fail");
        return FALSE;
    }
    HAL_MEMSET(coapSock, 0, sizeof(coapSock_t));
    coapSock->notifyCB = notifyCb;
    coapSock->mid = 1;
    Quos_socketChlInfoNode_t chlInfo;
    HAL_MEMSET(&chlInfo, 0, sizeof(Quos_socketChlInfoNode_t));
    if (urlA.isSecure)
    {
#if (SDK_ENABLE_TLS == 1)
        chlInfo.sockFd = Qhal_udpSslInit(&chlInfo.type, 0, urlA.hostname, urlA.port);
#else
        chlInfo.sockFd = SOCKET_FD_INVALID;
#endif
    }
    else
    {
        chlInfo.sockFd = Qhal_udpInit(&chlInfo.type, 0, urlA.hostname, urlA.port, &coapSock->peer);
    }
    if (SOCKET_FD_INVALID == chlInfo.sockFd)
    {
        Quos_logPrintf(LSDK_COAP, LL_ERR, "coap conneted fail:%s[%u]", urlA.hostname, urlA.port);
        return FALSE;
    }

    chlInfo.io.send = Qhal_sockWrite;
    chlInfo.send.txCnt = QUOS_COAP_RESEND_TIME;
    chlInfo.send.timeout = QUOS_COAP_SEND_TIMEOUT;
    chlInfo.recvDataFunc = quos_coapSocketRecv;
    chlInfo.io.close = Qhal_sockClose;
    chlInfo.paramFree = quos_coapSocketParamFree;
    chlInfo.param = coapSock;

    void *chlFd = Quos_socketChannelAdd(chlFdPoint, chlInfo);
    if (NULL == chlFd)
    {
        Qhal_sockClose(chlInfo.sockFd, chlInfo.type);
        HAL_FREE(coapSock->peer);
        HAL_FREE(coapSock);
        Quos_logPrintf(LSDK_COAP, LL_ERR, "add socket Channel fail");
        return FALSE;
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : coap message消息发送，回调recvCB()->recvData返回Coap_Message_t
                消息指针
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_coapMsgSend(void *chlFd, const char *path, Coap_Message_t *coapMsg, socketRecvNodeCb_f recvCB, qbool isAck)
{
    if (NULL == chlFd)
    {
        Quos_coapMessageFree(coapMsg);
        Quos_logPrintf(LSDK_COAP, LL_ERR, "param invalid;chlFd[%p]", chlFd);
        return FALSE;
    }

    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    coapSock_t *coapSock = (coapSock_t *)chlNode->param;
    if (COAP_HTYPE_CON == coapMsg->head.type || COAP_HTYPE_NON == coapMsg->head.type)
    {
        coapMsg->head.mid = ++coapSock->mid;
    }
    if (FALSE == Quos_coapOptionSetPath(coapMsg, path))
    {
        Quos_coapMessageFree(coapMsg);
        Quos_logPrintf(LSDK_COAP, LL_ERR, "path to option fail");
        return FALSE;
    }
    quint8_t *output = NULL;
    quint16_t outLen = quos_coapMessageFormat(coapMsg, &output);
    Quos_coapMessageFree(coapMsg);
    if (0 == outLen)
    {
        return FALSE;
    }
    else if (isAck)
    {
        return Quos_socketTxDisorder(chlFd, coapSock->peer, output, outLen);
    }
    else
    {
        return Quos_socketTx(chlFd, coapSock->peer, 0, 0, NULL, recvCB, coapSock->mid, output, outLen, NULL);
    }
}
#endif