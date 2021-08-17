/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 2020-11-23
** 功能   @brief   : 
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "Qhal_types.h"
#include "Ql_iotApi.h"
#include "qhal_property.h"

#include "helios_nw.h"
#include "helios_dev.h"
#include "helios_sim.h"
#include "helios_power.h"
#include "helios_datacall.h"
#include "helios_fs.h"
static char model_type[24] = {0};


/*
8421 bcd 转 十六进制，还不完善.
*/
static quint32_t bcd2hex(quint32_t bcd)
{
    quint32_t temp = 0;
    quint32_t index = 0;
    for (index = 0; bcd ; index++)
    {
        temp += (bcd & 0x0F) * (quint32_t)pow((double)10, (double)index);
        bcd = bcd >> 4;
    }
    return temp;
}

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Qhal_propertyDevGet(Qhal_propertyDev_t *dInfo)
{
    Helios_Dev_GetModel(model_type, 24);
    dInfo->cellLevel = (quint8_t)(Helios_Power_GetBatteryVol()/3300*100);
    dInfo->cellVoltage = (double)Helios_Power_GetBatteryVol()/1000.0; 
#if defined (PLAT_Unisoc)
    dInfo->flashFree = Helios_fs_free_size('U') - 10 * 1024; //暂时展锐不检查路径
#elif defined (PLAT_ASR)
    dInfo->flashFree = Helios_fs_free_size('U') - 10 * 1024;
#else
    dInfo->flashFree = Helios_fs_free_size('U') - 10 * 1024;
#endif
    
    dInfo->modelType = model_type;
    dInfo->pdpCxtIdMin = 1;
    dInfo->pdpCxtIdMax = 1; 
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Qhal_propertyNetGet(Qhal_propertyNet_t *nMasterInfo, Qhal_propertyNet_t (*nNeibhInfo)[], quint32_t maxCnt, quint32_t *neibhSize)
{
    UNUSED(maxCnt);

    Helios_NwSignalStrengthInfoStruct signal_info = {0};
    Helios_NwOperatorInfoStruct operator_info = {0};
    Helios_NwCellInfoStruct cell_info  = {0};
    Helios_NwConfigInfoStruct config_info = {0};
    //Helios_NwSelectionInfoStruct select_info = {0};
    Helios_NwRegisterStatusInfoStruct reg_info = {0};
    quint8_t cell_index = 0;

    Helios_Nw_GetSignalStrength(0, &signal_info);
    Helios_Nw_GetOperatorName(0, &operator_info);
    Helios_Nw_GetCellInfo(0, &cell_info);
    Helios_Nw_GetConfiguration(0, &config_info);  
    //Helios_Nw_GetSelection(0, &select_info);
    Helios_Nw_GetRegisterStatus(0, &reg_info);
    Quos_logPrintf(LL_DBG, LL_DBG, "net mode: %d\r\n", (int)config_info.net_mode);

    if (nMasterInfo)
    {
        nMasterInfo->snr = 0; 
        nMasterInfo->rsrp = 0;
        nMasterInfo->rsrq = 0;
        nMasterInfo->rssi = 0; 
        nMasterInfo->lac = 0;    
        nMasterInfo->mcc = 0; 
        nMasterInfo->mnc = 0;
        nMasterInfo->cellid = 0; 
        if (config_info.net_mode == HELIOS_NW_PREF_NET_TYPE_GSM)
        {
            nMasterInfo->rssi = signal_info.gw_signal_strength.rssi;   
            nMasterInfo->mcc = HAL_ATOI(operator_info.mcc); 
            nMasterInfo->mnc = HAL_ATOI(operator_info.mnc); 
            nMasterInfo->lac = reg_info.data_reg.lac;
            nMasterInfo->cellid = reg_info.data_reg.cid;
        }
        else if (config_info.net_mode == HELIOS_NW_PREF_NET_TYPE_UMTS)
        {
            nMasterInfo->rssi = cell_info.umts_info[0].rssi;
            nMasterInfo->mcc = HAL_ATOI(operator_info.mcc); 
            nMasterInfo->mnc = HAL_ATOI(operator_info.mnc); 
            nMasterInfo->lac = reg_info.data_reg.lac;
            nMasterInfo->cellid = reg_info.data_reg.cid;
        }
        else if(config_info.net_mode == HELIOS_NW_PREF_NET_TYPE_LTE)
        {
            nMasterInfo->rssi = signal_info.lte_signal_strength.rssi;
            nMasterInfo->rsrp = signal_info.lte_signal_strength.rsrp;
            nMasterInfo->rsrq = signal_info.lte_signal_strength.rsrq;
            nMasterInfo->mcc = HAL_ATOI(operator_info.mcc); 
            nMasterInfo->mnc = HAL_ATOI(operator_info.mnc); 
            nMasterInfo->lac = reg_info.data_reg.lac;
            nMasterInfo->cellid = reg_info.data_reg.cid;
        }
    }

    if (nNeibhInfo)
    {
        if (cell_info.gsm_info_valid)
        {
            Quos_logPrintf(LL_DBG, LL_DBG, "cell num: %d, valid:%d\r\n", (int)cell_info.gsm_info_num, (int)cell_info.gsm_info_valid);
            for (qint32_t i = 0; i < cell_info.gsm_info_num; i++)
            {
                if (cell_index > maxCnt -1)
                {
                    break;
                }
                (*nNeibhInfo)[cell_index].snr = 0;
                (*nNeibhInfo)[cell_index].rsrp = 0;
                (*nNeibhInfo)[cell_index].rsrq = 0;
                (*nNeibhInfo)[cell_index].rssi = cell_info.gsm_info[i].rssi;
                (*nNeibhInfo)[cell_index].lac = cell_info.gsm_info[i].lac;
                (*nNeibhInfo)[cell_index].mcc = bcd2hex(cell_info.gsm_info[i].mcc);
                (*nNeibhInfo)[cell_index].mnc = bcd2hex(cell_info.gsm_info[i].mnc);
                (*nNeibhInfo)[cell_index].cellid = cell_info.gsm_info[i].cid;
                cell_index++;
            }
        }
        if (cell_info.umts_info_valid)
        {
            Quos_logPrintf(LL_DBG, LL_DBG, "cell num: %d, valid:%d\r\n", (int)cell_info.umts_info_num, (int)cell_info.umts_info_valid);
            for (qint32_t i = 0; i < cell_info.umts_info_num; i++)
            {
                if (cell_index > maxCnt -1)
                {
                    break;
                }
                (*nNeibhInfo)[cell_index].snr = 0;
                (*nNeibhInfo)[cell_index].rsrp = 0;
                (*nNeibhInfo)[cell_index].rsrq = 0;
                (*nNeibhInfo)[cell_index].rssi = cell_info.umts_info[i].rssi;
                (*nNeibhInfo)[cell_index].lac = cell_info.umts_info[i].lac;
                (*nNeibhInfo)[cell_index].mcc = bcd2hex(cell_info.umts_info[i].mcc);
                (*nNeibhInfo)[cell_index].mnc = bcd2hex(cell_info.umts_info[i].mnc);
                (*nNeibhInfo)[cell_index].cellid = cell_info.umts_info[i].cid;
                cell_index++;
            }
        }
        if(cell_info.lte_info_valid)
        {
            Quos_logPrintf(LL_DBG, LL_DBG, "cell num: %d, valid:%d\r\n", (int)cell_info.lte_info_num, (int)cell_info.lte_info_valid);
            for (qint32_t i = 0; i < cell_info.lte_info_num; i++)
            {
                if (cell_index > maxCnt -1)
                {
                    break;
                }
                // mcc mnc 应该是BCD编码
                (*nNeibhInfo)[cell_index].snr = 0;
                (*nNeibhInfo)[cell_index].rsrp = 0;
                (*nNeibhInfo)[cell_index].rsrq = 0;
                (*nNeibhInfo)[cell_index].rssi = cell_info.lte_info[i].rssi;
                (*nNeibhInfo)[cell_index].lac = cell_info.lte_info[i].tac;
                (*nNeibhInfo)[cell_index].mcc = bcd2hex(cell_info.lte_info[i].mcc);
                (*nNeibhInfo)[cell_index].mnc = bcd2hex(cell_info.lte_info[i].mnc);
                (*nNeibhInfo)[cell_index].cellid = cell_info.lte_info[i].cid;
                cell_index++;
                Quos_logPrintf(LL_DBG, LL_DBG, "rssi %d,lac %d,mcc %d,mnc %d,cellid %d \r\n", (int)(*nNeibhInfo)[cell_index].rssi, (int)(*nNeibhInfo)[cell_index].lac, \
                                    (int)(*nNeibhInfo)[cell_index].mcc, (int)(*nNeibhInfo)[cell_index].mnc, (int)(*nNeibhInfo)[cell_index].cellid);
            }
        }
        if (neibhSize)
        {
            *neibhSize = cell_index;
        }
    }
    
    return TRUE;
}


/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Qhal_propertySimGet(Qhal_propertySim_t *sInfo)
{
    Helios_SIM_GetIMSI(0, sInfo->imsi, sizeof(sInfo->imsi));
    Helios_SIM_GetICCID(0, sInfo->iccid, sizeof(sInfo->iccid));
    Helios_SIM_GetPhoneNumber(0, sInfo->phoneid, sizeof(sInfo->phoneid));
    return TRUE;
}

/**************************************************************************
** 功能	@brief : 根据平台当前模式返回支持的定位功能列表
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t Qhal_propertyLocSupList(char **words, quint32_t maxSize)
{
    UNUSED(maxSize);
    quint32_t i = 0;
    words[i++] = QIOT_LOC_SUPPORT_NONE;
    // words[i++] = QIOT_LOC_SUPPORT_AUTO;
    words[i++] = QIOT_LOC_SUPPORT_LBS;
    return i;
}

/**************************************************************************
** 功能	@brief : 根据平台请求的定位类型，将数据添加到ttlv中
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Qhal_propertyGnssRawDataRead(void **ttlv, const char *title)
{
    qbool ret = FALSE;
    if (0 == HAL_STRCMP(title, "GPGGA") || 0 == HAL_STRCMP(title, "GGA"))
    {
        ret = TRUE;
        Ql_iotTtlvIdAddString(ttlv, 0, "$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F");
    }
    if (0 == HAL_STRCMP(title, "GNGGA") || 0 == HAL_STRCMP(title, "GGA"))
    {
        ret = TRUE;
        Ql_iotTtlvIdAddString(ttlv, 0, "$GNGGA,022106.000,3034.6466,N,10403.5680,E,1,7,1.04,470.2,M,-31.9,M,,*6B");
    }
    return ret;
}