/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "Qhal_driver.h"
#include "Ql_iotApi.h"
extern void CmdTestInit(void);
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
int FUNCTION_ATTR_ROM main(void)
{
    CmdTestInit();
    Ql_iotInit();
    while (1)
    {
        sleep(1000);
    }
}