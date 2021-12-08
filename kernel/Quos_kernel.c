/*************************************************************************
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : 
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
***************************************************************************/
#include "Quos_kernel.h"
#include "Qhal_driver.h"

static qbool QuosKernelIsResume = FALSE;
/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_kernelInit(void)
{
    HAL_LOCK_INIT(lockLogId);
    Quos_swTimerInit();
    Quos_sysTickInit();
    Quos_socketInit();
    Quos_signalInit();
}
/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_kernelResume(void)
{
    Qhal_KernelResume();
    QuosKernelIsResume = TRUE;
}
/**************************************************************************
** ����	@brief : 
** ����	@param : 
** ���	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_kernelTask(void)
{
    QuosKernelIsResume = FALSE;
    quint32_t swTimerIdle = Quos_swTimerTask();
    qbool sigRet = Quos_signalTask();
    quint32_t sockIdle = Quos_socketTask();

    swTimerIdle = sigRet ? 0 : (swTimerIdle < sockIdle ? swTimerIdle : sockIdle);
    return QuosKernelIsResume ? 0 : swTimerIdle;
}
