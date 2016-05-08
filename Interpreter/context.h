/*
 * context.h
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "hashtable.h"
#include "shared.h"
#include "object.h"

#define TABLE_SZ 128

typedef struct _Context {
  int *ip;
  int new_ip;
  Context *parent, *prev;
  Hashtable *table;
//Object  *heap;
} Context;

Context *context_open_with_parent(Context *context, Context *parent);
Context *context_open(Context *context);
Context *context_close(Context *context);
Object *context_lookup_unchecked(const char id[], Context *context);
Object *context_lookup(const char id[], Context *context);
void context_set(const char id[], Object val, Context *context);

#endif /* CONTEXT_H_ */
