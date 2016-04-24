/*
 * namespace.h
 *
 *  Created on: Apr 16, 2016
 *      Author: Jeff
 */

#ifndef NAMESPACE_H_
#define NAMESPACE_H_

#include <stddef.h>
#include <stdint.h>

#include "class.h"
#include "hashtable.h"
#include "queue.h"
#include "shared.h"

#define DEFAULT NS_HT_SZ     128
#define DEFAULT_CHILD_ARR_SZ 8

typedef struct _Namespace Namespace;

typedef struct _Namespace {
  char name[ID_SZ + 1];
  struct {
    Instruction *ins;
    size_t ins_len;
  };
  struct {
    Object *classes;
    Queue classes_q;
    Hashtable *classes_ht;
    size_t class_array_capacity;
    size_t classes_len;
  };
  struct {
    Namespace **children;
    size_t children_array_capacity;
    size_t children_len;
  };
} Namespace;

Namespace *namespace_create(const char name[], Instruction * const ins,
    size_t ins_len);
void namespace_add_child(Namespace * const space, Namespace * const child);
void namespace_add_class(Namespace * const space, Class * const class);
Object namespace_search(Namespace * const space, Queue * const exts);
void namespace_free(Namespace * const space);


Namespace *load();


#endif /* NAMESPACE_H_ */
