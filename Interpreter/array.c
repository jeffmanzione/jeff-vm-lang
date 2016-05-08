/*
 * array.c
 *
 *  Created on: Jan 30, 2016
 *      Author: Jeff
 */

#include "array.h"

#include "ArrayList.h"

Array *array_create() {
  return (Array *) array_list_create();
}

void array_delete(Array *list) {
  array_list_delete((ArrayList * const ) list);
}

void array_clear(Array * const list) {
  array_list_clear((ArrayList * const ) list);
}

void array_print(const Array * const list) {
  array_list_print((ArrayList * const ) list);
}

void array_push(Array * const list, Object obj) {
  array_list_push((ArrayList * const ) list, obj);
}

Object array_pop(Array * const list) {
  return array_list_pop((ArrayList * const ) list);
}

void array_enqueue(Array * const list, Object obj) {
  array_list_enqueue((ArrayList * const ) list, obj);
}

Object array_dequeue(Array * const list) {
  return array_list_dequeue((ArrayList * const ) list);
}

void array_insert(Array * const list, int index, Object obj) {
  array_list_insert((ArrayList * const ) list, index, obj);
}

Object array_remove(Array * const list, int index) {
  return array_list_remove((ArrayList * const ) list, index);
}

void array_set(Array * const list, int index, Object obj) {
  array_list_set((ArrayList * const ) list, index, obj);
}

Object array_get(Array * const list, int index) {
  return array_list_get((ArrayList * const ) list, index);
}

int array_size(const Array * const list) {
  return array_list_size((ArrayList * const ) list);
}

int array_is_empty(const Array * const list) {
  return array_list_is_empty((ArrayList * const ) list);
}

bool array_is_string(const Array *const list) {
  return array_list_is_string(list);
}
