#include "queueADT.h"

typedef struct node {
    elementType elem;
    struct node * tail;
} tNode;

typedef tNode * tList;

typedef struct queueCDT {
    tList first;
    tList last;
    tList iter;
} queueCDT;

queueADT newQueue(void) {
    return calloc(1, sizeof(queueCDT));
}

static tList newNode(elementType elem, tList next) {
    tList aux = malloc(sizeof(tNode));
    aux->elem = elem;
    aux->tail = next;
    return aux;
}

void queue(queueADT q, elementType n) {
    tList node = newNode(n, NULL);
    if(isEmpty(q)) {
        q->first = node;
        q->last = q->first;
        return;
    }
    q->last->tail = node;
    q->last = q->last->tail;
}

void dequeue(queueADT q, elementType * out) {
    if(isEmpty(q)) {
        perror("La cola ya esta vacia");
        exit(1);
    }
    *out = q->first->elem;
    tList aux = q->first;
    q->first = aux->tail;
    free(aux);
}

int isEmpty(queueADT q) {
    return q->first == NULL;
}

static void freeList(tList first) {
    if(first == NULL) return;
    freeList(first->tail);
    free(first);
}

void freeQueue(queueADT q) {
    freeList(q->first);
    free(q);
}

void toBegin(queueADT q) {
    q->iter = q->first;
}

int hasNext(queueADT q) {
    return q->iter != NULL;
}

elementType next(queueADT q) {
    if(!hasNext(q)) {
        perror("No habia siguiente, utilice \"hasNext()\" para verificar");
        exit(1);
    }
    elementType aux = q->iter->elem;
    q->iter = q->iter->tail;
    return aux;
}