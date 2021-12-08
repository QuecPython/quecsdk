#ifndef __QHAL_TYPES_H__
#define __QHAL_TYPES_H__
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include "helios_os.h"
#include "helios_debug.h"
#include <math.h>

extern void qhal_mutex_create(Helios_Mutex_t *mutex);
extern unsigned char Qhal_sleepExitDbg(const char *func, int line);
#define QHAL_APP_TASK_PRIORITY 90

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif
/*#define  NULL           ((void*)0)*/
#define quint8_t unsigned char
#define qint8_t char
#define quint16_t unsigned short
#define qint16_t short
#define quint32_t unsigned int
#define qint32_t int
#define quint64_t unsigned long long int 
#define qint64_t long long int
#define pointer_t unsigned long int
typedef unsigned char qbool;
#define FALSE 0
#define TRUE 1

#define PRINTF_LU    "%lu"
#define PRINTF_LD    "%ld"
#define PRINTF_FD    "%ld"

#define FUNCTION_ATTR_ROM
#define FUNCTION_ATTR_RAM


#define QUEC_ENABLE_QTH_OTA 1
#define HAL_MEMCPY(a,b,l)        memcpy((quint8_t*)(a),(quint8_t*)(b),l)
#define HAL_MEMCMP(a,b,l)        memcmp((quint8_t*)(a),(quint8_t*)(b),l)
#define HAL_MEMSET(a,b,l)        memset((void *)a, (int)b, (size_t)l)
#define HAL_MEMMOVE              memmove
#define HAL_SPRINTF              sprintf
#define HAL_SNPRINTF             snprintf
#define HAL_STRCHR(X,Y)          strchr(X,Y)
#define HAL_STRCPY(a,b)          strcpy((char*)a,(char*)b)
#define HAL_STRNCPY(a,b,l)       strncpy((char*)a,(char*)b,l)
#define HAL_STRCMP(a,b)          strcmp((char*)a,(char*)b)
#define HAL_STRNCMP(a,b,l)       strncmp((char*)a,(char*)b,l)
#define HAL_STRSTR(a,b)          strstr((char*)a,(char*)b)
#define HAL_STRLEN(a)            ((NULL==a)?0:strlen((char*)a))
#define HAL_STRCASECMP(a,b)      strcasecmp((char*)a,(char*)b)
#define HAL_STRNCASECMP(a,b,l)   strncasecmp((char*)a,(char*)b,l)
#define HAL_PRINTF(format,...)   Helios_Debug_Output(format,##__VA_ARGS__)
#define HAL_SSCANF               sscanf
#define HAL_ATOI				 atoi
#define HAL_STRTOL               strtol 
#define HAL_STRTOF               strtof    
#define HAL_STRTOD               strtod
#define HAL_STRTOUL              strtoul
#define HAL_LOCK_DEF(S,X)       S Helios_Mutex_t X;
#define HAL_LOCK_INIT(X)        qhal_mutex_create(&X)  
#define HAL_LOCK(X)             Helios_Mutex_Lock(X, 0xFFFFFFFF)
#define HAL_TRYLOCK(X)          (Helios_Mutex_Lock(X, 0) == 0)
#define HAL_UNLOCK(X)           Helios_Mutex_Unlock(X)
#define HAL_LOCK_INITDATA       
#define HAL_MALLOC(LEN)         qhal_malloc(LEN,__FUNCTION__,__LINE__)
#define HAL_CALLOC(X,Y)         qhal_malloc((X)*(Y),__FUNCTION__,__LINE__)
#define HAL_FREE(ADDR)          qhal_free(ADDR,__FUNCTION__,__LINE__)
#define HAL_STRDUP(X)           qhal_strdup(X,__FUNCTION__,__LINE__)
#define HAL_MEMDUP(X,Y)         qhal_memdup(X,Y,__FUNCTION__,__LINE__)

#define Qhal_sleepExit()        Qhal_sleepExitDbg(__FUNCTION__,__LINE__)

extern void *qhal_malloc(quint32_t len,const char *funName,qint32_t line);
extern void qhal_free(void *addr,const char *funName,qint32_t line);
extern void *qhal_memdup(const void *buf, quint32_t len, const char *funName, qint32_t line);
extern char *qhal_strdup(const char *str, const char *funName, qint32_t line);

/* LOG模块 PRINTF等级配置 */
#define HAL_DEV                     LL_DBG
#define HAL_FLASH                   LL_DBG              /* 开启日志写FLASH时该项一定要置为OFF */
#define HAL_TCP                     LL_DBG
#define HAL_TLS                     LL_DBG
#define HAL_DTLS                    LL_DBG
#define HAL_UDP                     LL_DBG
#define HAL_UART                    LL_DBG
#define HAL_SOCK                    LL_DBG
#define HAL_PROP                    LL_DBG
#endif