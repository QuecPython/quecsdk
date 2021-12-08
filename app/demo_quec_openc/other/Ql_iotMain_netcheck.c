/*************************************************************************
** 创建人 @author  : 杨粟
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 用于测试通信的丢包率和响应时间
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "Ql_iotApi.h"
#define QIOT_MQTT_REGISTER_URL "192.168.25.64:30004"
#define QIOT_MQTT_PRODUCT_KEY "p113ed"
#define QIOT_MQTT_PRODUCT_SECRET "Q0poU3ZvTVBlbXR6"

#define QLDEMO_NETCHECKTIMES 10
#define QLDEMO_NETCHECKSTR "NETWORKCHECK"
typedef enum
{
    TRANSDATA_SUCCEED = 0U,
    TRANSDATA_FAILED,
    TRANSDATA_NONACK,
} ql_transSend_flag_e;
static ql_transSend_flag_e ql_transSend_flag;

SWTimer_T *QIot_netcheckt = NULL;

/**************************************************************************
** 功能	@brief : 事件处理回调
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen)
{
    printf("event[%d] errcode[%d] valLen[%d]\r\n", event, errcode, valLen);
    switch (event)
    {
    /* 引导认证操作 */
    case QIOT_ATEVENT_TYPE_AUTH:
        printf("auth event,code:%d\r\n", errcode);
        break;
    /* 接入操作 */
    case QIOT_ATEVENT_TYPE_CONN:
        printf("connect event,code:%d\r\n", errcode);
        break;
    /* 订阅操作 */
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
    {
        printf("subcribe event,code:%d\r\n", errcode);
        break;
    }
    /* 发送数据操作 */
    case QIOT_ATEVENT_TYPE_SEND:
        printf("data send event,code:%d\r\n", errcode);
        if (QIOT_SEND_SUCC_TRANS == errcode)
        {
            ql_transSend_flag = TRANSDATA_SUCCEED;
        }
        else if (QIOT_SEND_ERR_TRANS == errcode)
        {
            ql_transSend_flag = TRANSDATA_FAILED;
        }
        break;
    /* 接收数据操作 */
    case QIOT_ATEVENT_TYPE_RECV:
        printf("data recv event,code:%d\r\n", errcode);
        /* 收到透传数据 */
        if (QIOT_RECV_SUCC_TRANS == errcode)
        {
            printf("pass data:%.*s\r\n", valLen, (char *)value);
            /* 测试，把收到的透传数据回传到平台 */
            Ql_iotCmdBusPassTransSend(0, (unsigned char *)value, valLen);
        }
        /* 收到物模型下发数据 */
        else if (QIOT_RECV_SUCC_PHYMODEL_RECV == errcode)
        {
            /* 测试，把收到的物模型数据回传到平台 */
            Ql_iotCmdBusPhymodelReport(0, value);
        }
        /* 收到物模型请求数据 */
        else if (QIOT_RECV_SUCC_PHYMODEL_REQ == errcode && value)
        {
        }
        break;
    /* 注销操作 */
    case QIOT_ATEVENT_TYPE_LOGOUT:
        printf("logout event,code:%d\r\n", errcode);
        break;
    /* FOTA操作 */
    case QIOT_ATEVENT_TYPE_OTA:
        printf("ota event,code:%d\r\n", errcode);
        break;
    /* 平台事件 */
    case QIOT_ATEVENT_TYPE_SERVER:
        printf("server event,code:%d\r\n", errcode);
        break;
    default:
        break;
    }
}

/**************************************************************************
** 功能	@brief : 发送透传超时响应
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotNetcheckTimeoutCB(SWTimer_T *swtimer)
{
    qbool *tout = (qbool *)swtimer->parm;
    *tout = TRUE;
}

/**************************************************************************
** 功能	@brief : 统计丢包率 阻塞
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_lossrate_task(void)
{
    quint16_t i = 0, j = 0;
    quint16_t mode = 1;
    quint32_t responseTime;
    Systick_T pretime, nextime;
    quint64_t averageTime = 0;
    qbool timeout = FALSE;
    float lossrate = 0.0, averageTime_f = 0.0;
    while (i < QLDEMO_NETCHECKTIMES)
    {
        i++;
        Quos_swTimerStart(&QIot_netcheckt, "NET", SWT_ONE_SECOND * 5, 0, ql_iotNetcheckTimeoutCB, &timeout);
        Ql_iotCmdBusPassTransSend(mode, (quint8_t *)QLDEMO_NETCHECKSTR, sizeof(QLDEMO_NETCHECKSTR));
        timeout = FALSE;
        pretime = Quos_sysTickGet();
        do
        {
            usleep(1000);
        } while ((ql_transSend_flag == TRANSDATA_NONACK) && (!timeout));
        switch (ql_transSend_flag)
        {
        case TRANSDATA_SUCCEED:
        {
            j++;
            nextime = Quos_sysTickGet();
            responseTime = Quos_sysTickdiff(pretime, nextime, TRUE);
            averageTime += responseTime;
            break;
        }
        case TRANSDATA_FAILED:

        case TRANSDATA_NONACK: /* timeout */
        {
            HAL_PRINTF("connection failed. check connection\r\n");
            break;
        }
        default:

            break;
        }
        Quos_swTimerDelete(QIot_netcheckt);
        ql_transSend_flag = TRANSDATA_NONACK;
        sleep(1);
    }
    if (j)
    {
        averageTime /= j;
    }
    averageTime_f = averageTime;
    averageTime_f /= 1000;
    if (i)
    {
        lossrate = (float)(i - j) / (float)i;
    }
    else
        lossrate = 1.0;
    printf("message sended %d times, pkg lossrate is %.3f%%.\r\n", i, lossrate * 100);
    printf("average responsetime is %.3f seconds.\r\n", averageTime_f);
}

/**************************************************************************
** 功能	@brief : 程序初始化入口
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
int FUNCTION_ATTR_ROM main(void)
{
    /* 初始化quecsdk */
    Ql_iotInit();
    /* 注册事件回调函数 */
    Ql_iotConfigSetEventCB(Ql_iotEventCB);
    /* 配置产品信息*/
    Ql_iotConfigSetProductinfo(QIOT_MQTT_PRODUCT_KEY, QIOT_MQTT_PRODUCT_SECRET);
    /* 配置服务器信息，可选，默认连接MQTT生产环境服务器 */
    Ql_iotConfigSetServer(QIOT_PPROTOCOL_MQTT, QIOT_MQTT_REGISTER_URL);
    /* 启动云平台连接 */
    Ql_iotConfigSetConnmode(QIOT_CONNMODE_REQ);
    while (1)
    {
        QIot_state_e status = Ql_iotGetWorkState();
        printf("work status:%d\r\n", status);
        ql_lossrate_task();
        sleep(100);
    }
}
