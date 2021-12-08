/*************************************************************************
**		    Դ��δ����¼,ʹ�������
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : �ź�������
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
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
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_signalInit(void)
{
    HAL_LOCK_INIT(lockId);
}
/**************************************************************************
** ����	@brief : �����ź�
** ����	@param : 
** ���	@retval: 
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
** ����	@brief : �����ź�
** ����	@param : 
** ���	@retval: 
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