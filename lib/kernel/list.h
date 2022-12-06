#ifndef LIB_KERNEL_LIST_H_
#define LIB_KERNEL_LIST_H_

#include "kernel/global.h"
#include "lib/stdint.h"

#define Offset(structType, member) (int)(&((structType*)0)->member)
#define Elem2Entry(structType, structMemberName, pElemPtr) (structType*)((int)pElemPtr - Offset(structType, structMemberName))

/*
* 节点
*/
typedef struct _ListElem {
    struct _ListElem* prev;
    struct _ListElem* next;
} ListElem;

/*
* 链表
*/
typedef struct _List {
    ListElem head;
    ListElem tail;
} List;

typedef bool (function)(ListElem*, int arg);

void ListInit(List*);
void ListInsertBefore(ListElem* before, ListElem* pElem);
void ListPush(List* pList, ListElem* pElem);
void ListIterate(List* pList);
void ListAppend(List* pList, ListElem* pElem);
void ListRemove(ListElem* pElem);
ListElem* ListPop(List* pList);
bool ListEmpty(List* pList);
uint32 ListLen(List* pList);
ListElem* ListTraversal(List* pList, function func, int arg);
bool ElemFind(List* pList, ListElem* objElem);


void ElemPrint(ListElem* pElem, const char* info);
void ListPrint(List* pList, const char* info);

#endif // LIB_KERNEL_LIST_H_