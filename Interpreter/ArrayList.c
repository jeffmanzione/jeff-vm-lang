/*
 * ArrayList.c
 *
 *  Created on: Jan 20, 2016
 *      Author: Jeff
 */

#include "ArrayList.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

typedef struct _ArrayList {
  int current;
  int size;
  int increment_rate;
  Object *Objects;
} ArrayList;

typedef enum {
  RIFHT, LEFT
} Shift;

void initWithSize(ArrayList* const, int);
void initWithSizeAndIncRate(ArrayList* const, int, int);
// static (private) utility functions:

/* Abstracting the print method of the Object by delegating it to the Object itself (OOP-like feature) */
static void shift(ArrayList * const list, int index, int rooms, Shift dir);
static void wide(ArrayList* const);
static void widen(ArrayList * const, int);

static void ensure_capacity(ArrayList * const list, int total_size) {
  widen(list, total_size);
//  while (total_size >= list->size) {
//    printf("size=%d\n", list->current); fflush(stdout);
//    wide(list);
//  }
}

void arraryCopy(void *dest, int dIndex, const void* src, int sIndex, int len,
    int destLen, size_t size) {
  unsigned char *udest = (unsigned char*) dest;
  unsigned char *usrc = (unsigned char*) src;
  dIndex *= size;
  sIndex *= size;
  len *= size;
  destLen *= size;

  if (src != dest) {
    memcpy(&udest[dIndex], &usrc[sIndex], len);
  } else {
    if (dIndex > sIndex) {
      unsigned char *tmp = (unsigned char*) calloc(destLen, size);
      memcpy(tmp, &udest[dIndex], (destLen - dIndex));
      memcpy(&udest[dIndex], &usrc[sIndex], len);
      memcpy(&udest[dIndex + len], tmp, (destLen - dIndex));
      free(tmp);
    } else if (sIndex > dIndex) {
      memcpy(&udest[dIndex], &usrc[sIndex], (destLen - sIndex) + 1);
    } else
      return;
  }
}

ArrayList *array_list_create() {
  ArrayList *list = NEW(list, ArrayList)
  initWithSize(list, 100);
  return list;
}
void initWithSize(ArrayList * const list, int size) {
  initWithSizeAndIncRate(list, size, 50);
}

void initWithSizeAndIncRate(ArrayList * const list, int size, int rate) {
  list->size = size;
  list->increment_rate = rate;
  list->Objects = (Object*) calloc(sizeof(Object), list->size);
  list->current = -1;
}

void array_list_clear(ArrayList * const list) {
  while (list->current >= 0) {
    list->Objects[list->current].type = NONE;
    list->current--;
  }
}

void array_list_set(ArrayList * const list, int index, Object e) {
  if (index > list->current) {
    ensure_capacity(list, index);
    list->current = index;
  }
  list->Objects[index] = e;
}

Object array_list_get(ArrayList * const list, int index) {
  Object to_get;
  to_get.type = REFERENCE;
  if (index <= list->current) {
    to_get.ref = &list->Objects[index];
    return to_get;
  }
  return NONE_OBJECT;
}

void array_list_push(ArrayList* const list, Object e) {
  array_list_insert(list, 0, e);
  //array_print(list);
}

Object array_list_pop(ArrayList* const list) {
  return array_list_remove(list, 0);
}

void array_list_enqueue(ArrayList * const list, Object e) {
  if (++list->current < list->size) {
    list->Objects[list->current] = e;
  } else {
    wide(list);
    list->Objects[list->current] = e;
  }

  //array_print(list);
}

Object array_list_dequeue(ArrayList* const list) {
  return array_list_remove(list, list->current);
}

static void widen(ArrayList* const list, int sz) {
  //printf("A\n"); fflush(stdout);
  list->size = sz;
  //printf("B\n"); fflush(stdout);
  list->Objects = RENEW(list->Objects, list->size, Object)
  //printf("C\n"); fflush(stdout);
//  arraryCopy(newArr, 0, list->Objects, 0, list->current, list->size,
//      sizeof(Object));
  //printf("D\n"); fflush(stdout);
  //free(list->Objects);
  //printf("E\n"); fflush(stdout);
//list->Objects = newArr;
  //printf("F\n"); fflush(stdout);
}

static void wide(ArrayList* const list) {
  list->size += list->increment_rate;
  list->Objects = RENEW(list->Objects, list->size, Object)
//  ;
//  arraryCopy(newArr, 0, list->Objects, 0, list->current, list->size,
//      sizeof(Object));
//  free(list->Objects);
//  list->Objects = newArr;
}

void array_list_insert(ArrayList * const list, int index, Object e) {
//  printf("array_insert(list,%d,%d)\n", index, (int) e.int_value);
//  printf("\tlist->current = %d\n", list->current);
//  printf("\tlist->size = %d\n", list->size);

  if (index >= list->size) {
    ensure_capacity(list, index);
    list->current = index - 1;
  }

  list->current++;
  if (index < list->current) {
    shift(list, index, 1, RIFHT);
  }

  list->Objects[index] = e;

//  printf("\tlist->current = %d\n", list->current);
//  printf("\tlist->size = %d\n", list->size);

}

int array_list_is_empty(const ArrayList * const list) {
  return list->current == -1;
}

int array_list_size(const ArrayList* const list) {
  return list->current + 1;
}

Object array_list_remove(ArrayList * const list, int index) {
  Object to_remove;
  to_remove.type = REFERENCE;

  if (list->current >= index) {
    to_remove.ref = &list->Objects[index];
    if (index != list->current) {
      shift(list, index, 1, LEFT);
    }
    list->current--;

    return to_remove;
  }
  return NONE_OBJECT;
}

void array_list_print(const ArrayList * const list) {
  int i;
  for (i = 0; i <= list->current; i++) {
    Object e = list->Objects[i];
    object_print(e, stdout);
    printf(",");
  }
  printf("\n");
}

void array_list_delete(ArrayList *list) {
  NULL_CHECK(list, "array_delete(null)!")
  free(list->Objects);
  free(list);
}

static void shift(ArrayList * const list, int index, int rooms, Shift dir) {
  if (dir == RIFHT) {
    arraryCopy(list->Objects, index + 1, list->Objects, index, rooms,
        list->current, sizeof(Object));
  } else //SHIFT
  {
    arraryCopy(list->Objects, index, list->Objects, index + 1, rooms,
        list->current + 1, sizeof(Object));
  }
}
