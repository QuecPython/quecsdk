/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "Quos_kernel.h"
#include "Qhal_driver.h"

static qbool QuosKernelIsResume = FALSE;
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
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
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_kernelResume(void)
{
    Qhal_KernelResume();
    QuosKernelIsResume = TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
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
