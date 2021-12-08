/*************************************************************************
**		    源码未经检录,使用需谨慎
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 信号量管理
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_signal.h"
#include "Quos_kernel.h"

#if (SDK_ENABLE_SIGNAL == 1)
typedef struct
{
    TWLLHead_T head;
    SignalCB_f eventCB;
    quint32_t argLen;
    quint8_t arg[1];
} SignalInfoNode_t;

static TWLLHead_T *SignalList = NULL;
HAL_LOCK_DEF(static,lockId)

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_signalInit(void)
{
    HAL_LOCK_INIT(lockId);
}
/**************************************************************************
** 功能	@brief : 发起信号
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_signalSet(void *arg, quint32_t len, SignalCB_f eventCB)
{
    if (NULL == eventCB)
    {
        return FALSE;
    }
    SignalInfoNode_t *node = HAL_MALLOC(__GET_POS_ELEMENT(SignalInfoNode_t, arg) + len);
    if (NULL == node)
    {
        Quos_logPrintf(LSDK_SIG, LL_ERR, "signal node malloc fail");
        return FALSE;
    }
    node->eventCB = eventCB;
    HAL_MEMCPY(node->arg, arg, len);
    node->argLen = len;
    HAL_LOCK(lockId);
    Quos_twllHeadAdd(&SignalList, &node->head);
    HAL_UNLOCK(lockId);
    Quos_kernelResume();
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 处理信号
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_RAM Quos_signalTask(void)
{
    if (NULL == SignalList)
    {
        Quos_logPrintf(LSDK_SIG, LL_DBG,"is empty");
        return FALSE;
    }
    HAL_LOCK(lockId);
    SignalInfoNode_t *node = __GET_STRUCT_BY_ELEMENT(SignalList, SignalInfoNode_t, head);
    Quos_twllHeadDelete(&SignalList, SignalList);
    HAL_UNLOCK(lockId);
    Quos_logHexDump(LSDK_SIG, LL_DUMP, "signal get", node->arg, node->argLen);
    Quos_logPrintf(LSDK_SIG, LL_DBG, "exec cb[%p]",node->eventCB);
    node->eventCB(node->arg, node->argLen);
    Quos_logPrintf(LSDK_SIG, LL_DBG, "exec cb[%p] finish",node->eventCB);
    HAL_FREE(node);
    return TRUE;
}
#endif