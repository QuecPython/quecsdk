/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    : 2021-02-09
** ����   @brief   : �������
** Ӳ��   @hardware��
** ����   @other   ��
***************************************************************************/
#include "quos_net.h"
#include "quos_swTimer.h"
#include "quos_event.h"
#include "quos_log.h"
#include "quos_SupportTool.h"
#include "quos_twll.h"
#include "Qhal_driver.h"

typedef struct
{
    TWLLHead_T head;
    char hostname[QUOS_DNS_HOSTNANE_MAX_LENGHT];
    quint8_t defaultIp[QUOS_IP_ADDR_MAX_LEN];
    quint8_t ip[DNS_IP_FROM_HOSTNAME_MAX_NUM][QUOS_IP_ADDR_MAX_LEN];
    qint8_t ip_num;
} Quos_netHostInfo_t;

static TWLLHead_T *HostInfoHead = NULL;
static void *NetRetryTimer = NULL;
static qbool NetIsConnected = FALSE;
/**************************************************************************
** ����	@brief : ��������Ӳ����������
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_netRetry(void *swtimer)
{
    quint32_t timeout = 30 * SWT_ONE_SECOND;
    Qhal_netOpen(&timeout);
    Quos_logPrintf(LSDK_NET, LL_DBG, "net open wait:%dms", timeout);
    if (NetIsConnected)
    {
        /* ��Qhal_netOpen����ִ�й����У����������ӳɹ��¼������Ѿ�ͨ������Quos_netIOStatusNotify�Ѿ���NetIsConnected����ΪTRUE�ˣ��ʴ˴���Ҫ�ж�NetIsConnected�Ƿ���ΪTRUE */
    }
    else if (0 == timeout)
    {
        if (Quos_eventPost(QUOS_SYSTEM_EVENT_NETWORK, (void *)QUOS_SEVENT_NET_CONNECTED))
        {
            Quos_swTimerTimeoutSet(swtimer, SWT_SUSPEND);
            NetIsConnected = TRUE;
            Quos_logPrintf(LSDK_NET, LL_DBG, "post EVENT_NETCONNECTED ok");
        }
        else
        {
            Quos_logPrintf(LSDK_NET, LL_ERR, "post EVENT_NETCONNECTED fail");
        }
    }
    else
    {
        if(0 != Quos_swTimerTimeoutGet(swtimer))
        {
            Quos_eventPost(QUOS_SYSTEM_EVENT_NETWORK, (void *)QUOS_SEVENT_NET_CONNTIMEOUT);
        }
        Quos_swTimerTimeoutSet(swtimer, timeout);
    }
}
/**************************************************************************
** ����	@brief : ���������
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_netOpen(void)
{
    Quos_logPrintf(LSDK_NET, LL_DBG, "NetRetryTimer:%p", NetRetryTimer);
    if (NULL == NetRetryTimer || SWT_SUSPEND == Quos_swTimerTimeoutGet(NetRetryTimer))
    {
        if (NetIsConnected && Quos_eventPost(QUOS_SYSTEM_EVENT_NETWORK, (void *)QUOS_SEVENT_NET_DISCONNECT))
        {
            NetIsConnected = FALSE;
        }
        Quos_swTimerStart(&NetRetryTimer, "net", 0, 0, quos_netRetry, NULL);
    }
}
/**************************************************************************
** ����	@brief : �ر��������
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_netClose(void)
{
    if (NetRetryTimer && SWT_SUSPEND == Quos_swTimerTimeoutGet(NetRetryTimer))
    {
        if (Quos_eventPost(QUOS_SYSTEM_EVENT_NETWORK, (void *)QUOS_SEVENT_NET_DISCONNECT))
        {
            Quos_logPrintf(LSDK_NET, LL_DBG, "post EVENT_NETDISCONNECT ok");
            NetIsConnected = FALSE;
            Quos_swTimerDelete(NetRetryTimer);
            Qhal_netClose();
        }
        else
        {
            Quos_logPrintf(LSDK_NET, LL_ERR, "post EVENT_NETDISCONNECT fail");
        }
    }
    else
    {
        Quos_swTimerDelete(NetRetryTimer);
    }
}
/**************************************************************************
** ����	@brief : ����״̬�仯֪ͨ
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_netIOStatusNotify(qbool isConn)
{
    Quos_logPrintf(LSDK_NET, LL_DBG, "isConn:%s", _BOOL2STR(isConn));
    if (NULL == NetRetryTimer)
    {
        return;
    }
    else if (SWT_SUSPEND == Quos_swTimerTimeoutGet(NetRetryTimer) && FALSE == isConn)
    {
        if (Quos_eventPost(QUOS_SYSTEM_EVENT_NETWORK, (void *)QUOS_SEVENT_NET_DISCONNECT))
        {
            NetIsConnected = FALSE;
            Quos_logPrintf(LSDK_NET, LL_INFO, "post EVENT_NETDISCONNECT ok");
        }
        else
        {
            Quos_logPrintf(LSDK_NET, LL_ERR, "post EVENT_NETDISCONNECT fail");
        }
        Quos_swTimerTimeoutSet(NetRetryTimer, 0);
    }
    else if (SWT_SUSPEND != Quos_swTimerTimeoutGet(NetRetryTimer) && TRUE == isConn)
    {
        if (Quos_eventPost(QUOS_SYSTEM_EVENT_NETWORK, (void *)QUOS_SEVENT_NET_CONNECTED))
        {
            Quos_logPrintf(LSDK_NET, LL_DBG, "post EVENT_NETCONNECTED ok");
            NetIsConnected = TRUE;
            Quos_swTimerTimeoutSet(NetRetryTimer, SWT_SUSPEND);
        }
        else
        {
            Quos_logPrintf(LSDK_NET, LL_ERR, "post EVENT_NETCONNECTED fail");
        }
    }
}
/**************************************************************************
** ����	@brief :����hostname���ҵ�ǰ�ڵ�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static Quos_netHostInfo_t *quos_hostnameNodeFind(const char *hostname)
{
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA(HostInfoHead, temp, next)
    {
        Quos_netHostInfo_t *node = __GET_STRUCT_BY_ELEMENT(temp, Quos_netHostInfo_t, head);
        if (0 == HAL_STRCMP(node->hostname, hostname))
        {
            Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] node ok", hostname);
            return node;
        }
    }
    Quos_logPrintf(LSDK_NET, LL_ERR, "no [%s] node", hostname);
    return NULL;
}
/**************************************************************************
** ����	@brief : ɾ�����������Ϣ����ڵ�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Quos_netHostnameDelete(const char *hostname)
{
    Quos_netHostInfo_t *node = quos_hostnameNodeFind(hostname);
    if (node)
    {
        Quos_twllHeadDelete(&HostInfoHead, &node->head);
        HAL_FREE(node);
        Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] node ok", hostname);
    }
}
/**************************************************************************
** ����	@brief : �������������Ϣ
** ����	@param : 
** ���	@retval: 
***************************************************************************/
Quos_netHostInfo_t *Quos_hostnameNodeAdd(const char *hostname)
{
    Quos_netHostInfo_t *node = quos_hostnameNodeFind(hostname);
    if (NULL == node && (node = HAL_MALLOC(sizeof(Quos_netHostInfo_t))) != NULL)
    {
        HAL_MEMSET(node, 0, sizeof(Quos_netHostInfo_t));
        Quos_twllHeadAdd(&HostInfoHead, &node->head);
        HAL_SNPRINTF(node->hostname, sizeof(node->hostname), "%s", hostname);
        Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] node ok", hostname);
    }

    return node;
}
/**************************************************************************
** ����	@brief :���������Ϣ����ڵ�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Quos_netHostnameSetDefault(const char *hostname, const char *ip)
{
    Quos_netHostInfo_t *node = Quos_hostnameNodeAdd(hostname);
    if (node)
    {
        if (ip && HAL_STRLEN(ip))
        {
            HAL_MEMCPY(node->defaultIp, ip, QUOS_IP_ADDR_MAX_LEN);
        }
        else
        {
            HAL_MEMSET(node->defaultIp, 0, QUOS_IP_ADDR_MAX_LEN);
        }
        Quos_logPrintf(LSDK_NET, LL_DBG, "[%s][%s] ok", hostname, ip);
    }
}
/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Quos_netHostnameDnsEnable(const char *hostname)
{
    Quos_netHostInfo_t *node = Quos_hostnameNodeAdd(hostname);
    if (node)
    {
        node->ip_num = HAL_STRLEN(node->defaultIp) ? -1 : 0;
        Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] %d ok", hostname, node->ip_num);
    }
    else
    {
        Quos_logPrintf(LSDK_NET, LL_ERR, "[%s] enable fail", hostname);
    }
}

/**************************************************************************
** ����	@brief : ����DNS��ȡipֵ
** ����	@param : 
** ���	@retval: 
***************************************************************************/
quint8_t *Quos_netGetIpFromHostname(const char *hostname)
{
    Quos_netHostInfo_t *node = quos_hostnameNodeFind(hostname);
    if (NULL == node || node->ip_num >= DNS_IP_FROM_HOSTNAME_MAX_NUM)
    {
        Quos_logPrintf(LSDK_NET, LL_ERR, "[%s] vaild", hostname);
        return NULL;
    }
#if 0 
    //xjin.gao 20211206 adaptation//depot10/quecthing/quecthingSDK/2.9.0/ node->ip_num Adapt to the "char", Will not be -1
    if (-1 == node->ip_num)
    {
        node->ip_num = 0;
        Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] default ip[%s]", hostname, node->defaultIp);
        return HAL_STRLEN(node->defaultIp) ? node->defaultIp : NULL;
    }
    else 
#endif
    if (0 == node->ip_num)
    {
        /* ʹ����������DNS���� */
        quint8_t *ipPtr[DNS_IP_FROM_HOSTNAME_MAX_NUM];
        for (/*NULL*/; node->ip_num < DNS_IP_FROM_HOSTNAME_MAX_NUM; node->ip_num++)
        {
            ipPtr[(quint8_t)node->ip_num] = node->ip[(quint8_t)node->ip_num];
        }
        node->ip_num = (quint8_t)Qhal_dns2IPGet(hostname, ipPtr, node->ip_num);
        Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] do dns[%d]", hostname, node->ip_num);
        while (node->ip_num--)
        {
            //xjin.gao 20211206 adaptation//depot10/quecthing/quecthingSDK/2.9.0/ node->ip_num Adapt to the "char", do not print this log
            //Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] [%d][%s]", hostname, node->ip_num, node->ip[node->ip_num]);
        }
        node->ip_num = 0;
    }
    return HAL_STRLEN(node->ip[(quint8_t)node->ip_num]) ? node->ip[(quint8_t)node->ip_num++] : NULL;
}
/**************************************************************************
** ����	@brief : �ײ����ӳɹ����֪��ǰ������Ӧ����Чip���
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Quos_netHostnameValidIpNumSet(const char *hostname)
{
    Quos_netHostInfo_t *node = quos_hostnameNodeFind(hostname);
    if (node && node->ip_num > 0)
    {
        HAL_MEMCPY(node->defaultIp, node->ip[(quint8_t)node->ip_num - 1], QUOS_IP_ADDR_MAX_LEN);
        Quos_logPrintf(LSDK_NET, LL_DBG, "[%s] set [%d][%s] to default", hostname, node->ip_num - 1, node->defaultIp);
    }
}
/**************************************************************************
** ����	@brief : ����������ȡ��Чip
** ����	@param : 
** ���	@retval: 
***************************************************************************/
qbool Quos_netHostnameValidIpGet(const char *hostname, char *ip)
{
    Quos_netHostInfo_t *node = quos_hostnameNodeFind(hostname);

    /* û���ҵ�������Ӧ�Ľڵ㣬���ߵ�ǰ����û�з���������ip��IPû�б仯���ʲ���Ҫ�����滻�ͱ��� */
    if (NULL == node || 0 == HAL_STRLEN(node->defaultIp))
    {
        return FALSE;
    }
    if (ip)
    {
        HAL_MEMCPY((quint8_t *)ip, node->defaultIp, QUOS_IP_ADDR_MAX_LEN);
    }
    return TRUE;
}
