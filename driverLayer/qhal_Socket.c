/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : tcp
** 硬件   @hardware：Linux平台
** 其他   @other   ：
***************************************************************************/
#include "qhal_Socket.h"
#include "Qhal_types.h"
#include "Ql_iotApi.h"


#include "netdb.h"
#include "ip4.h"
#include "igmp.h"
#include "netif.h"
#include "helios_datacall.h"
#include "helios_os.h"
#include "ssl.h"
#include "net_sockets.h"
#include "entropy.h"
#include "ctr_drbg.h"
#if defined (PLAT_Unisoc)
#include "errno.h"
#include "sockets.h"
#elif defined (PLAT_ASR)
#include "sockets.h"
#endif


extern char *Qhal_softversionGet(void);


typedef struct
{
    mbedtls_ssl_context ssl_ctx;
    mbedtls_net_context net_ctx;
    mbedtls_ssl_config ssl_conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_x509_crt_profile profile;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
} SockSslContext_T;

/**************************************************************************
** 功能	@brief : socket发送,适用于LINUX TCP/UART
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Qhal_sockWrite(pointer_t sockFd, quint8_t type, const void *peer, const quint8_t *buf, quint16_t bufLen, qbool *isSending)
{
    quint16_t offset = 0;
    UNUSED(peer);
    Quos_logPrintf(HAL_SOCK, LL_DBG, "sockFd:" PRINTF_FD " type:%d len:%d", sockFd, type, bufLen);
    Quos_logHexDump(HAL_SOCK, LL_DUMP, "", (void *)buf, bufLen);
    while (offset < bufLen)
    {
        int writeLen = -1;
        switch (type)
        {
        case SOCKET_TYPE_UART:
        case SOCKET_TYPE_TCP_CLI:
        case SOCKET_TYPE_UDP:
            writeLen = write(sockFd, buf + offset, bufLen - offset);
            if (writeLen <= 0)
            {
                Quos_logPrintf(HAL_SOCK, LL_DBG, "sockFd:" PRINTF_FD " error:%d", sockFd, (int)errno);
            }
            break;
        case SOCKET_TYPE_TCP_SSL_CLI:
        case SOCKET_TYPE_UDP_SSL_CLI:
        {
            SockSslContext_T *sockSslCtx = (SockSslContext_T *)sockFd;
            writeLen = mbedtls_ssl_write(&sockSslCtx->ssl_ctx, buf + offset, bufLen - offset);
            if(MBEDTLS_ERR_SSL_WANT_READ == writeLen || MBEDTLS_ERR_SSL_WANT_WRITE == writeLen)
            {
                *isSending = TRUE;
                writeLen = bufLen - offset;
            }
            break;
        }
        default:
            break;
        }
        if (writeLen <= 0)
        {
            Quos_logPrintf(HAL_SOCK, LL_ERR, "sockFd[" PRINTF_FD "] type[%d] Write fail:len[%d %d/%d]", sockFd, type, writeLen, offset, bufLen);
            return FALSE;
        }
        Quos_logPrintf(HAL_SOCK, LL_DBG, "len:%d", (int)writeLen);
        offset += writeLen;
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : socket关闭
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Qhal_sockClose(pointer_t sockFd, quint8_t type)
{
    Quos_logPrintf(HAL_SOCK, LL_INFO, "close sockFd=" PRINTF_FD " type:%d", sockFd, type);
    switch (type)
    {
    case SOCKET_TYPE_UART:
    case SOCKET_TYPE_TCP_CLI:
    case SOCKET_TYPE_UDP:
    case SOCKET_TYPE_TCP_LISTEN:
        shutdown(sockFd, 2);
        close(sockFd);
        break;
    case SOCKET_TYPE_TCP_SSL_CLI:
    case SOCKET_TYPE_UDP_SSL_CLI:
    {
        SockSslContext_T *sockSslCtx = (SockSslContext_T *)sockFd;
        mbedtls_net_free(&sockSslCtx->net_ctx);
        mbedtls_ssl_close_notify(&sockSslCtx->ssl_ctx);
        mbedtls_x509_crt_free(&sockSslCtx->cacert);
        mbedtls_x509_crt_free(&sockSslCtx->clicert);
        mbedtls_pk_free(&sockSslCtx->pkey);
        mbedtls_ssl_free(&sockSslCtx->ssl_ctx);
        mbedtls_ssl_config_free(&sockSslCtx->ssl_conf);
        mbedtls_ctr_drbg_free(&sockSslCtx->ctr_drbg);
        mbedtls_entropy_free(&sockSslCtx->entropy);
        break;
    }
    default:
        break;
    }
}

/**************************************************************************
** 功能	@brief : UDP初始化
** 输入	@param :
** 输出	@retval:
***************************************************************************/
pointer_t FUNCTION_ATTR_ROM Qhal_udpInit(quint8_t *type, quint16_t l_port, const char *hostname, quint16_t r_port, void **peer)
{
    UNUSED(type);
    UNUSED(l_port);
    UNUSED(hostname);
    UNUSED(r_port);
    UNUSED(peer);
    return SOCKET_FD_INVALID;
}
/**************************************************************************
** 功能	@brief : tcp server初始化
** 输入	@param :
** 输出	@retval:
***************************************************************************/
pointer_t FUNCTION_ATTR_ROM Qhal_tcpServerInit(quint8_t *type, quint16_t l_port, quint8_t maxClient)
{
    UNUSED(type);
    UNUSED(l_port);
    UNUSED(maxClient);
    return SOCKET_FD_INVALID;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/


static void qhal_SockTcpRecvTask(void *arg)
{
    pointer_t sockFd = (pointer_t)arg;
    quint8_t tcp_buf[1400] = {0};
    int32_t bufLen = 0;
    int result = 0;
    Quos_logPrintf(HAL_TCP, LL_INFO,"tcp s");
    Quos_socketIOConnResult(sockFd, SOCKET_TYPE_TCP_CLI,  TRUE);
    while (1)
    {
       fd_set rset;
       FD_ZERO(&rset);
       FD_SET(sockFd, &rset);
       bufLen = 0;
       result = 0;
       result = select(sockFd + 1, &rset, NULL, NULL, NULL);
       if (result < 0)
       {
           break;
       }
       else if (result > 0)
       {
            bufLen = read(sockFd, tcp_buf, sizeof(tcp_buf));
            if (bufLen > 0)
            {
                Quos_socketIORx(sockFd, SOCKET_TYPE_TCP_CLI, NULL, tcp_buf, bufLen);
            }
            else
            {
                break;
            }
        }
    }
    Quos_logPrintf(HAL_SOCK, LL_DBG, "sockFd:" PRINTF_FD " len:%d result %d error:%d", sockFd, (int)bufLen, result, (int)errno);
    Quos_socketIORx(sockFd, SOCKET_TYPE_TCP_CLI, NULL, NULL, 0);
	Helios_Thread_Delete(Helios_Thread_GetID());
}

/**************************************************************************
** 功能	@brief : TCP client connect
** 输入	@param :
** 输出	@retval:
***************************************************************************/
pointer_t FUNCTION_ATTR_ROM Qhal_tcpClientInit(quint8_t *type, const char *hostname, quint16_t r_port, quint32_t *connectTimeout)
{
	UNUSED(type);
    UNUSED(hostname);
    UNUSED(r_port);
    UNUSED(connectTimeout);
    Quos_logPrintf(HAL_TCP, LL_DBG, "hostname:%s[%d]", hostname, r_port);
    struct addrinfo hints = {0}, *dns = NULL, *node = NULL;
    char portStr[6] = {0};
    int ret = 0;
	int profile_idx = Ql_iotConfigGetPdpContextId();
    HAL_SPRINTF(portStr, "%d", r_port);
    *type = SOCKET_TYPE_TCP_CLI;


    Helios_DataCallInfoStruct info;
	struct in_addr qhal_ip4_addr = {0};
    char ip4_addr_str[16] = {0};
    HAL_MEMSET(&info, 0, sizeof(Helios_DataCallInfoStruct));
    Helios_DataCall_GetInfo(profile_idx, 0, &info);
    
	Quos_logPrintf(HAL_TCP, LL_INFO,"info.profile_idx: %d\r\n", (int)info.profile_idx);
	Quos_logPrintf(HAL_TCP, LL_INFO,"info.ip_version: %d\r\n", (int)info.ip_version);
	Quos_logPrintf(HAL_TCP, LL_INFO,"info.v4.state: %d\r\n", (int)info.v4.state);
	Quos_logPrintf(HAL_TCP, LL_INFO,"info.v4.reconnect: %d\r\n", (int)info.v4.reconnect);

	inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
	Quos_logPrintf(HAL_TCP, LL_INFO,"info.v4.addr.ip: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
	Quos_logPrintf(HAL_TCP, LL_INFO,"info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
	Quos_logPrintf(HAL_TCP, LL_INFO,"info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
    
    qhal_ip4_addr = info.v4.addr.ip;
	if(!info.v4.state)
	{
		return SOCKET_FD_INVALID;
	}
	
    hints.ai_socktype = SOCK_STREAM;
    if (0 != getaddrinfo_with_pcid(hostname, NULL, &hints, &dns, profile_idx) || NULL == dns)
    {
        Quos_logPrintf(HAL_TCP, LL_ERR, "getaddrinfo [%s] fail!", hostname);
        return SOCKET_FD_INVALID;
    }
    node = dns;
    while (node)
    {
        int sockFd = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
        if (sockFd >= 0)
        {
            int flag = 0;
			int32_t size = 1;
            struct timeval tm;
            Helios_ThreadAttr TcpRecvTask;
            fd_set set;
			struct sockaddr_in	ip4_local_addr = {0};

            TcpRecvTask.name = "qhal_tcp";
            TcpRecvTask.argv = (void *)(pointer_t)sockFd;
            TcpRecvTask.entry = qhal_SockTcpRecvTask;
            TcpRecvTask.priority = QHAL_APP_TASK_PRIORITY;
            TcpRecvTask.stack_size = 1024*10;

            
            tm.tv_sec = 5;
            tm.tv_usec = 0;
            FD_ZERO(&set);
            FD_SET(sockFd, &set);
                        
			flag = fcntl(sockFd, F_GETFL, 0);             //获取文件的flags值
			fcntl(sockFd, F_SETFL, flag | O_NONBLOCK);    //设置成非阻塞模式

            ip4_local_addr.sin_family = AF_INET;
            ip4_local_addr.sin_port = 0;
            ip4_local_addr.sin_addr = qhal_ip4_addr;

            ret = bind(sockFd, (struct sockaddr *)&ip4_local_addr, sizeof(ip4_local_addr));
            if(ret < 0)
        	{
        		Quos_logPrintf(HAL_TCP, LL_ERR, "bind fail");
                lwip_freeaddrinfo(dns);
        	    return SOCKET_FD_INVALID;
        	}
            struct sockaddr_in	* ip4_svr_addr;
            ip4_svr_addr = (struct sockaddr_in *)dns->ai_addr;
	        ip4_svr_addr->sin_port = htons(r_port);
            
            if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &size, sizeof(int32_t)) < 0)
            {
                Quos_logPrintf(HAL_TCP, LL_ERR, "socket REUSEADDR err");
                close(sockFd);
            }
            else if (connect(sockFd, (struct sockaddr *)ip4_svr_addr, sizeof(struct sockaddr)) == 0 ||
                     select(sockFd + 1, NULL, &set, NULL, &tm) > 0)
            {
				flag = fcntl(sockFd, F_GETFL, 0);             //获取文件的flags值。
				fcntl(sockFd, F_SETFL, flag | ~O_NONBLOCK);   //设置成阻塞模式；
				lwip_freeaddrinfo(dns);
				if(0 >= Helios_Thread_Create(&TcpRecvTask))
				{
					close(sockFd);
					return SOCKET_FD_INVALID;
				}
                *connectTimeout = 3*SWT_ONE_SECOND;
				Quos_logPrintf(HAL_TCP, LL_ERR, "qhal_SockTcpRecvTask create ok");
				return (pointer_t)sockFd;
            }
            else
            {
                close(sockFd);
            }
        }
        node = node->ai_next;
    }
    lwip_freeaddrinfo(dns);
    Quos_logPrintf(HAL_TCP, LL_ERR, "connect server fail");
    return SOCKET_FD_INVALID;
}

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void qhal_SockTcpTlsRecvTask(void *arg)
{
    SockSslContext_T *sockSslCtx = (SockSslContext_T *)arg;
	quint8_t buf[1400] = {0};
    Quos_logPrintf(HAL_TCP, LL_INFO,"tcp t");
    while (1)
    {
        int len = mbedtls_ssl_read(&sockSslCtx->ssl_ctx, buf, sizeof(buf));
        if (len > 0)
        {
            Quos_socketIORx((pointer_t)sockSslCtx, SOCKET_TYPE_TCP_SSL_CLI, NULL, buf, len);
        }
        else if (MBEDTLS_ERR_SSL_WANT_READ != len && MBEDTLS_ERR_SSL_WANT_WRITE != len)
        {
            break;
        }
    }
    
    Quos_logPrintf(HAL_TCP, LL_INFO,"tcp q");
    Quos_socketIORx((pointer_t)sockSslCtx, SOCKET_TYPE_TCP_SSL_CLI, NULL, NULL, 0);
    HAL_FREE(sockSslCtx);
	Helios_Thread_Delete(Helios_Thread_GetID());
}

extern void mbedtls_debug_set_threshold(int threshold);

static void mbedtls_debug_print(void *p_dbg, int level, const char *file, int line, const char *idstr)
{
    UNUSED(level);
    printf("< %s > %s[%d] %s", (char *)p_dbg, file, line, idstr);
}
/**************************************************************************
** 功能	@brief : TCP SSL client connect
** 输入	@param :
** 输出	@retval:
***************************************************************************/
pointer_t FUNCTION_ATTR_ROM Qhal_tcpSslClientInit(quint8_t *type, const char *hostname, quint16_t r_port, quint32_t *connectTimeout)
{
    UNUSED(connectTimeout);
    Quos_logPrintf(HAL_TLS, LL_DBG, "hostname:%s[%d]", hostname, r_port);
    int ret = 0;
    SockSslContext_T *sockSslCtx = HAL_MALLOC(sizeof(SockSslContext_T));
    if (NULL == sockSslCtx)
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "malloc sockSslCtx fail");
        return SOCKET_FD_INVALID;
    }
    *type = SOCKET_TYPE_TCP_SSL_CLI;
    HAL_MEMSET(sockSslCtx, 0, sizeof(SockSslContext_T));
    mbedtls_net_init(&sockSslCtx->net_ctx);
    mbedtls_ssl_init(&sockSslCtx->ssl_ctx);
    mbedtls_ssl_config_init(&sockSslCtx->ssl_conf);
    mbedtls_x509_crt_init(&sockSslCtx->cacert);
    mbedtls_x509_crt_init(&sockSslCtx->clicert);
    mbedtls_pk_init(&sockSslCtx->pkey);
    mbedtls_ctr_drbg_init(&sockSslCtx->ctr_drbg);
    mbedtls_debug_set_threshold(0);
    mbedtls_entropy_init(&sockSslCtx->entropy);
    
	
    mbedtls_ssl_conf_handshake_timeout(&sockSslCtx->ssl_conf, 1000, 30000);
    if ((ret = mbedtls_ctr_drbg_seed(&sockSslCtx->ctr_drbg,
                                     mbedtls_entropy_func,
                                     &sockSslCtx->entropy,
                                     (const quint8_t *)Qhal_softversionGet(), HAL_STRLEN(Qhal_softversionGet()))) != 0)
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "ctr_drbg_seed fail ret=-0x%X", -ret);
        goto exit;
    }

    /*if( && (ret =mbedtls_x509_crt_parse( &sockSslCtx->cacert,  )))
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "mbedtls_x509_crt_parse fail ret=-0x%X", -ret);
        goto exit;
    }*/
    Quos_logPrintf(HAL_TLS, LL_DBG, "net_connect");
    if ((ret = mbedtls_net_connect(&sockSslCtx->net_ctx, Ql_iotConfigGetPdpContextId(), hostname, (char *)(int)r_port, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "x509_crt_parse fail ret=-0x%X", -ret);
        goto exit;
    }
    if ((ret = mbedtls_ssl_config_defaults(&sockSslCtx->ssl_conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "ssl_config_defaults fail ret=-0x%X", -ret);
        goto exit;
    }

    /* TODO: add customerization encryption algorithm */
    HAL_MEMCPY(&sockSslCtx->profile, sockSslCtx->ssl_conf.cert_profile, sizeof(mbedtls_x509_crt_profile));
    sockSslCtx->profile.allowed_mds = sockSslCtx->profile.allowed_mds | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_MD5);
    mbedtls_ssl_conf_cert_profile(&sockSslCtx->ssl_conf, &sockSslCtx->profile);

    mbedtls_ssl_conf_authmode(&sockSslCtx->ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(&sockSslCtx->ssl_conf, &sockSslCtx->cacert, NULL);
    mbedtls_ssl_conf_rng(&sockSslCtx->ssl_conf, mbedtls_ctr_drbg_random, &sockSslCtx->ctr_drbg);

    mbedtls_ssl_conf_dbg(&sockSslCtx->ssl_conf, mbedtls_debug_print, (void *)"MBEDTLS");

    if ((ret = mbedtls_ssl_setup(&sockSslCtx->ssl_ctx, &sockSslCtx->ssl_conf)) != 0)
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "ssl_setup fail ret=-0x%X", -ret);
        goto exit;
    }
    if ((ret = mbedtls_ssl_set_hostname(&sockSslCtx->ssl_ctx, hostname)) != 0)
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "ssl_set_hostname fail ret=-0x%X", -ret);
        goto exit;
    }

    mbedtls_ssl_set_bio(&sockSslCtx->ssl_ctx, &sockSslCtx->net_ctx, mbedtls_net_send, mbedtls_net_recv, NULL);
    Quos_logPrintf(HAL_TLS, LL_DBG, "ssl_handshake");
    while ((ret = mbedtls_ssl_handshake(&sockSslCtx->ssl_ctx)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            Quos_logPrintf(HAL_TLS, LL_ERR, "ssl_handshake fail ret=-0x%X", -ret);
            goto exit;
        }
    }
    if ((ret = mbedtls_ssl_get_verify_result(&sockSslCtx->ssl_ctx)) != 0)
    {
        Quos_logPrintf(HAL_TLS, LL_ERR, "ssl_get_verify_result fail ret=-0x%X", -ret);
        goto exit;
    }
    else
    {
        Helios_ThreadAttr TcpTlsRecvTask;
        TcpTlsRecvTask.name = "qhal_tls";
        TcpTlsRecvTask.argv = (void *)sockSslCtx;
        TcpTlsRecvTask.entry = qhal_SockTcpTlsRecvTask;
        TcpTlsRecvTask.priority = QHAL_APP_TASK_PRIORITY+2;
        TcpTlsRecvTask.stack_size = 1024*10;
        if(0 >= Helios_Thread_Create(&TcpTlsRecvTask))
        {
            Quos_logPrintf(HAL_TLS, LL_ERR, "pthread fail");
            goto exit;
        }
        Quos_logPrintf(HAL_TLS, LL_INFO, "tls client ok");
        return (pointer_t)sockSslCtx;
    }

exit:
    Quos_logPrintf(HAL_TLS, LL_DBG, "err");

    mbedtls_net_free(&sockSslCtx->net_ctx);
    mbedtls_ssl_close_notify(&sockSslCtx->ssl_ctx);
    mbedtls_x509_crt_free(&sockSslCtx->cacert);
    mbedtls_x509_crt_free(&sockSslCtx->clicert);
    mbedtls_pk_free(&sockSslCtx->pkey);
    mbedtls_ssl_free(&sockSslCtx->ssl_ctx);
    mbedtls_ssl_config_free(&sockSslCtx->ssl_conf);
    mbedtls_ctr_drbg_free(&sockSslCtx->ctr_drbg);
    mbedtls_entropy_free(&sockSslCtx->entropy);
    HAL_FREE(sockSslCtx);
    return SOCKET_FD_INVALID;
}

/**************************************************************************
** 功能	@brief : UDP SSL client connect
** 输入	@param :
** 输出	@retval:
***************************************************************************/
pointer_t FUNCTION_ATTR_ROM Qhal_udpSslInit(quint8_t *type, quint16_t l_port, const char *hostname, quint16_t r_port)
{
    UNUSED(type);
    UNUSED(l_port);
    UNUSED(hostname);
    UNUSED(r_port);
    return SOCKET_FD_INVALID;
}
