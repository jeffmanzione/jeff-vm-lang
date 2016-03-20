/*
 * array.h
 *
 *  Created on: Jan 20, 2016
 *      Author: Jeff
 */

#ifndef ARRAY_H_
#define ARRAY_H_


#include "shared.h"

Array *array_create();
void array_delete(Array *);

void array_clear(Array * const);

void array_print(const Array * const);

void array_push(Array * const, Object);
Object array_pop(Array * const);

void array_enqueue(Array * const, Object);
Object array_dequeue(Array * const);

void array_insert(Array * const, int, Object);
Object array_remove(Array * const, int);

void array_set(Array * const, int, Object);
Object array_get(Array * const, int);

int array_size(const Array * const);
int array_is_empty(const Array * const);
#endif /* ARRAY_H_ */
