#ifndef LIB_KERNEL_LIST_H_
#define LIB_KERNEL_LIST_H_

#include "kernel/global.h"
#include "lib/stdint.h"

#define offset(structType, member)
#define elem2entry(structType, structMemberName, elemPtr) (structType*)((int)elemPtr - offset(structType, structMemberName))

typedef struct _ListElem {
    struct _ListElem* prev;
    struct _LISTElem* next;
} ListElem;

typedef struct _List {
    ListElem head;
    ListElem tail;
} List;

typedef bool (function)(ListElem*, int arg);

void ListInit(List*);
void ListInsertBefore(ListElem* before, ListElem* elem);
void ListPush(List* pList, ListElem* elem);
void ListIterate(List* pList);
void ListAppend(List* pList, ListElem* elem);
void ListRemove(ListElem* pElem);
ListElem* ListPop(List* pList);
bool ListEmpty(List* pList);
uint32 ListLen(List* pList);
ListElem* ListTraversal(List* pList, function func, int arg);
bool ElemFind(List* pList, ListElem* objElem);

#endif // LIB_KERNEL_LIST_H_