#include "list.h"

#include "kernel/interrupt.h"
#include "lib/kernel/print.h"

/*
* 初始化链表
*/
void ListInit(List* list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

/*
* 将pElem插入到before前面
*/
void ListInsertBefore(ListElem* before, ListElem* pElem) {
    IntrStatus oldStatus = IntrDisable();

    before->prev->next = pElem;

    pElem->prev = before->prev;
    pElem->next = before;

    before->prev = pElem;

    IntrSetStatus(oldStatus);
}

/*
* 将pElem插入到链表头
*/
void ListPush(List* pList, ListElem* pElem) {
    ListInsertBefore(&pList->tail, pElem);
}

/*
* 将pElem插入到链表尾
*/
void ListAppend(List* pList, ListElem* pElem) {
    ListInsertBefore(&pList->tail, pElem);
}

/*
* 将pElem从链表摘除
*/
void ListRemove(ListElem* pElem) {
    IntrStatus oldStatus = IntrDisable();

    pElem->prev->next = pElem->next;
    pElem->next->prev = pElem->prev;

    IntrSetStatus(oldStatus);
}

/*
* 从链表中弹出头节点
*/
ListElem* ListPop(List* list) {
    ListElem* pElem = list->head.next;
    ListRemove(pElem);
    return pElem;
}

/*
* 判断pElem是否在链表中
*/
bool ElemFind(List* pList, ListElem* pElem) {
    ListElem* pElem_ = pList->head.next;
    while (pElem_ != &pList->tail) {
        if (pElem_ == pElem) {
            return true;
        }
        pElem_ = pElem_->next;
    }
    return false;
    
}

void ElemPrint(ListElem* pElem, const char* info) {
    IntrStatus oldStatus = IntrDisable();
    PutStr("elem."); PutStr(info); PutStr(" 0x"); PutInt(pElem); PutChar(' ');
    IntrSetStatus(oldStatus);
}

void ListPrint(List* pList, const char* info) {
    IntrStatus oldStatus = IntrDisable();
    PutStr("list."); PutStr(info); PutStr(" beg"); PutChar(' ');
    PutStr("head:0x"); PutInt(&pList->head); PutChar(' ');
    PutStr("tail:0x"); PutInt(&pList->tail); PutChar('\n');
    ListElem* pElem_ = pList->head.next;
    while (pElem_ != &pList->tail) {
        PutStr("elem:0x"); PutInt(pElem_); PutChar(' ');
        pElem_ = pElem_->next;
    }
    PutStr("list end"); PutChar('\n');
    IntrSetStatus(oldStatus);
}

/*
* 判断pElem是否在链表中
*/
ListElem* ListTraversal(List* pList, function func, int arg) {
    ListElem* pElem = pList->head.next;
    if (ListEmpty(pList)) {
        return NULL;
    }

    while (pElem != &pList->tail) {
        if (func(pElem, arg)) {
            return pElem;
        }
    }
    return NULL;
}

/*
* 获取链表长度
*/
uint32_t ListLen(List* pList) {
    ListElem* pElem = pList->head.next;
    uint32_t length = 0;
    while (pElem != &pList->tail) {
        length++;
        pElem = pElem->next;
    }
    return length;
}

/*
* 判断链表是否为空
*/
bool ListEmpty(List* pList) {
    return (pList->head.next == &pList->tail ? true : false);
}
