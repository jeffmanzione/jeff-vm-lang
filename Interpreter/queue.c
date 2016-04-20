/*
 * queue.c
 *
 *  Created on: Jan 6, 2016
 *      Author: Jeff
 */

#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

#include "shared.h"
#include "tokenizer.h"

void queue_init(Queue *queue) {
  queue->size = 0;
  queue->head = NULL;
  queue->tail = NULL;
}

void queue_deep_delete(Queue *queue, Deleter del) {
  while (queue->size > 0) {
    del(queue_remove(queue));
  }
}

void queue_shallow_delete(Queue *queue) {
  while (queue->size > 0) {
    queue_remove(queue);
  }
}

void queue_iterate(Queue *queue, Q_Action act) {
  NULL_CHECK(queue, "Queue was null!")
  QueueElement *elt = queue->head;

  while (NULL != elt) {
    act(elt->value);
    elt = elt->next;
  }
}

void *queue_peek(const Queue *queue) {
  if (NULL == queue->head) {
    return NULL;
  }

  //printf("QUEUE PEEK: %s\n", ((Token *) queue->head->value)->text);

  return queue->head->value;
}

void *queue_last(const Queue *queue) {
  if (NULL == queue->tail) {
    return NULL;
  }

  return queue->tail->value;
}

void *queue_remove(Queue *queue) {
  void *to_remove;
  if (NULL == queue->head) {
    return NULL;
  }

  to_remove = queue->head->value;

//  printf("QUEUE REMOVE: %s\n", ((Token *) to_remove)->text);
//  fflush(stdout);

  queue->head = queue->head->next;

  queue->size--;

  return to_remove;
}

void queue_add(Queue *queue, void *elt) {
  QueueElement *new_elt;
  if (NULL == queue) {
    return;
  }

  new_elt = NEW(new_elt, QueueElement)
  new_elt->value = elt;
  new_elt->next = NULL;

  if (NULL == queue->tail) {
    queue->head = queue->tail = new_elt;
    queue->size = 1;
  } else {
    queue->tail->next = new_elt;
    queue->tail = new_elt;
    queue->size++;
  }
  //printf("QUEUE SIZE = %d\n", queue->size);
}

void queue_add_front(Queue *queue, void *elt) {
  QueueElement *new_elt;
  if (NULL == queue) {
    return;
  }

  new_elt = NEW(new_elt, QueueElement)
  new_elt->value = elt;
  new_elt->next = queue->head;

  if (NULL == queue->tail) {
    queue->head = queue->tail = new_elt;
    queue->size = 1;
  } else {
    queue->head = new_elt;
    queue->size++;
  }
}
