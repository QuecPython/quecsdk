/*************************************************************************
** ������ @author  : ����
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : ���ڲ���ͨ�ŵĶ����ʺ���Ӧʱ��
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
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
** ����	@brief : �¼�����ص�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen)
{
    printf("event[%d] errcode[%d] valLen[%d]\r\n", event, errcode, valLen);
    switch (event)
    {
    /* ������֤���� */
    case QIOT_ATEVENT_TYPE_AUTH:
        printf("auth event,code:%d\r\n", errcode);
        break;
    /* ������� */
    case QIOT_ATEVENT_TYPE_CONN:
        printf("connect event,code:%d\r\n", errcode);
        break;
    /* ���Ĳ��� */
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
    {
        printf("subcribe event,code:%d\r\n", errcode);
        break;
    }
    /* �������ݲ��� */
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
    /* �������ݲ��� */
    case QIOT_ATEVENT_TYPE_RECV:
        printf("data recv event,code:%d\r\n", errcode);
        /* �յ�͸������ */
        if (QIOT_RECV_SUCC_TRANS == errcode)
        {
            printf("pass data:%.*s\r\n", valLen, (char *)value);
            /* ���ԣ����յ���͸�����ݻش���ƽ̨ */
            Ql_iotCmdBusPassTransSend(0, (unsigned char *)value, valLen);
        }
        /* �յ���ģ���·����� */
        else if (QIOT_RECV_SUCC_PHYMODEL_RECV == errcode)
        {
            /* ���ԣ����յ�����ģ�����ݻش���ƽ̨ */
            Ql_iotCmdBusPhymodelReport(0, value);
        }
        /* �յ���ģ���������� */
        else if (QIOT_RECV_SUCC_PHYMODEL_REQ == errcode && value)
        {
        }
        break;
    /* ע������ */
    case QIOT_ATEVENT_TYPE_LOGOUT:
        printf("logout event,code:%d\r\n", errcode);
        break;
    /* FOTA���� */
    case QIOT_ATEVENT_TYPE_OTA:
        printf("ota event,code:%d\r\n", errcode);
        break;
    /* ƽ̨�¼� */
    case QIOT_ATEVENT_TYPE_SERVER:
        printf("server event,code:%d\r\n", errcode);
        break;
    default:
        break;
    }
}

/**************************************************************************
** ����	@brief : ����͸����ʱ��Ӧ
** ����	@param : 
** ���	@retval: 
***************************************************************************/
static void ql_iotNetcheckTimeoutCB(SWTimer_T *swtimer)
{
    qbool *tout = (qbool *)swtimer->parm;
    *tout = TRUE;
}

/**************************************************************************
** ����	@brief : ͳ�ƶ����� ����
** ����	@param : 
** ���	@retval: 
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
** ����	@brief : �����ʼ�����
** ����	@param : 
** ���	@retval: 
***************************************************************************/
int FUNCTION_ATTR_ROM main(void)
{
    /* ��ʼ��quecsdk */
    Ql_iotInit();
    /* ע���¼��ص����� */
    Ql_iotConfigSetEventCB(Ql_iotEventCB);
    /* ���ò�Ʒ��Ϣ*/
    Ql_iotConfigSetProductinfo(QIOT_MQTT_PRODUCT_KEY, QIOT_MQTT_PRODUCT_SECRET);
    /* ���÷�������Ϣ����ѡ��Ĭ������MQTT�������������� */
    Ql_iotConfigSetServer(QIOT_PPROTOCOL_MQTT, QIOT_MQTT_REGISTER_URL);
    /* ������ƽ̨���� */
    Ql_iotConfigSetConnmode(QIOT_CONNMODE_REQ);
    while (1)
    {
        QIot_state_e status = Ql_iotGetWorkState();
        printf("work status:%d\r\n", status);
        ql_lossrate_task();
        sleep(100);
    }
}
