/*************************************************************************
** ´´½¨ÈË @author  : Îâ½¡³¬ JCWu
** °æ±¾   @version : V1.0.0 Ô­Ê¼°æ±¾
** ÈÕÆÚ   @date    :
** ¹¦ÄÜ   @brief   : 
** Ó²¼þ   @hardware£ºLinuxÆ½Ì¨
** ÆäËû   @other   £º
***************************************************************************/
#include "qhal_Dev.h"
#include "qhal_FlashOpt.h"
#include "Qhal_types.h"
#include "Ql_iotApi.h"


#include "helios_rtc.h"
#include "helios_dev.h"
#include "helios_power.h"
#include "helios_os.h"
#include "helios_datacall.h"
#include "helios_fota.h"
#include "helios_fs.h"

#define FILE_READ_BUF_LEN 1024

#define CONFIG_KERNEL_HWTICK_FREQ 2000000
#if (CONFIG_KERNEL_HWTICK_FREQ == 16384)
#define __OSI_HWTICK_CMN_DIV_MS (8)
#define __OSI_HWTICK_CMN_DIV_US (64)
#define __OSI_HWTICK_CMN_DIV_16K (16384)
#define __OSI_HWTICK_CMN_DIV_32K (16384)
#endif

#if (CONFIG_KERNEL_HWTICK_FREQ == 2000000)
#define __OSI_HWTICK_CMN_DIV_MS (1000)
#define __OSI_HWTICK_CMN_DIV_US (1000000)
#define __OSI_HWTICK_CMN_DIV_16K (128)
#define __OSI_HWTICK_CMN_DIV_32K (128)
#endif

#if (CONFIG_KERNEL_HWTICK_FREQ == 1000000)
#define __OSI_HWTICK_CMN_DIV_MS (1000)
#define __OSI_HWTICK_CMN_DIV_US (1000000)
#define __OSI_HWTICK_CMN_DIV_16K (64)
#define __OSI_HWTICK_CMN_DIV_32K (64)
#endif
#define __OSI_TICK_CONVERT(t, sfreq, dfreq, cdiv) (((t) * ((dfreq) / (cdiv))) / ((sfreq) / (cdiv)))
#define OSI_TICK_CONVERT(t, sfreq, dfreq, cdiv) ((int64_t)__OSI_TICK_CONVERT((int64_t)(t), (unsigned)(sfreq), (unsigned)(dfreq), (unsigned)(cdiv)))
#define OSI_HWTICK_TO_MS(t) OSI_TICK_CONVERT(t, CONFIG_KERNEL_HWTICK_FREQ, 1000, __OSI_HWTICK_CMN_DIV_MS)

static Systick_T RtcTime = {0,0};
HAL_LOCK_DEF(static, lockMallocId)
//HAL_LOCK_DEF(static, lockKernelId)
/**************************************************************************
** ¹¦ÄÜ	@brief : ¿´ÃÅ¹·Î¹¹·
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
void FUNCTION_ATTR_ROM Qhal_devFeeddog(void)
{
}
/**************************************************************************
** ¹¦ÄÜ	@brief : ³õÊ¼»¯RTC
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
qbool FUNCTION_ATTR_ROM Qhal_rtcInit(void)
{
    Quos_logPrintf(HAL_DEV, LL_DBG, "%llu", Helios_RTC_GetTicks());
#if defined (PLAT_Unisoc)
    RtcTime.sec = OSI_HWTICK_TO_MS(Helios_RTC_GetTicks())/1000;
	RtcTime.ms  = OSI_HWTICK_TO_MS(Helios_RTC_GetTicks())%1000;
#elif defined (PLAT_ASR)
    Helios_RTCTime tm;
    Quos_tm_t tm_t;
    Helios_RTC_GetTime(&tm);
    tm_t.tm_hour = tm.tm_hour;
    tm_t.tm_mday = tm.tm_mday;
    tm_t.tm_min = tm.tm_min;
    tm_t.tm_mon = tm.tm_mon;
    tm_t.tm_sec = tm.tm_sec;
    tm_t.tm_year = tm.tm_year;
    RtcTime.sec = Quos_mktime(tm_t);
	RtcTime.ms = 0;
#else
#endif
    return TRUE;
}
/**************************************************************************
** ¹¦ÄÜ	@brief : ÉèÖÃRTCÊ±¼ä
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
void FUNCTION_ATTR_ROM Qhal_rtcSet(quint32_t timestamp, quint16_t ms)
{
	RtcTime.sec = timestamp;
	RtcTime.ms = ms;
	Quos_sysTickRectify(RtcTime);
}
/**************************************************************************
** ¹¦ÄÜ	@brief : »ñÈ¡RTCÊ±¼ä´Á
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
void FUNCTION_ATTR_ROM Qhal_rtcGet(quint32_t *sec, quint32_t *ms)
{
#if defined (PLAT_Unisoc)
    RtcTime.sec = OSI_HWTICK_TO_MS(Helios_RTC_GetTicks())/1000;
	RtcTime.ms  = OSI_HWTICK_TO_MS(Helios_RTC_GetTicks())%1000;
#elif defined (PLAT_ASR)
    static quint64_t oldtick = 0;
	quint64_t nowtick = Helios_RTC_GetTicks();
	RtcTime.ms += (nowtick - oldtick)/32;
	oldtick = nowtick-(nowtick - oldtick)%32;
	if(RtcTime.ms>1000)
	{
		RtcTime.sec += RtcTime.ms/1000;
		RtcTime.ms %= 1000;
	}
#else
#error tick
#endif
	
	if(sec)
	{
		*sec = RtcTime.sec;
	}
	if(ms)
	{
		*ms = RtcTime.ms;
	}
}
/**************************************************************************
** ¹¦ÄÜ	@brief : »ñÈ¡Èí¼þ°æ±¾
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/

char *FUNCTION_ATTR_ROM Qhal_softversionGet(void)
{
    static char mob_sw_rev[60];
    Helios_Dev_GetFwVersion((void *)mob_sw_rev, sizeof(mob_sw_rev));
    return (char *)mob_sw_rev;
}
/**************************************************************************
** ¹¦ÄÜ	@brief : µ±Ç°Ê±¼äµÄ×Ö·û´®¸ñÊ½
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
char FUNCTION_ATTR_ROM *Qhal_logHeadString(void)
{
    quint32_t sec, ms;
    Qhal_rtcGet(&sec, &ms);
    Quos_tm_t tm = Quos_localtime(sec);
    static char cmd[50];
	HAL_MEMSET(cmd, 0, sizeof(cmd));
    HAL_SPRINTF(cmd, "[%02d%02d%02d:%03d]", tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
    return cmd;
    //return "";
}
/**************************************************************************
** ¹¦ÄÜ	@brief : ÖØÆôÓ¦ÓÃ
** ÊäÈë	@param :
** Êä³ö	@retval:
***************************************************************************/
void FUNCTION_ATTR_ROM Qhal_devRestart(void)
{
    Quos_logPrintf(HAL_DEV, LL_DBG, "reboot now!");
    Helios_Power_Reset(1);
    return;
}
/**************************************************************************
** ¹¦ÄÜ	@brief : »ñÈ¡MAC
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
char FUNCTION_ATTR_ROM *Qhal_devUuidGet(void)
{
    static quint8_t imei[64] = {0};
    Helios_Dev_GetIMEI(imei,sizeof(imei));
    return (char *)imei;
}
/**************************************************************************
** ¹¦ÄÜ	@brief : »ñÈ¡Ëæ»úÊý
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
quint32_t FUNCTION_ATTR_RAM Qhal_randomGet(void)
{
    static quint8_t cnt = 0;
	quint32_t sec,min;
    if (0 == cnt % 10)
    {
		Qhal_rtcGet(&sec, &min);
        srand(sec);
    }
    cnt++;
    return rand();
}

/**************************************************************************
** ¹¦ÄÜ	@brief : µ÷ÓÃÓ²¼þAPIÊµÏÖ¹Ì¼þ¸üÐÂ
** ÊäÈë	@param : 
** Êä³ö	@retval: 
***************************************************************************/
#if defined (PLAT_Unisoc)
typedef struct
{
    int file;
    char *buffer;
    int data_size;
} helios_fota_ctx_s;
#endif
quint32_t  FUNCTION_ATTR_ROM Qhal_devOtaNotify(const char *filename, quint32_t fileSize)
{
    Quos_logPrintf(HAL_DEV, LL_ERR,"filename:%s size:%d",filename, fileSize);
    int ret = 0;
#if defined (PLAT_Unisoc)
    helios_fota_ctx_s ctx = {0};
#elif defined (PLAT_ASR)
    int ctx = 0;
    quint8_t * file_read_buf = NULL;
    quint32_t file_size = 0;
    quint32_t file_read_total_len = 0;
    HeliosFILE * fp = NULL;
    ctx = Helios_Fota_Init();
    if(!ctx)
    {
        Quos_logPrintf(HAL_DEV, LL_ERR,"init fail");
        goto exit;
    }

    Quos_logPrintf(HAL_DEV, LL_DBG,"init successfully");
#else
#error "FOTA"
#endif
    
#if defined (PLAT_Unisoc)
    ret = Helios_Fota_PackageVerify((int)&ctx);
    if(ret)
    {
        Quos_logPrintf(HAL_DEV, LL_ERR,"verify fail");
        goto exit;
    }
#elif defined (PLAT_ASR)
    fp = Helios_fopen(filename, "r");
    if(!fp)
    {
        Quos_logPrintf(HAL_DEV, LL_ERR,"open failed");
        goto exit;
    }

    file_read_buf = HAL_MALLOC(FILE_READ_BUF_LEN);
    if(!file_read_buf)
    {
        Quos_logPrintf(HAL_DEV, LL_ERR,"heap memory is not enough");
        goto exit;
    }

    file_size = Helios_fsize(fp);
    if(file_size <= 0 || fileSize != file_size)
    {
        Quos_logPrintf(HAL_DEV, LL_ERR,"file is empty or corrupted");
        goto exit;
    }
    Quos_logPrintf(HAL_DEV, LL_ERR,"size:%u", file_size);

    Quos_logPrintf(HAL_DEV, LL_DBG,"write image to flash");
    
    while((ret = Helios_fread(file_read_buf, FILE_READ_BUF_LEN, 1, fp)) > 0)
    {
        file_read_total_len += ret;
        
        ret = Helios_Fota_PackageWrite(ctx, (void *)file_read_buf, ret, file_size);
        if(ret)
        {
            Quos_logPrintf(HAL_DEV, LL_ERR,"write fail");
            goto exit;
        }
    }

    if(file_read_total_len != file_size)
    {
        Quos_logPrintf(HAL_DEV, LL_ERR,"read fail");
        goto exit;
    }
    
    if(file_read_total_len)
    {
        ret = Helios_Fota_PackageFlush(ctx);
        if(ret)
        {
            Quos_logPrintf(HAL_DEV, LL_ERR,"flush fail");
            goto exit;
        }
        
        Quos_logPrintf(HAL_DEV, LL_DBG,"write done, verifing");

        ret = Helios_Fota_PackageVerify(ctx);
        if(ret)
        {
            Quos_logPrintf(HAL_DEV, LL_ERR,"verify fail");
            goto exit;
        }
    }
#else

#endif  
#if defined (PLAT_Unisoc)
#elif defined (PLAT_ASR)
    if(fp) Helios_fclose(fp);
    if(file_read_buf) HAL_FREE(file_read_buf); 
    if(ctx) Helios_Fota_Deinit(ctx);
#else
#endif
    Qhal_devRestart();
    return (quint32_t)0;
exit:
#if defined (PLAT_Unisoc)
#elif defined (PLAT_ASR)
    if(fp) Helios_fclose(fp);
    if(file_read_buf) HAL_FREE(file_read_buf); 
    if(ctx) Helios_Fota_Deinit(ctx);
#else
#endif
    return (quint32_t)1000;
}

/**************************************************************************
** ¹¦ÄÜ	@brief : 
** ÊäÈë	@param : 
** Êä³ö	@retval: 
***************************************************************************/
static unsigned int msg_quos_task;

void  qhal_MainTask(void *argv)
{
    UNUSED(argv);
    Helios_Debug_Output("[INIT]qhal_MainTask");
    msg_quos_task = (unsigned int)Helios_MsgQ_Create(10, sizeof(quint32_t));
	
	if(msg_quos_task <= 0)
	{
		Helios_Debug_Output("[INIT]init msg_quos_task queue fail\r\n");
        Helios_Thread_Delete(Helios_Thread_GetID());
		return;
	}
    //Ql_iotInit();
    Quos_logPrintf(HAL_DEV, LL_DBG," qhal_MainTask\r\n");
    while (1)
    {
		quint32_t msg = 1;
		quint32_t idletime = Quos_kernelTask();
        Quos_logPrintf(HAL_DEV, LL_DBG," idletime exec:%u\r\n", idletime);
		if(idletime)
		{
            Helios_MsgQ_Get(msg_quos_task, (void *)&msg, sizeof(quint32_t), idletime);
		}
    }
	Helios_Thread_Delete(Helios_Thread_GetID());
}

qbool FUNCTION_ATTR_ROM Qhal_quecsdk_init(void)
{
    Helios_Debug_Enable();
    Helios_ThreadAttr qhal_main_ref;
    qhal_main_ref.name = "qhal_main";
    qhal_main_ref.argv = NULL;
    qhal_main_ref.entry = qhal_MainTask;
    qhal_main_ref.priority = QHAL_APP_TASK_PRIORITY;
    qhal_main_ref.stack_size = 1024 * 64;
    if(0 == Helios_Thread_Create(&qhal_main_ref))
    {
        Helios_Debug_Output("[INIT]Helios_Thread_Create fail");
        return FALSE;
    }
    return TRUE;
}
/**************************************************************************
** ¹¦ÄÜ	@brief : ÔÚÖ´ÐÐmainº¯ÊýÇ°ÏÈÖ´ÐÐ´Ëº¯Êý
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
qbool FUNCTION_ATTR_ROM Qhal_beforeMain(void)
{
    HAL_LOCK_INIT(lockMallocId);
    //HAL_LOCK_INIT(lockKernelId);
#if defined (PLAT_Unisoc)
    Quos_logPrintf(HAL_DEV, LL_DBG,"********** This is PLAT_Unisoc **********");
#elif defined (PLAT_ASR)
    Quos_logPrintf(HAL_DEV, LL_DBG,"********** This is PLAT_ASR **********");
#else
    Quos_logPrintf(HAL_DEV, LL_DBG,"********** This is unknown PLAT **********");
#endif
    return TRUE;
}

/**************************************************************************
** åŠŸèƒ½	@brief : é€€å‡ºä»»åŠ¡æŒ‚èµ·æ¨¡å¼
** è¾“å…¥	@param : 
** è¾“å‡º	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Qhal_KernelResume(void)
{
    quint32_t msg = 1;
    if(msg_quos_task)
    {
        Helios_MsgQ_Put(msg_quos_task, (void *)&msg, sizeof(quint32_t), HELIOS_NO_WAIT);       
    }   
    //HAL_UNLOCK(lockKernelId);
}

/**************************************************************************
** ¹¦ÄÜ	@brief : ÍË³öµÍ¹¦ºÄÄ£Ê½
** ÊäÈë	@param : 
** Êä³ö	@retval: 
***************************************************************************/
qbool Qhal_sleepExitDbg(const char *func, int line)
{
	quint32_t msg = 1;
	Quos_logPrintf(HAL_DEV, LL_DBG, "%s[%d]", func, line);
    if(msg_quos_task)
    {
        Helios_MsgQ_Put(msg_quos_task, (void *)&msg, sizeof(quint32_t), HELIOS_NO_WAIT);       
    }	
    return TRUE;
}
/**************************************************************************
** ¹¦ÄÜ	@brief : ³¢ÊÔ´ò¿ªÍøÂç
** ÊäÈë	@param : 
** Êä³ö	@retval: 
***************************************************************************/

static void qhal_datacall_status_cb(long int profile_idx, unsigned char sim_id, long int status)
{
    UNUSED(sim_id);
	Quos_logPrintf(HAL_SOCK, LL_ERR, "profile:%d status:%d", (int)profile_idx, (int)status);
	if(Ql_iotConfigGetPdpContextId() == profile_idx)
	{
        if (status == 1)
        {
    		Quos_netIOStatusNotify(TRUE);
        }
        else
        {
            Quos_netIOStatusNotify(FALSE);
        }
	}
}

void Qhal_netOpen(quint32_t *timeout)
{
	int profile_idx = Ql_iotConfigGetPdpContextId();
    Helios_DataCallInitStruct DataCallInitStruct = {0};
    Helios_DataCallInfoStruct info = {0};
    
    
    DataCallInitStruct.user_cb = qhal_datacall_status_cb;
    

    Quos_logPrintf(HAL_SOCK, LL_INFO, "net open");
    if(Helios_DataCall_Init(profile_idx, 0, &DataCallInitStruct) != 0)
	{
		Quos_logPrintf(HAL_SOCK, LL_ERR, "network register fail");
		*timeout = 15000;
	}
    Helios_DataCall_GetInfo(profile_idx, 0, &info);
    if (!info.v4.state)
    {
        *timeout = 150000;
    }
    else
    {
       *timeout = 0; 
    }
}
/**************************************************************************
** ¹¦ÄÜ	@brief : ¹Ø±ÕÍøÂ·
** ÊäÈë	@param : 
** Êä³ö	@retval: 
***************************************************************************/
void Qhal_netClose(void)
{
	Quos_logPrintf(HAL_SOCK, LL_INFO, "net close");
}
/**************************************************************************
** ¹¦ÄÜ	@brief : ÄÚ´æ
** ÊäÈë	@param :         
** Êä³ö	@retval:        
***************************************************************************/
void FUNCTION_ATTR_RAM *qhal_malloc(quint32_t len, const char *funName, qint32_t line)
{
    void *point;
    HAL_LOCK(lockMallocId);
    point = malloc(len);
    HAL_UNLOCK(lockMallocId);
    if (NULL == point)
    {
        HAL_PRINTF("malloc fail:%s[%d] len=%u\n", funName, line, len);
    }
    return point;
}
void FUNCTION_ATTR_RAM qhal_free(void *addr, const char *funName, qint32_t line)
{
    HAL_LOCK(lockMallocId);
    if (addr)
        free(addr);
    if (NULL == addr)
    {
        HAL_PRINTF("free fail:%s[%d]\n", funName, line);
    }
    HAL_UNLOCK(lockMallocId);
}
/**************************************************************************
** ¹¦ÄÜ	@brief : 
** ÊäÈë	@param : 
** Êä³ö	@retval: 
***************************************************************************/
void FUNCTION_ATTR_RAM *qhal_memdup(const void *buf, quint32_t len, const char *funName, qint32_t line)
{
    char *val = qhal_malloc(len, funName, line);
    if (NULL == val)
    {
        return NULL;
    }
    HAL_MEMCPY(val, buf, len);
    return (void *)val;
}
/**************************************************************************
** ¹¦ÄÜ	@brief : 
** ÊäÈë	@param : 
** Êä³ö	@retval: 
***************************************************************************/
char FUNCTION_ATTR_RAM *qhal_strdup(const char *str, const char *funName, qint32_t line)
{
    quint32_t len = HAL_STRLEN(str);
    char *p = (char *)qhal_memdup((const void *)str, len + 1, funName, line);
    if (p)
    {
        p[len] = 0;
    }
    return p;
}


