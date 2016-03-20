/*
 * queue.h
 *
 *  Created on: Jan 6, 2016
 *      Author: Jeff
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>

#include "shared.h"

typedef struct _Queue Queue;
typedef struct _QueueElement QueueElement;

typedef struct _QueueElement {
  QueueElement *next;
  void *value;
} QueueElement;

typedef struct _Queue {
  QueueElement *head;
  QueueElement *tail;
  size_t size;
} Queue;

void queue_init(Queue *queue);
void queue_shallow_delete(Queue *queue);
void queue_deep_delete(Queue *queue, Deleter del);
void *queue_peek(const Queue *queue);
void *queue_last(const Queue *queue);
void *queue_remove(Queue *queue);
void queue_add(Queue *queue, void *elt);
void queue_add_front(Queue *queue, void *elt);

#endif /* QUEUE_H_ */
