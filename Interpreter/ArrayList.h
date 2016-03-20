/*
 * ArrayList.h
 *
 *  Created on: Jan 20, 2016
 *      Author: Jeff
 */

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

typedef struct _ArrayList ArrayList;
typedef struct _Object Object;

#include "shared.h"

// public functions:
ArrayList *array_list_create();
void array_list_delete(ArrayList*);

void array_list_clear(ArrayList* const);

void array_list_print(const ArrayList * const);

void array_list_push(ArrayList* const, Object);
Object array_list_pop(ArrayList* const);

void array_list_enqueue(ArrayList* const, Object);
Object array_list_dequeue(ArrayList* const);

void array_list_insert(ArrayList* const, int, Object);
Object array_list_remove(ArrayList* const, int);

void array_list_set(ArrayList* const, int, Object);
Object array_list_get(ArrayList* const, int);

int array_list_size(const ArrayList* const);
int array_list_is_empty(const ArrayList* const);

#endif /* ARRAYLIST_H_ */
