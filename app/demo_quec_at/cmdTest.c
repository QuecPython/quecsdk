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
#define APP_AT LL_DBG

typedef struct __appSock
{
    TWLLHead_T head;
    pointer_t sockFd;
} appSock_T;

static TWLLHead_T *appSockHead = NULL;
#define APP_AT_SOCKET_PORT 4321
void cmdTestSocketSend(pointer_t sockFd, const quint8_t *data, quint32_t len);

typedef struct
{
    char *cmd;
    qint32_t (*cb)(QIot_atAction_e action, char *retBuf, quint32_t retMaxLen, quint8_t count, char *arg[]);
} atCmdTable_t;

static atCmdTable_t table[] =
    {
        {"QIOTREG", Ql_iotAtQIOTREG},
        {"QIOTSEND", Ql_iotAtQIOTTransTx},
        {"QIOTRD", Ql_iotAtQIOTTransRx},
        {"QIOTCFG", Ql_iotAtQIOTCFG},
        {"QIOTMODELTD", Ql_iotAtQIOTModelTx},
        {"QIOTMODELRD", Ql_iotAtQIOTModelRx},
        {"QIOTMCUVER", Ql_iotAtQIOTMCUVER},
        {"QIOTUPDATE", Ql_iotAtQIOTUPDATE},
        {"QIOTINFO", Ql_iotAtQIOTINFO},
        {"QIOTOTARD", Ql_iotAtQIOTOTARD},
        {"QIOTSTATE", Ql_iotAtQIOTSTATE},
        {"QIOTLOCCFG", Ql_iotAtQIOTLOCCFG},
        {"QIOTLOCRPT", Ql_iotAtQIOTLOCRPT},
        {NULL, NULL}};
/**************************************************************************
** 功能	@brief : AT透传模式
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static char *cmdTestPassMode(pointer_t sockFd, int dataLen)
{
    int ret;
    int bufLen = 0;
    char *buf = malloc(dataLen);
    if(NULL == buf)
    {
        Quos_logPrintf(APP_AT, LL_ERR, "malloc fail");
        return NULL;
    }
    do
    {
        if(SOCKET_FD_INVALID == sockFd)
        {
            ret = read(STDIN_FILENO, buf+bufLen, dataLen-bufLen);
        }
        else
        {
            ret = read(sockFd, buf+bufLen, dataLen-bufLen);
        }
        if(ret < 0)
        {
            Quos_logPrintf(APP_AT, LL_ERR, "read error");
            break;
        }
        bufLen += ret;
        if(bufLen == dataLen)
        {
            return buf;
        }
    } while (1);
    return NULL;
}
/**************************************************************************
** 功能	@brief : AT命令解析
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void atCmdAnalyze(pointer_t sockFd, char *buf)
{
#define AT_HEAD "AT+"
    if (0 != strncasecmp(buf, AT_HEAD, HAL_STRLEN(AT_HEAD)) || '\n' != buf[HAL_STRLEN(buf) - 1])
    {
        return;
    }
    buf[HAL_STRLEN(buf) - 1] = 0;
    if (buf[HAL_STRLEN(buf) - 1] == '\r')
    {
        buf[HAL_STRLEN(buf) - 1] = 0;
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
        char retBuf[QIOT_AT_BUFFER_MAX] = {0};
        char *words[200];
        quint32_t size = 0;
        QIot_atAction_e action = QIOT_AT_ACTION_UNKOWN;
        if ('=' == buf[0])
        {
            if ('?' == buf[1] && '\0' == buf[2])
            {
                Quos_logPrintf(APP_AT, LL_DBG, "AT test");
                action = QIOT_AT_ACTION_TEST;
            }
            else if ('\0' != buf[1])
            {
                quint32_t i;
                size = Quos_stringSplit(buf + 1, words, 200, ",", TRUE);
                for (i = 0; i < size; i++)
                {
                    while (' ' == words[i][0])
                    {
                        words[i] = &words[i][1];
                    }
                    Quos_stringRemoveMarks(words[i]);
                }
                action = QIOT_AT_ACTION_WRITE;
            }
            else
            {
                Quos_logPrintf(APP_AT, LL_ERR, "AT err");
            }
        }
        else if ('?' == buf[0] && '\0' == buf[1])
        {
            Quos_logPrintf(APP_AT, LL_DBG, "AT read");
            action = QIOT_AT_ACTION_READ;
        }
        else if ('\0' == buf[0])
        {
            action = QIOT_AT_ACTION_EXEC;
        }

        if (QIOT_AT_ACTION_UNKOWN != action)
        {
            qint32_t ret;
            char *passData = NULL;
            if((HAL_STRCMP("QIOTSEND",table[i].cmd) == 0 && 2 == size) ||
               (HAL_STRCMP("QIOTMODELTD",table[i].cmd) == 0 && (2 == size || 3 == size)))
            {
                Quos_logPrintf(APP_AT, LL_ERR, "need wait data,len:%d",atoi(words[1]));
                cmdTestSocketSend(sockFd, (const quint8_t *)"> ", HAL_STRLEN("> "));
                passData = cmdTestPassMode(sockFd,atoi(words[1]));
                if(passData)
                {
                    words[size++] = passData;
                }
            }
            ret = table[i].cb(action, retBuf, sizeof(retBuf), size, words);
            if (ret == 0)
            {
                cmdTestSocketSend(sockFd, (const quint8_t *)"OK\r\n", HAL_STRLEN("OK\r\n"));
            }
            else if (ret < 0)
            {
                cmdTestSocketSend(sockFd, (const quint8_t *)"ERROR\r\n", HAL_STRLEN("ERROR\r\n"));
            }
            else
            {
                cmdTestSocketSend(sockFd, (const quint8_t *)retBuf, ret);
                cmdTestSocketSend(sockFd, (const quint8_t *)"\r\n\r\nOK\r\n", HAL_STRLEN("\r\n\r\nOK\r\n"));
            }
            if(passData)
            {
                free(passData);
            }
        }
        i++;
    }
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void *cmdTestTask(void *arg)
{
    fd_set rset;
    UNUSED(arg);
    while (1)
    {
        qint32_t bufLen;
        quint8_t buf[1024];
        FD_ZERO(&rset);
        FD_SET(STDIN_FILENO, &rset);
        int result = select(STDIN_FILENO + 1, &rset, NULL, NULL, NULL);
        if (result > 0 && (bufLen = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
        {
            buf[bufLen] = 0;
            atCmdAnalyze(SOCKET_FD_INVALID, (char *)buf);
        }
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void cmdTestSocketSend(pointer_t sockFd, const quint8_t *data, quint32_t len)
{
    TWLLHead_T *temp, *next;
    Quos_logPrintf(APP_AT, LL_DBG, "data[%d]:\n%s", len, data);
    if (SOCKET_FD_INVALID == sockFd)
    {
        TWLIST_FOR_DATA(appSockHead, temp, next)
        {
            appSock_T *listTemp = __GET_STRUCT_BY_ELEMENT(temp, appSock_T, head);
            sockFd = listTemp->sockFd;
        }
    }
    if(data && len)
    {
        send(sockFd, data, len, 0);
    }
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void *cmdTestSocketRecvTask(void *arg)
{
    pointer_t sockFd = (pointer_t)arg;
    while (1)
    {
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(sockFd, &rset);
        int result = select(sockFd + 1, &rset, NULL, NULL, NULL);
        if (result < 0)
        {
            break;
        }
        else if (result > 0)
        {
            quint8_t buf[1024];
            qint32_t bufLen = read(sockFd, buf, sizeof(buf));
            if (bufLen > 0)
            {
                buf[bufLen] = '\0';
                Quos_logPrintf(APP_AT, LL_DBG, "recv:%s", buf);
                atCmdAnalyze(sockFd, (char *)buf);
            }
            else
            {
                break;
            }
        }
    }
    Quos_logPrintf(APP_AT, LL_DBG, "fd[%ld] disconnect", sockFd);
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA(appSockHead, temp, next)
    {
        appSock_T *listTemp = __GET_STRUCT_BY_ELEMENT(temp, appSock_T, head);
        if (listTemp->sockFd == sockFd)
        {
            Quos_twllHeadDelete(&appSockHead, temp);
            free(listTemp);
        }
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void *cmdTestSocketTask(void *arg)
{
    quint32_t len;
    pointer_t fd = (pointer_t)arg;
    Quos_logPrintf(APP_AT, LL_DBG, "fd[%ld]", fd);

    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    len = sizeof(client);

    while (1)
    {
        pthread_t pthreadId;
        pointer_t appAtFd = accept(fd, (struct sockaddr *)&client, &len);
        if (0 != pthread_create(&pthreadId, NULL, (void *)cmdTestSocketRecvTask, (void *)(pointer_t)appAtFd))
        {
            close(appAtFd);
            break;
        }
        appSock_T *listNew = (appSock_T *)malloc(sizeof(appSock_T));
        listNew->sockFd = appAtFd;
        if (listNew == NULL)
        {
            break;
        }
        Quos_twllHeadAdd(&appSockHead, &listNew->head);
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void CmdTestInit(void)
{
    pthread_t pthreadId;
    pthread_t pthreadSocketId;
    quint8_t sockType;
    pointer_t fd = Qhal_tcpServerInit(&sockType, APP_AT_SOCKET_PORT, 1);
    Quos_logPrintf(APP_AT, LL_DBG, "fd[%ld]", fd);
    pthread_create(&pthreadSocketId, NULL, (void *)cmdTestSocketTask, (void *)fd);
    pthread_create(&pthreadId, NULL, (void *)cmdTestTask, NULL);
}
