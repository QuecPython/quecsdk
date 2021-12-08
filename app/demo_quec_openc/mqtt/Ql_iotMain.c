/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : 
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
***************************************************************************/
#include "Ql_iotApi.h"
#define QIOT_MQTT_REGISTER_URL "iot-south.quectel.com:2883"
#define QIOT_MQTT_PRODUCT_KEY "p1115X"
#define QIOT_MQTT_PRODUCT_SECRET "d2c5Q1FsVWpwT1k3"


#define QIOT_MCU_COMPONENT_NO "MCU"
#define QIOT_MCU_VERSION "1"

/**************************************************************************
** ����	@brief : ��ģ�����ݱ���
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Ql_iotTtlvHandle(const void *ttlvHead)
{
    quint32_t count = Ql_iotTtlvCountGet(ttlvHead);
    quint32_t i;
    for(i=0;i<count;i++)
    {
        uint16_t id;
        QIot_dpDataType_e type;
        void *node = Ql_iotTtlvNodeGet(ttlvHead, i, &id, &type);
        if(node)
        {
            switch (type)
            {
            case QIOT_DPDATA_TYPE_BOOL:
            {
                qbool value;
                Ql_iotTtlvNodeGetBool(node, &value);
                printf("model id:%d data:%s\r\n",id,value?"TRUE":"FALSE");
                break;
            }
            case QIOT_DPDATA_TYPE_INT:
            {
                qint64_t num;
                Ql_iotTtlvNodeGetInt(node, &num);
                printf("model id:%d data:%ld\r\n",id,num);
                break;
            }  
            case QIOT_DPDATA_TYPE_FLOAT:
            {
                double num;
                Ql_iotTtlvNodeGetFloat(node, &num);
                printf("model id:%d data:%lg\r\n",id,num);
                break;
            }               
            case QIOT_DPDATA_TYPE_BYTE:
            {
                quint8_t *value;
                quint32_t len = Ql_iotTtlvNodeGetByte(node, &value);
                printf("model id:%d data:%.*s\r\n",id,len,value);
                break;
            }
            case QIOT_DPDATA_TYPE_STRUCT:
                Ql_iotTtlvHandle(Ql_iotTtlvNodeGetStruct(node));
                break;
            default:
                break;
            }
        }
    }
}


/**************************************************************************
** ����	@brief : �¼�����ص�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Ql_iotEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen)
{
    printf("event[%d] errcode[%d] valLen[%d]\r\n", event, errcode, valLen);
    switch (event)
    {
    /* ������֤���� */
    case QIOT_ATEVENT_TYPE_AUTH:
        printf("auth event,code:%d\r\n",errcode);
        break;
    /* ������� */
    case QIOT_ATEVENT_TYPE_CONN:
        printf("connect event,code:%d\r\n",errcode);
        break;
    /* ���Ĳ��� */
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
    {
        printf("subcribe event,code:%d\r\n",errcode);
        /* �ϱ��豸״̬��ģ����Ϣ */
        quint16_t statusIds[] = {QIOT_DPID_STATUS_BATTERY,
                                    QIOT_DPID_STATUS_VOLTAGE,
                                    QIOT_DPID_STATUS_SIGNAL,
                                    QIOT_DPID_STATUS_FLASHFREE,
                                    QIOT_DPID_STATUS_RSRP,
                                    QIOT_DPID_STATUS_RSRQ,
                                    QIOT_DPID_STATUS_SNR};
        quint16_t infoIds[] = {QIOT_DPID_INFO_MODEL_TYPE,
                                QIOT_DPID_INFO_MODEL_VER,
                                QIOT_DPID_INFO_MCU_VER,
                                QIOT_DPID_INFO_CELLID,
                                QIOT_DPID_INFO_ICCID,
                                QIOT_DPID_INFO_MCC,
                                QIOT_DPID_INFO_MNC,
                                QIOT_DPID_INFO_LAC,
                                QIOT_DPID_INFO_PHONE_NUM,
                                QIOT_DPID_INFO_SIM_NUM,
                                QIOT_DPID_INFO_SDK_VER,
                                QIOT_DPID_INFO_LOC_SUPLIST,
                                QIOT_DPIO_INFO_DP_VER,
                                QIOT_DPIO_INFO_CP_VER};
        Ql_iotCmdSysStatusReport(statusIds, sizeof(statusIds) / sizeof(statusIds[0]));
        Ql_iotCmdSysDevInfoReport(infoIds, sizeof(infoIds) / sizeof(infoIds[0]));

        /* �����ⲿ��λ���� */
        void *nmeaTtlv=NULL;
        Ql_iotTtlvIdAddString(&nmeaTtlv,0,"$GPGGA,042523.0,3413.610533,N,10854.063257,E,1,05,2.6,438.5,M,-28.0,M,,*78");
        Ql_iotTtlvIdAddString(&nmeaTtlv,0,"$GPRMC,042523.0,A,3413.610533,N,10854.063257,E,0.0,245.9,190716,0.0,E,A*0F");
        Ql_iotCmdBusLocReportOutside(nmeaTtlv);
        Ql_iotTtlvFree(&nmeaTtlv);

        /* �����ڲ���λ���� */
        void *titleTtlv = NULL;
        Ql_iotTtlvIdAddString(&titleTtlv, 0, "LBS");
        Ql_iotCmdBusLocReportInside(titleTtlv);
        Ql_iotTtlvFree(&titleTtlv);
        break;
    }
    /* �������ݲ��� */
    case QIOT_ATEVENT_TYPE_SEND:
        printf("data send event,code:%d\r\n",errcode);
        break;
    /* �������ݲ��� */
    case QIOT_ATEVENT_TYPE_RECV:
        printf("data recv event,code:%d\r\n",errcode);
        /* �յ�͸������ */
        if(QIOT_RECV_SUCC_TRANS == errcode)
        {
            printf("pass data:%.*s\r\n",valLen,(char *)value);
            /* ���ԣ����յ���͸�����ݻش���ƽ̨ */
            Ql_iotCmdBusPassTransSend(0,(unsigned char *)value,valLen);
        }
        /* �յ���ģ���·����� */
        else if(QIOT_RECV_SUCC_PHYMODEL_RECV == errcode)
        {
            /* ���ԣ����յ�����ģ�����ݻش���ƽ̨ */
            Ql_iotCmdBusPhymodelReport(0,value);
            /* ��ģ�����ݽ��� */
            Ql_iotTtlvHandle(value);
        }
        /* �յ���ģ���������� */
        else if(QIOT_RECV_SUCC_PHYMODEL_REQ == errcode && value)
        {
            quint16_t pkgId = *(quint16_t *)value;
            quint16_t *ids = (quint16_t *)(value+sizeof(quint16_t));
            void *ttlvHead = NULL;
            printf("model read event,pkgid:%d\r\n",pkgId);
            quint32_t i;
            for(i=0;i<valLen;i++)
            {
                quint16_t modelId = ids[i];
                printf("modelId:%d\r\n",modelId);
                /* id1:bool id2:int id3:string id4:int array id5:string array*/
                switch (modelId)
                {
                case 1:
                    Ql_iotTtlvIdAddBool(&ttlvHead, modelId, TRUE);
                    break;
                case 2:
                    Ql_iotTtlvIdAddInt(&ttlvHead, modelId, 1);
                    break;
                case 3:
                    Ql_iotTtlvIdAddString(&ttlvHead, modelId, "hello world");
                    break;
                case 4:
                    {
                        void *ttlvArrayHead = NULL;
                        Ql_iotTtlvIdAddInt(&ttlvArrayHead, 1, 1);
                        Ql_iotTtlvIdAddInt(&ttlvArrayHead, 2, 2);
                        Ql_iotTtlvIdAddInt(&ttlvArrayHead, 3, 3);
                        Ql_iotTtlvIdAddStruct(&ttlvHead, modelId, ttlvArrayHead);
                    }
                    break;
                case 5:
                    {
                        void *ttlvArrayHead = NULL;
                        Ql_iotTtlvIdAddString(&ttlvArrayHead, 1, "hello a");
                        Ql_iotTtlvIdAddString(&ttlvArrayHead, 2, "hello b");
                        Ql_iotTtlvIdAddString(&ttlvArrayHead, 3, "hello c");
                        Ql_iotTtlvIdAddStruct(&ttlvHead, modelId, ttlvArrayHead);
                    }
                    break;
                default:
                    break;
                }
            }
            Ql_iotCmdBusPhymodelAck(0, pkgId,ttlvHead);
            Ql_iotTtlvFree(&ttlvHead);
        }
        break;
    /* ע������ */
    case QIOT_ATEVENT_TYPE_LOGOUT:
        printf("logout event,code:%d\r\n",errcode);
        break;
    /* FOTA���� */
    case QIOT_ATEVENT_TYPE_OTA:
    {
        switch (errcode)
        {
        /* �·��������� */
        case QIOT_OTA_TASK_NOTIFY:
        {
            char *words[100];
            quint32_t count = Quos_stringSplit((char *)value,HAL_STRLEN(value), words, sizeof(words) / sizeof(words[0]),",", FALSE);
            printf("recv ota task\r\n");
            if(count < 6)
            {
                break;
            }
            printf("componentNo:%s,sourceVersion:%s,targetVersion:%s,batteryLimit:%s,minSignalIntensity:%s,minSignalIntensity:%s\r\n",
                words[0],words[1],words[2],words[3],words[4],words[5]);
            /* ȷ������ */
            Ql_iotCmdOtaAction(1);
            break;
        }
        /* ��ʼ���� */
        case QIOT_OTA_START:
            printf("ota download start\r\n");
            break;
        /* ������ */
        case QIOT_OTA_DOWNLOADING:
            printf("ota downloading\r\n");
            break;
        /* ������� */
        case QIOT_OTA_DOWNLOADED:
        {
            char *words[100];
            quint32_t count = Quos_stringSplit((char *)value,HAL_STRLEN(value), words, sizeof(words) / sizeof(words[0]),",", FALSE);
            printf("ota download complete\r\n");
            if(count < 4)
            {
                break;
            }
            printf("componentNo:%s,length:%s,startaddr:%s,piece_length:%s\r\n",words[0],words[1],words[2],words[3]);
            /* �����SOTA������ɣ���ͨ��API��ȡ�ļ� */
            if(strcmp(QIOT_MCU_COMPONENT_NO,Quos_stringRemoveMarks(words[0])) == 0)
            {
                quint8_t readBuf[1024];
                quint32_t ret = Ql_iotCmdOtaMcuFWDataRead(0,readBuf,sizeof(readBuf));
                printf("sota read file...ret:%d\r\n",ret);
                /* SOTA��ɺ�֪ͨ��ƽ̨MCU�������״̬ */
                Ql_iotCmdOtaAction(3);
            }
            break;
        }
        /* ������ */
        case QIOT_OTA_UPDATING:
            printf("ota updating\r\n");
            break;
        /* ������� */
        case QIOT_OTA_UPDATE_OK:
            printf("ota update success\r\n");
            break;
        /* ����ʧ�� */
        case QIOT_OTA_UPDATE_FAIL:
            printf("ota update fail\r\n");
            break;
        case QIOT_OTA_UPDATE_FLAG:
            if(((quint32_t *)value)[0] == 0)
            {
                printf("mutil devices confirm update\r\n");
            }
            else
                printf("mutil devices refuse update\r\n");
            break;   
        default:
            break;
        }
        break;  
    }  
    /* ƽ̨�¼� */
    case QIOT_ATEVENT_TYPE_SERVER:
        printf("server event,code:%d\r\n",errcode);
        break; 
    default:
        break;
    }
}
#ifdef QUEC_ENABLE_GATEWAY
/**************************************************************************
** ����	@brief : �¼�����ص�
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void Ql_iotSubEventCB(quint32_t event, qint32_t errcode, const char *subPk, const char *subDk, const void *value, quint32_t valLen)
{
    printf("subPk[%s] subDk[%s] event[%d] errcode[%d] valLen[%d]\r\n", subPk, subDk, event, errcode, valLen);
    if (valLen)
    printf("value:%.*s\r\n", valLen, (char *)value);
}
#endif
/**************************************************************************
** ����	@brief : �����ʼ�����
** ����	@param : 
** ���	@retval: 
***************************************************************************/
int main(void)
{
    /* ��ʼ��qucsdk */
    Ql_iotInit();
    /* ��������汾��ʶ */
    Ql_iotConfigSetAppVersion("app");
    /* ע���¼��ص����� */
    Ql_iotConfigSetEventCB(Ql_iotEventCB);
#ifdef QUEC_ENABLE_GATEWAY
    Ql_iotConfigSetSubDevEventCB(Ql_iotSubEventCB);
#endif
    /* ���ò�Ʒ��Ϣ*/
    Ql_iotConfigSetProductinfo(QIOT_MQTT_PRODUCT_KEY, QIOT_MQTT_PRODUCT_SECRET);
    /* ���÷�������Ϣ����ѡ��Ĭ������MQTT�������������� */
    Ql_iotConfigSetServer(QIOT_PPROTOCOL_MQTT,QIOT_MQTT_REGISTER_URL);
    /* ����PDP context Id����ѡ��Ĭ��Ϊ1 */
    Ql_iotConfigSetPdpContextId(1);
    /* ����lifetime����ѡ��MQTTĬ��Ϊ120 */
    Ql_iotConfigSetLifetime(120);
    /* �����ⲿMCU��ʶ�źͰ汾�ţ���ѡ����û���ⲿMCU����Ҫ���� */
    Ql_iotConfigSetMcuVersion(QIOT_MCU_COMPONENT_NO,QIOT_MCU_VERSION);
    /* ������ƽ̨���� */
    Ql_iotConfigSetConnmode(QIOT_CONNMODE_REQ);
    while (1)
    {
        QIot_state_e status = Ql_iotGetWorkState();
        printf("work status:%d\r\n",status);
        sleep(100);
    }
}
