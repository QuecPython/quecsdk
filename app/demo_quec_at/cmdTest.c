/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 2020-11-26
** 功能   @brief   : Linux调试命令
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "ql_iotAt.h"
#include "qhal_Socket.h"
#include "qhal_atCmd.h"
#define APP_AT LL_DBG


#define APP_AT_SOCKET_PORT 39999
#define APP_AT_RECV_MAX    5000 
void cmdTestSocketSend(pointer_t sockFd, const quint8_t *data, quint32_t len);

typedef struct
{
    char *cmd;
    void (*cb)(qhal_atcmd_t *cmd);
} atCmdTable_t;

static atCmdTable_t table[] =
    {
        {"QIOTREG", Qhal_atCmdIotAtQIOTREG},
        {"QIOTSEND", Qhal_atCmdIotAtQIOTTransTx},
        {"QIOTRD", Qhal_atCmdIotAtQIOTTransRx},
        {"QIOTCFG", Qhal_atCmdIotAtQIOTCFG},
        {"QIOTMODELTD", Qhal_atCmdIotAtQIOTModelTx},
        {"QIOTMODELRD", Qhal_atCmdIotAtQIOTModelRx},
        {"QIOTMCUVER", Qhal_atCmdIotAtQIOTMCUVER},
        {"QIOTUPDATE", Qhal_atCmdIotAtQIOTUPDATE},
        {"QIOTINFO", Qhal_atCmdIotAtQIOTINFO},
        {"QIOTOTARD", Qhal_atCmdIotAtQIOTOTARD},
        {"QIOTSTATE", Qhal_atCmdIotAtQIOTSTATE},
        {"QIOTLOCIN", Qhal_atCmdIotAtQIOTLOCIN},
        {"QIOTLOCEXT", Qhal_atCmdIotAtQIOTLOCEXT},
        {"QIOTOTAREQ", Qhal_atCmdIotAtQIOTOTARequest},
        #ifdef QUEC_ENABLE_HTTP_OTA
        {"QFOTAUP", Qhal_atCmdIotAtQFOTAUP},
        {"QFOTACFG", Qhal_atCmdIotAtQFOTACFG},
        #endif
        #ifdef QUEC_ENABLE_GATEWAY
		{"QIOTSUBCONN", Qhal_atCmdIotAtQIOTSUBCONN},
        {"QIOTSUBDISCONN", Qhal_atCmdIotAtQIOTSUBDISCONN},
        {"QIOTSUBRD", Qhal_atCmdIotAtQIOTSUBRD},
        {"QIOTSUBSEND", Qhal_atCmdIotAtQIOTSUBSEND},
        {"QIOTSUBTSLRD", Qhal_atCmdIotAtQIOTSUBTSLRD},
        {"QIOTSUBTSLTD", Qhal_atCmdIotAtQIOTSUBTSLTD},
//        {"QIOTSUBINFO", Qhal_atCmdIotAtQIOTSUBINFO},
        {"QIOTSUBHTB", Qhal_atCmdIotAtQIOTSUBHTB},
        #endif
        {NULL, NULL}};

#define AT_SOCKECT_MASK "AT"
/**************************************************************************
** 功能	@brief : AT透传模式
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static quint32_t cmdTestPassMode(pointer_t sockFd, int dataLen,char **buf)
{
    int ret;
    int bufLen = 0;
    qint32_t timeoutSum = 10;
    *buf = HAL_MALLOC(dataLen);
    if(NULL == *buf)
    {
        Quos_logPrintf(APP_AT, LL_ERR, "malloc fail");
        return 0;
    }
    do
    {
        if(SOCKET_FD_INVALID == sockFd)
        {
            ret = read(STDIN_FILENO, *buf+bufLen, dataLen-bufLen);
        }
        else
        {
            ret = read(sockFd, *buf+bufLen, dataLen-bufLen);
        }
        if(ret < 0)
        {
            Quos_logPrintf(APP_AT, LL_ERR, "read error");
            break;
        }
        bufLen += ret;
        if(bufLen >= dataLen)
        {
            break;
        }
        sleep(1);
        if(timeoutSum > 0)
        {
            timeoutSum--;
        }
        else
        {
            return 0;
        }
    } while (1);
    return bufLen;
}

/**************************************************************************
** 功能	@brief : AT命令参数提取
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void atCmd_argExtract(char *src, qhal_atcmd_t *cmd)
{
    char *args[QHAL_AT_PARAM_MAX];
    quint32_t argNum = 0; 
    char *p = src;
    do
    {
        qbool isStr = FALSE;
        args[argNum] = p;
        if(*p == '"')
        {
            while ((p = HAL_STRSTR(p+1,"\"")))
            {
                p++;
                if(*p == '\0' || *p == ',')
                {
                    isStr = TRUE;
                    break;
                }
            }
        }
        else
        {
            p = HAL_STRSTR(p,",");
        }
        if(p && *p == ',')
        {
            *p++ = '\0';
        }
        if(isStr)
        {
            cmd->params[argNum].type = QHAL_AT_TYPE_STRING;
            cmd->params[argNum].val = (quint8_t *)Quos_stringRemoveMarks(args[argNum]);
            cmd->params[argNum].len = HAL_STRLEN(args[argNum]);
        }
        else
        {
            if(Quos_strIsUInt(args[argNum], HAL_STRLEN(args[argNum]),NULL) == TRUE)
            {
                cmd->params[argNum].type = QHAL_AT_TYPE_INT;
                cmd->params[argNum].val = (quint8_t *)args[argNum];
            }
            else
            {
                cmd->params[argNum].type = QHAL_AT_TYPE_RAW;
                cmd->params[argNum].val = (quint8_t *)args[argNum];
                cmd->params[argNum].len = HAL_STRLEN(args[argNum]);
            }
        }
        argNum++;
    }while (argNum < QHAL_AT_PARAM_MAX && p && *p != '\0');
    cmd->param_count = argNum;
}

/**************************************************************************
** 功能	@brief : AT命令解析
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void atCmdAnalyze(pointer_t sockFd, char *buf,int len)
{
#define AT_HEAD "AT+"
    if (0 != HAL_STRNCASECMP(buf, AT_HEAD, HAL_STRLEN(AT_HEAD)))
    {
        return;
    }
    else if('\n' != buf[len - 1])
    {
        cmdTestSocketSend(sockFd, (const quint8_t *)"ERROR\r\n", HAL_STRLEN("ERROR\r\n"));
        return;
    }
    buf[len - 1] = 0;
    len--;
    if (buf[len - 1] == '\r')
    {
        buf[len - 1] = 0;
        len--;
    }
    buf += HAL_STRLEN(AT_HEAD);
    quint32_t i = 0;
    while (table[i].cmd)
    {
        if (NULL == table[i].cb || 0 != HAL_STRNCMP(buf, table[i].cmd, HAL_STRLEN(table[i].cmd)))
        {
            i++;
            continue;
        }
        buf += HAL_STRLEN(table[i].cmd);
        qhal_atcmd_t cmd;
        cmd.param_count = 0;
        cmd.sockFd = sockFd;
        cmd.action = QIOT_AT_ACTION_UNKOWN;
        if ('=' == buf[0])
        {
            if ('?' == buf[1] && '\0' == buf[2])
            {
                Quos_logPrintf(APP_AT, LL_DBG, "AT test");
                cmd.action = QIOT_AT_ACTION_TEST;
            }
            else if ('\0' != buf[1])
            {
                atCmd_argExtract(buf + 1, &cmd);
                cmd.action = QIOT_AT_ACTION_WRITE;
            }
            else
            {
                Quos_logPrintf(APP_AT, LL_ERR, "AT err");
            }
        }
        else if ('?' == buf[0] && '\0' == buf[1])
        {
            Quos_logPrintf(APP_AT, LL_DBG, "AT read");
            cmd.action = QIOT_AT_ACTION_READ;
        }
        else if ('\0' == buf[0])
        {
            cmd.action = QIOT_AT_ACTION_EXEC;
        }
        if (QIOT_AT_ACTION_UNKOWN != cmd.action)
        {
            char *passData = NULL;
            uint32_t length = 0;
            if (((HAL_STRCMP("QIOTSEND",table[i].cmd) == 0) && 2 == cmd.param_count) ||
                ((HAL_STRCMP("QIOTMODELTD",table[i].cmd) == 0) && (2 == cmd.param_count ||(3 == cmd.param_count && cmd.params[2].type == QHAL_AT_TYPE_INT))))
            {
                length = HAL_ATOI((const char *)cmd.params[1].val);
            }
#ifdef QUEC_ENABLE_GATEWAY
            else if ((HAL_STRCMP("QIOTSUBTSLTD",table[i].cmd) == 0 && ((4 == cmd.param_count && cmd.params[3].type == QHAL_AT_TYPE_INT) || 3 == cmd.param_count)) ||
                (HAL_STRCMP("QIOTSUBSEND",table[i].cmd) == 0 && 3 == cmd.param_count))
            {
                length = HAL_ATOI((const char *)cmd.params[2].val);
            }
#endif
            if (0 != length)
            {
                Quos_logPrintf(APP_AT, LL_ERR, "need wait data,len:%d",length);
                cmdTestSocketSend(sockFd, (const quint8_t *)"> ", HAL_STRLEN("> "));
                quint32_t passDataLen = cmdTestPassMode(sockFd,(int)length,&passData);
                if(passDataLen)
                {
                    cmd.params[cmd.param_count].type = QHAL_AT_TYPE_PASS;
                    cmd.params[cmd.param_count].val = (quint8_t*)passData;
                    cmd.params[cmd.param_count].len = passDataLen;
                    cmd.param_count++;
                }
                else
                {
                    cmdTestSocketSend(sockFd, (const quint8_t *)"ERROR\r\n", HAL_STRLEN("ERROR\r\n"));
                    if(passData)
                    {
                        HAL_FREE(passData);
                    }
                    i++;
                    continue;
                }
            }
            table[i].cb(&cmd);
            if(passData)
            {
                HAL_FREE(passData);
            }
        }
        else
        {
            cmdTestSocketSend(sockFd, (const quint8_t *)"ERROR\r\n", HAL_STRLEN("ERROR\r\n"));
        }
        i++;
    }
}

/**************************************************************************
** 功能	@brief : TCP客户端数据发送
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void cmdTestSocketSend(pointer_t sockFd, const quint8_t *data, quint32_t len)
{
    if(NULL == data || 0 == len)
    {
        return ;
    }
    Quos_logPrintf(APP_AT, LL_DBG, "data[%d]:\n%s", len, data);
    if (SOCKET_FD_INVALID == sockFd)
    {
        void *chlList[100];
        quint32_t count = Quos_socketGetChlFdList(SOCKET_TYPE_TCP_CLI, (void*)AT_SOCKECT_MASK, chlList,sizeof(chlList)/sizeof(chlList[0]));
        while (count--)
        {
            quint8_t *newData = HAL_MEMDUP(data, len);
            if(newData)
            {
                Quos_socketTxDisorder(chlList[count],NULL,(quint8_t*)newData, len);
            }
        }
    }
    else
    {
        Quos_socketChlInfoNode_t *node = (Quos_socketChlInfoNode_t *)Quos_socketGetChlFd(sockFd,SOCKET_TYPE_TCP_CLI);
        if(node)
        {
            quint8_t *newData = HAL_MEMDUP(data, len);
            if(newData)
            {
                Quos_socketTxDisorder(node,NULL,(quint8_t*)newData, len);
            }
        }
    }
}

/**************************************************************************
** 功能	@brief : TCP客户端数据接收
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM cmdTestSocketRecv(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData)
{
    UNUSED(peer);
    UNUSED(peerSize);
    Quos_socketChlInfoNode_t *chlNode = (Quos_socketChlInfoNode_t *)chlFd;
    if(NULL == recvData)
    {
        Quos_logPrintf(APP_AT, LL_DBG,"socket disconnect");
        return TRUE;
    }
    Quos_logPrintf(APP_AT, LL_DBG,"recv data:%.*s",recvData->bufLen,recvData->Buf);
    atCmdAnalyze(chlNode->sockFd, (char *)recvData->Buf,recvData->bufLen);
    return TRUE;
}

/**************************************************************************
** 功能	@brief : 局域网监听新的TCP客户端连接
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool FUNCTION_ATTR_ROM cmdTestClientListen(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData)
{
    UNUSED(chlFd);
    UNUSED(peer);
    UNUSED(peerSize);
    if(NULL == recvData)
    {
        return FALSE;
    }
    Quos_socketChlInfoNode_t chlInfo;
    HAL_MEMCPY(&chlInfo,recvData->Buf,recvData->bufLen);
    Quos_logPrintf(APP_AT, LL_DBG,"new cliend sockFd:"PRINTF_FD,chlInfo.sockFd);
    chlInfo.io.send = Qhal_sockWrite;
    chlInfo.send.txCnt = 1;
    chlInfo.send.timeout = 2000;
    chlInfo.recvDataFunc = cmdTestSocketRecv;
    chlInfo.io.close = Qhal_sockClose;
    chlInfo.param = AT_SOCKECT_MASK;
    Quos_socketChannelAdd(NULL,chlInfo);
    return TRUE;
}

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void CmdTestInit(void)
{
    Quos_socketChlInfoNode_t chlInfo;
    HAL_MEMSET(&chlInfo, 0, sizeof(Quos_socketChlInfoNode_t));
    chlInfo.sockFd = Qhal_tcpServerInit(&chlInfo.type, APP_AT_SOCKET_PORT, 100);
    Quos_logPrintf(APP_AT, LL_DBG, "fd:" PRINTF_FD, chlInfo.sockFd);
    if (SOCKET_FD_INVALID == chlInfo.sockFd)
    {
        Quos_logPrintf(APP_AT, LL_ERR, "listening port failed");
        return;
    }
    chlInfo.send.txCnt = 1;
    chlInfo.send.timeout = 2000;
    chlInfo.recvDataFunc = cmdTestClientListen;
    Quos_socketChannelAdd(NULL, chlInfo);
}
