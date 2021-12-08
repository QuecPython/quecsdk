/*************************************************************************
**		    Դ��δ����¼,ʹ�������
** ������ @author  : �⽡�� JCWu
** �汾   @version : V1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   : ʵ��˫���ѭ������������
** Ӳ��   @hardware���κ�ANSI-Cƽ̨
** ����   @other   ��
***************************************************************************/
#include "quos_twll.h"
#if (SDK_ENABLE_TWLL ==1)
#include "Quos_kernel.h"

/**************************************************************************
** ����	@brief : ������β�����ӽڵ�
** ����	@param : list �������׽ڵ��ַ node �������ڵ�
** ���	@retval: ���ӵĽڵ�ָ��
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_twllHeadAdd(TWLLHead_T **list, TWLLHead_T *node)
{
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == node)
        {
            return;
        }
    }
    node->next = NULL;
    if (*list == NULL)
    {
        *list = node;
    }
    else
    {
        node->prev = (*list)->prev;
        (*list)->prev->next = node;
    }
    (*list)->prev = node;
}
/**************************************************************************
** ����	@brief : ������ͷ�����ӽڵ�
** ����	@param : list �������׽ڵ��ַ node �������ڵ�
** ���	@retval: ���ӵĽڵ�ָ��
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_twllHeadAddFirst(TWLLHead_T **list, TWLLHead_T *node)
{
    node->next = *list;
    if(NULL == *list)
    {
        node->prev = node; 
    }
    else
    {
        node->prev = (*list)->prev;
        (*list)->prev = node;
    }
    *list = node;
}
/**************************************************************************
** ����	@brief : ɾ��ָ���ڵ�
** ����	@param : list �������׽ڵ��ַ  node ��ɾ�������ڵ�
** ���	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_twllHeadDelete(TWLLHead_T **list, TWLLHead_T *node)
{
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == node)
        {
            if (node == *list)
            {
                if (node->next)
                {
                    node->next->prev = (*list)->prev;
                }
                *list = node->next;
            }
            else if (node->next)
            {
                node->next->prev = node->prev;
                node->prev->next = node->next;
            }
            else
            {
                (*list)->prev = node->prev;
                node->prev->next = NULL;
            }
            node->prev = NULL;
            node->next = NULL;
            return;
        }
    }
}
/**************************************************************************
** ����	@brief : ��ָ���ڵ�ǰ�����ڵ�
** ����	@param : list �������׽ڵ��ַ referNode�ο��ڵ� node �������ڵ�
** ���	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_twllHeadInsertFront(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node)
{
    Quos_twllHeadDelete(list, node);
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == referNode)
        {
            node->next = referNode;
            node->prev = referNode->prev;

            if (referNode == *list)
            {
                *list = node;
            }
            else
            {
                referNode->prev->next = node;
            }
            referNode->prev = node;
            return TRUE;
        }
    }
    return FALSE;
}
/**************************************************************************
** ����	@brief : ��ָ���ڵ�������ڵ�
** ����	@param : list �������׽ڵ��ַ referNode�ο��ڵ� node �������ڵ�
** ���	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_twllHeadInsertBehind(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node)
{
    Quos_twllHeadDelete(list, node);
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == referNode)
        {
            node->next = referNode->next;
            node->prev = referNode;
            if (referNode->next)
            {
                referNode->next->prev = node;
            }
            referNode->next = node;
            return TRUE;
        }
    }
    return FALSE;
}
/**************************************************************************
** ����	@brief : ����������N���ڵ�
** ����	@param : list �������׽ڵ��ַ,nodeId �ڵ��ŵ�һ���ڵ���0
** ���	@retval:
***************************************************************************/
TWLLHead_T FUNCTION_ATTR_ROM *Quos_twllHeadFineNodeByNodeId(TWLLHead_T *list, quint32_t nodeId)
{
    TWLLHead_T *tempNode = list;
    while (tempNode && nodeId)
    {
        nodeId--;
        tempNode = tempNode->next;
    }
    return tempNode;
}
/**************************************************************************
** ����	@brief : ��ȡָ�������ڵ���
** ����	@param : list �������׽ڵ��ַ
** ���	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_twllHeadGetNodeCount(TWLLHead_T *list)
{
    quint16_t nodeCnt = 0;
    TWLLHead_T *tempNode = list;
    while (tempNode)
    {
        nodeCnt++;
        tempNode = tempNode->next;
    }
    return nodeCnt;
}

#endif