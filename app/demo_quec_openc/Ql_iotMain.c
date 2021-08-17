/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : 
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
***************************************************************************/
#include "Ql_iotApi.h"
#include "Qhal_driver.h"
#define QIOT_MQTT_REGISTER_URL "http://iot-south.quectel.com:2883"
#define QIOT_MQTT_PRODUCT_KEY "p1115X"
#define QIOT_MQTT_PRODUCT_SECRET "d2c5Q1FsVWpwT1k3"

#define QIOT_COAP_BOOTSTRAP_URL "coap://220.180.239.212:8416"
#define QIOT_COAP_PRODUCT_KEY "MXJac1VyQUV1emZj"
#define QIOT_COAP_PRODUCT_SECRET "NlZGU3JUR0pYRGlR"

#define QIOT_MCU_COMPONENT_NO "MCU"
#define QIOT_MCU_VERSION "1"

#define LAPP_MAIN LL_DBG


/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotTtlvHandle(const void *ttlvHead)
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
                Quos_logPrintf(LAPP_MAIN, LL_DBG,"model id:%d data:%s",id,_BOOL2STR(value));
                break;
            }
            case QIOT_DPDATA_TYPE_INT:
            {
                qint64_t num;
                Ql_iotTtlvNodeGetInt(node, &num);
                Quos_logPrintf(LAPP_MAIN, LL_DBG,"model id:%d data:%ld",id,num);
                break;
            }  
            case QIOT_DPDATA_TYPE_FLOAT:
            {
                double num;
                Ql_iotTtlvNodeGetFloat(node, &num);
                Quos_logPrintf(LAPP_MAIN, LL_DBG,"model id:%d data:%lg",id,num);
                break;
            }               
            case QIOT_DPDATA_TYPE_BYTE:
            {
                quint8_t *value;
                quint32_t len = Ql_iotTtlvNodeGetByte(node, &value);
                Quos_logPrintf(LAPP_MAIN, LL_DBG,"model id:%d",id);
                Quos_logHexDump(LAPP_MAIN, LL_DBG,"",value, len);
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
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen)
{
    Quos_logPrintf(LAPP_MAIN, LL_DBG, "event[%d] errcode[%d] valLen[%d]", event, errcode, valLen);
    switch (event)
    {
    /* ������֤���� */
    case QIOT_ATEVENT_TYPE_AUTH:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "auth event,code:%d",errcode);
        break;
    /* ������� */
    case QIOT_ATEVENT_TYPE_CONN:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "connect event,code:%d",errcode);
        break;
    /* ���Ĳ��� */
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
    {
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "subcribe event,code:%d",errcode);
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
                                QIOT_DPID_INFO_LOC_SUPLIST};
        Ql_iotCmdSysStatusReport(statusIds, sizeof(statusIds) / sizeof(statusIds[0]));
        Ql_iotCmdSysDevInfoReport(infoIds, sizeof(infoIds) / sizeof(infoIds[0]));
        break;
    }
    /* �������ݲ��� */
    case QIOT_ATEVENT_TYPE_SEND:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "data send event,code:%d",errcode);
        break;
    /* �������ݲ��� */
    case QIOT_ATEVENT_TYPE_RECV:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "data recv event,code:%d",errcode);
        /* �յ�͸������ */
        if(QIOT_RECV_SUCC_TRANS == errcode)
        {
            Quos_logHexDump(LAPP_MAIN, LL_DBG,"pass data:",(unsigned char *)value,valLen);
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
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "model read event,pkgid:%d",pkgId);
            quint32_t i;
            for(i=0;i<valLen;i++)
            {
                quint16_t modelId = ids[i];
                Quos_logPrintf(LAPP_MAIN, LL_DBG, "modelId:%d",modelId);
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
            Ql_iotCmdBusPhymodelAck(0,pkgId,ttlvHead);
            Ql_iotTtlvFree(&ttlvHead);
        }
        break;
    /* ע������ */
    case QIOT_ATEVENT_TYPE_LOGOUT:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "logout event,code:%d",errcode);
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
            quint32_t count = Quos_stringSplit((char *)value, words, sizeof(words) / sizeof(words[0]),",", FALSE);
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "recv ota task");
            if(count < 6)
            {
                break;
            }
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "componentNo:%s,sourceVersion:%s,targetVersion:%s,batteryLimit:%s,minSignalIntensity:%s,minSignalIntensity:%s",
                words[0],words[1],words[2],words[3],words[4],words[5]);
            /* ȷ������ */
            Ql_iotCmdOtaAction(1);
            break;
        }
        /* ��ʼ���� */
        case QIOT_OTA_START:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota download start");
            break;
        /* ������ */
        case QIOT_OTA_DOWNLOADING:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota downloading");
            break;
        /* ������� */
        case QIOT_OTA_DOWNLOADED:
        {
            char *words[100];
            quint32_t count = Quos_stringSplit((char *)value, words, sizeof(words) / sizeof(words[0]),",", FALSE);
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota download complete");
            if(count < 4)
            {
                break;
            }
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "componentNo:%s,length:%s,startaddr:%s,piece_length:%s",words[0],words[1],words[2],words[3]);
            /* �����SOTA������ɣ���ͨ��API��ȡ�ļ� */
            if(HAL_STRCMP(QIOT_MCU_COMPONENT_NO,Quos_stringRemoveMarks(words[0])) == 0)
            {
                quint8_t readBuf[1024];
                quint32_t ret = Ql_iotCmdOtaMcuFWDataRead(0,readBuf,sizeof(readBuf));
                Quos_logPrintf(LAPP_MAIN, LL_DBG, "sota read file...ret:%d",ret);
                /* SOTA��ɺ�֪ͨ��ƽ̨MCU�������״̬ */
                Ql_iotCmdOtaAction(3);
            }
            break;
        }
        /* ������ */
        case QIOT_OTA_UPDATING:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota updating");
            break;
        /* ������� */
        case QIOT_OTA_UPDATE_OK:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota update success");
            break;
        /* ����ʧ�� */
        case QIOT_OTA_UPDATE_FAIL:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota update fail");
            break;
        default:
            break;
        }
        break;  
    }  
    /* ƽ̨�¼� */
    case QIOT_ATEVENT_TYPE_SERVER:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "server event,code:%d",errcode);
        break; 
    default:
        break;
    }
}

/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
int FUNCTION_ATTR_ROM main(void)
{
    /* ��ʼ��qucsdk */
    Ql_iotInit();
    /* ��������汾��ʶ */
    Ql_iotConfigAppendAppVersion("app");
    /* ע���¼��ص����� */
    Ql_iotConfigSetEventCB(Ql_iotEventCB);
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
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "work status:%d",status);
        sleep(100);
    }
}
