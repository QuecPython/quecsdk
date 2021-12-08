#ifndef __QHAL_PROPERTY_H__
#define __QHAL_PROPERTY_H__
#include "Qhal_types.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define QIOT_LOC_SUPPORT_AUTO "AUTO"
#define QIOT_LOC_SUPPORT_LBS "LBS"
    typedef struct
    {
        quint8_t cellLevel;
        double cellVoltage;
        quint32_t flashFree;
        char *modelType;
        quint8_t pdpCxtIdMin;
        quint8_t pdpCxtIdMax;
    } Qhal_propertyDev_t;

    typedef struct
    {
        quint32_t cellid;
        quint16_t mcc;
        quint16_t mnc;
        quint16_t lac;
        qint32_t rsrp;
        qint32_t rsrq;
        qint32_t snr;
        qint32_t rssi;
    } Qhal_propertyNet_t;

    typedef struct
    {
        char iccid[20 + 1];
        char phoneid[20 + 1];
        char imsi[15 + 1];
    } Qhal_propertySim_t;

    qbool Qhal_propertyDevGet(Qhal_propertyDev_t *dInfo);
    qbool Qhal_propertyNetGet(Qhal_propertyNet_t *nMasterInfo, Qhal_propertyNet_t (*nNeibhInfo)[], quint32_t maxCnt, quint32_t *size);
    qbool Qhal_propertySimGet(Qhal_propertySim_t *sInfo);
    quint32_t Qhal_propertyLocSupList(char **words, quint32_t maxSize);
    qbool Qhal_propertyGnssRawDataRead(void **ttlv, const char *title);

#ifdef __cplusplus
}
#endif
#endif