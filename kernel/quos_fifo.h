#ifndef __FIFO_H__
#define __FIFO_H__

#include "quos_config.h"
#if (SDK_ENABLE_FIFO == 1)

typedef struct
{
    quint32_t head;
    quint32_t tail;
    quint32_t bufSize;
    quint8_t *fifoBuf;
} FifoDat_T;
FifoDat_T Quos_fifoInit(quint8_t *buf, quint32_t size);
quint32_t Quos_fifoUsedLen(FifoDat_T *fifo);
quint32_t Quos_fifoFreeLen(FifoDat_T *fifo);
void Quos_fifoClear(FifoDat_T *fifo);
void Quos_fifoDelete(FifoDat_T *fifo);
quint32_t Quos_fifoWrite(FifoDat_T *fifo, void *buf, quint32_t len);
qint8_t Quos_fifoCheckOffset(FifoDat_T *fifo, void *buf, quint32_t offset);
quint32_t Quos_fifoReadByte_noDel(FifoDat_T *fifo, void *buf, quint32_t offset, quint32_t len);
quint32_t Quos_fifoReadByte_del(FifoDat_T *fifo, void *buf, quint32_t len);
void Quos_fifoDeleteNByte(FifoDat_T *fifo, quint32_t len);
qint32_t Quos_fifoStrstr(FifoDat_T *fifo, quint32_t offset, void *buf, quint32_t bufLen);

#endif
#endif
