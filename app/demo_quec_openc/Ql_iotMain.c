/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
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
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
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
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Ql_iotEventCB(quint32_t event, qint32_t errcode, const void *value, quint32_t valLen)
{
    Quos_logPrintf(LAPP_MAIN, LL_DBG, "event[%d] errcode[%d] valLen[%d]", event, errcode, valLen);
    switch (event)
    {
    /* 引导认证操作 */
    case QIOT_ATEVENT_TYPE_AUTH:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "auth event,code:%d",errcode);
        break;
    /* 接入操作 */
    case QIOT_ATEVENT_TYPE_CONN:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "connect event,code:%d",errcode);
        break;
    /* 订阅操作 */
    case QIOT_ATEVENT_TYPE_SUBCRIBE:
    {
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "subcribe event,code:%d",errcode);
        /* 上报设备状态和模组信息 */
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
    /* 发送数据操作 */
    case QIOT_ATEVENT_TYPE_SEND:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "data send event,code:%d",errcode);
        break;
    /* 接收数据操作 */
    case QIOT_ATEVENT_TYPE_RECV:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "data recv event,code:%d",errcode);
        /* 收到透传数据 */
        if(QIOT_RECV_SUCC_TRANS == errcode)
        {
            Quos_logHexDump(LAPP_MAIN, LL_DBG,"pass data:",(unsigned char *)value,valLen);
            /* 测试，把收到的透传数据回传到平台 */
            Ql_iotCmdBusPassTransSend(0,(unsigned char *)value,valLen);
        }
        /* 收到物模型下发数据 */
        else if(QIOT_RECV_SUCC_PHYMODEL_RECV == errcode)
        {
            /* 测试，把收到的物模型数据回传到平台 */
            Ql_iotCmdBusPhymodelReport(0,value);
            /* 物模型数据解析 */
            Ql_iotTtlvHandle(value);
        }
        /* 收到物模型请求数据 */
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
    /* 注销操作 */
    case QIOT_ATEVENT_TYPE_LOGOUT:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "logout event,code:%d",errcode);
        break;
    /* FOTA操作 */
    case QIOT_ATEVENT_TYPE_OTA:
    {
        switch (errcode)
        {
        /* 下发升级任务 */
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
            /* 确认升级 */
            Ql_iotCmdOtaAction(1);
            break;
        }
        /* 开始下载 */
        case QIOT_OTA_START:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota download start");
            break;
        /* 下载中 */
        case QIOT_OTA_DOWNLOADING:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota downloading");
            break;
        /* 下载完成 */
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
            /* 如果是SOTA下载完成，则通过API读取文件 */
            if(HAL_STRCMP(QIOT_MCU_COMPONENT_NO,Quos_stringRemoveMarks(words[0])) == 0)
            {
                quint8_t readBuf[1024];
                quint32_t ret = Ql_iotCmdOtaMcuFWDataRead(0,readBuf,sizeof(readBuf));
                Quos_logPrintf(LAPP_MAIN, LL_DBG, "sota read file...ret:%d",ret);
                /* SOTA完成后通知云平台MCU进入更新状态 */
                Ql_iotCmdOtaAction(3);
            }
            break;
        }
        /* 更新中 */
        case QIOT_OTA_UPDATING:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota updating");
            break;
        /* 更新完成 */
        case QIOT_OTA_UPDATE_OK:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota update success");
            break;
        /* 更新失败 */
        case QIOT_OTA_UPDATE_FAIL:
            Quos_logPrintf(LAPP_MAIN, LL_DBG, "ota update fail");
            break;
        default:
            break;
        }
        break;  
    }  
    /* 平台事件 */
    case QIOT_ATEVENT_TYPE_SERVER:
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "server event,code:%d",errcode);
        break; 
    default:
        break;
    }
}

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
int FUNCTION_ATTR_ROM main(void)
{
    /* 初始化qucsdk */
    Ql_iotInit();
    /* 设置软件版本标识 */
    Ql_iotConfigAppendAppVersion("app");
    /* 注册事件回调函数 */
    Ql_iotConfigSetEventCB(Ql_iotEventCB);
    /* 配置产品信息*/
    Ql_iotConfigSetProductinfo(QIOT_MQTT_PRODUCT_KEY, QIOT_MQTT_PRODUCT_SECRET);
    /* 配置服务器信息，可选，默认连接MQTT生产环境服务器 */
    Ql_iotConfigSetServer(QIOT_PPROTOCOL_MQTT,QIOT_MQTT_REGISTER_URL);
    /* 配置PDP context Id，可选，默认为1 */
    Ql_iotConfigSetPdpContextId(1);
    /* 配置lifetime，可选，MQTT默认为120 */
    Ql_iotConfigSetLifetime(120);
    /* 配置外部MCU标识号和版本号，可选，如没有外部MCU则不需要配置 */
    Ql_iotConfigSetMcuVersion(QIOT_MCU_COMPONENT_NO,QIOT_MCU_VERSION);
    /* 启动云平台连接 */
    Ql_iotConfigSetConnmode(QIOT_CONNMODE_REQ);
    while (1)
    {
        QIot_state_e status = Ql_iotGetWorkState();
        Quos_logPrintf(LAPP_MAIN, LL_DBG, "work status:%d",status);
        sleep(100);
    }
}
