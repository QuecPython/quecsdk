#ifndef __QHAL_DEV_H__
#define __QHAL_DEV_H__
#include "Qhal_types.h"
typedef enum
{
    DEV_RESTART_FACTORY = 0, /* ³ö³§×´Ì¬ */
    DEV_RESTART_SEGMENTFAULT,
    DEV_RESTART_NORMAL,
    DEV_RESTART_OTA,
    DEV_RESTART_WDT,
    DEV_RESTART_NET_EXCP,
    DEV_RESTART_HW_FAULT,
} DevRestartReason_e;

#define DEV_RESTART_REASON_STRING(X) \
    ( \
    (X == DEV_RESTART_FACTORY) ? "DEV_RESTART_FACTORY" : \
    (X == DEV_RESTART_SEGMENTFAULT) ? "DEV_RESTART_SEGMENTFAULT" : \
    (X == DEV_RESTART_NORMAL) ? "DEV_RESTART_NORMAL" : \
    (X == DEV_RESTART_OTA) ? "DEV_RESTART_OTA" : \
    (X == DEV_RESTART_WDT) ? "DEV_RESTART_WDT" : \
    (X == DEV_RESTART_NET_EXCP) ? "DEV_RESTART_NET_EXCP" : \
    (X == DEV_RESTART_HW_FAULT) ? "DEV_RESTART_HW_FAULT" : \
    "Unknown")
    
void Qhal_devFeeddog(void);
qbool Qhal_rtcInit(void);
void Qhal_rtcSet(quint32_t timestamp, quint16_t ms);
void Qhal_rtcGet(quint32_t *sec, quint32_t *ms);
char *Qhal_softversionGet(void);
char *Qhal_logHeadString(void);
void Qhal_devRestart(void);
qbool Qhal_beforeMain(void);
char *Qhal_devUuidGet(void);
quint32_t Qhal_randomGet(void);
#ifdef AT_ENABLE_QUEC
void Qhal_urcReport(const quint8_t *data, quint32_t len);
#endif
quint32_t Qhal_devOtaNotify(const char *filename, quint32_t fileSize);
void Qhal_netOpen(quint32_t *timeout);
void Qhal_netClose(void);
#endif