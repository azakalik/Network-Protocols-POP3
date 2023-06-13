#ifndef QUEUEADT_H
#define QUEUEADT_H

#include <stdlib.h>
#include <stdio.h>

typedef int elementType;

typedef struct queueCDT * queueADT;

// Genera una nueva cola
queueADT newQueue(void);

// Agrega un elemento al final de la cola
void queue(queueADT q, elementType n);

// Remueve un elemento del principio de la cola y lo deja en out
void dequeue(queueADT q, elementType * out);

// retorna si la queue esta vacia
int isQueueEmpty(queueADT q);

// libera la estructura
void freeQueue(queueADT q);

/* Funciones para iterar */

void toBegin(queueADT q);

int hasNext(queueADT q);

elementType next(queueADT q);

#endif //QUEUEADT_H