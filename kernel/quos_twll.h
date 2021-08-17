#ifndef __QUOS_TWLL_H__
#define __QUOS_TWLL_H__
#include "quos_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __TWLLHead
{
    struct __TWLLHead *prev;
    struct __TWLLHead *next;
}TWLLHead_T;
/* ¶Ô½ÚµãÂÖÑ¯ */
#define TWLIST_FOR_DATA(LISTHEAD,LISTTEMP,NEXTLIST)    \
    for((LISTTEMP) = (LISTHEAD),(NEXTLIST) = (LISTHEAD)?(LISTHEAD)->next:NULL;   \
        NULL != (LISTTEMP);   \
        (LISTTEMP) = (NEXTLIST),(NEXTLIST) = (NEXTLIST)?(NEXTLIST)->next:NULL) 

void Quos_twllHeadAdd(TWLLHead_T **twList,TWLLHead_T *twNode);
void Quos_twllHeadAddFirst(TWLLHead_T **list, TWLLHead_T *node);
void Quos_twllHeadDelete(TWLLHead_T **twList,TWLLHead_T *twNode);
qbool Quos_twllHeadInsertFront(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node);
qbool Quos_twllHeadInsertBehind(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node);
TWLLHead_T *Quos_twllHeadFineNodeByDataCmp(TWLLHead_T *list,void* dat,quint16_t offset,quint16_t len);
TWLLHead_T *Quos_twllHeadFineNodeByNodeId(TWLLHead_T *list, quint32_t nodeId);
quint32_t Quos_twllHeadGetNodeCount(TWLLHead_T *list);
#ifdef __cplusplus
}
#endif

#endif
