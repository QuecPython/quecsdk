/*************************************************************************
**		    源码未经检录,使用需谨慎
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 事件分发管理
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_event.h"
#include "Quos_kernel.h"
#if (SDK_ENABLE_EVENT == 1)
typedef struct
{
    qint32_t event;
    void *arg;
}EventArg_t;

typedef struct
{
    TWLLHead_T head;
    qint32_t eventId;
    quint32_t cbArraySize;
    EventCB_f *cbArray;
} EventNode_t;
static TWLLHead_T *EventList = NULL;

/**************************************************************************
** 功能	@brief : 事件注册
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_eventCbReg(const qint32_t eventArray[], quint32_t eventCnt, EventCB_f eventCb)
{
    quint32_t i;
    for (i = 0; i < eventCnt; i++)
    {
        EventNode_t *newNode = NULL;
        TWLLHead_T *temp, *next;
        TWLIST_FOR_DATA(EventList, temp, next)
        {
            EventNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, EventNode_t, head);
            if (node->eventId == eventArray[i])
            {
                newNode = node;
                break;
            }
        }
        if (NULL == newNode)
        {
            newNode = HAL_MALLOC(sizeof(EventNode_t));
            if (NULL == newNode)
            {
                continue;
            }
            HAL_MEMSET(newNode, 0, sizeof(EventNode_t));
            Quos_twllHeadAdd(&EventList, &newNode->head);
            newNode->eventId = eventArray[i];
        }

        quint32_t cbId;
        for (cbId = 0; cbId < newNode->cbArraySize; cbId++)
        {
            if (newNode->cbArray[cbId] == eventCb)
            {
                break;
            }
        }
        if (cbId == newNode->cbArraySize)
        {
            EventCB_f *newCbArray = HAL_MALLOC(sizeof(EventCB_f) * (newNode->cbArraySize + 1));
            if (newCbArray)
            {
                HAL_MEMCPY(newCbArray, newNode->cbArray, newNode->cbArraySize * sizeof(EventCB_f));
                newCbArray[newNode->cbArraySize++] = eventCb;
                HAL_FREE(newNode->cbArray);
                newNode->cbArray = newCbArray;
                Quos_logPrintf(LSDK_EVENT, LL_DBG, "add event[%d] Cb[%p] ok", newNode->eventId, eventCb);
            }
        }
    }
}
/**************************************************************************
** 功能	@brief : 事件分发处理
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void FUNCTION_ATTR_ROM quos_eventHandle(const void *arg, quint32_t argLen)
{
    UNUSED(argLen);
    EventArg_t *eventArg = (EventArg_t*)arg;
    Quos_logPrintf(LSDK_EVENT, LL_DBG, "event:%d", eventArg->event);
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA(EventList, temp, next)
    {
        EventNode_t *node = __GET_STRUCT_BY_ELEMENT(temp, EventNode_t, head);
        if (eventArg->event == node->eventId)
        {
            quint32_t i;
            for ( i= 0; i < node->cbArraySize; i++)
            {
                node->cbArray[i](eventArg->event,eventArg->arg);
            }
            break;
        }
    }
}
/**************************************************************************
** 功能	@brief : 事件post
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_eventPost(qint32_t event,void *arg)
{
    EventArg_t eventArg;
    eventArg.event = event;
    eventArg.arg = arg;
    Quos_logPrintf(LSDK_EVENT, LL_DBG, "event:%d", event);
    return Quos_signalSet(&eventArg, sizeof(eventArg), quos_eventHandle);
}
#endif