#include "list.h"

#include "kernel/interrupt.h"

void ListInit(List* list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}


void ListInsertBefore(ListElem*before, ListElem* elem) {
    IntrStatus oldStatus = IntrDisable();

    before->prev->next = elem;

    elem->prev->next = elem;

    elem->prev = before->prev;
    elem->next = before;

    before->prev = elem;

    IntrSetStatus(oldStatus);
}

void ListPush(List* pListm, ListElem* elem) {
    ListInsertBefore(&pListm->tail, elem);
}


void ListAppend(List* pList, ListElem* elem) {
    ListInsertBefore(&pList->tail, elem);
}


void ListRemove(ListElem* pElem) {
    IntrStatus oldStatus = IntrDisable();

    pElem->prev->next = pElem->next;
    pElem->next->prev = pElem->prev;

    IntrSetStatus(oldStatus);
}

ListElem* ListPop(List* pList) {
    ListElem* elem = pList->head.next;
    ListRemove(elem);
    return elem;
}


bool ElemFind(List* pList, ListElem* objElem) {
    ListElem* elem = pList->head.next;
    while (elem != & pList->tail) {
        if (elem == objElem) {
            return true;
        }
        elem = elem->next;
    }
    return false;
}

ListElem* ListTraversal(List* pList, function func, int arg) {
    ListElem* elem = pList->head.next;
    if (ListEmpty(pList)) {
        return NULL;
    }

    while (elem != &pList->tail) {
        if (func(elem, arg)) {
            return elem;
        }
    }
    return NULL;
}

uint32 ListLen(List* pList) {
    ListElem* elem = pList->head.next;
    uint32 length = 0;
    while (elem != &pList->tail) {
        length++;
        elem = elem->next;
    }
    return length;
}

bool ListEmpty(List* pList) {
    return (pList->head.next == & pList->tail ? true : false);
}
