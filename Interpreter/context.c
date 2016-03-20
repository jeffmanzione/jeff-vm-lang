/*
 * context.c
 *
 *  Created on: Jan 4, 2016
 *      Author: Jeff
 */

#include "context.h"

#include <stdlib.h>

Context *context_open(Context *context) {
  Context *new_context = NEW(new_context, Context)
  new_context->parent = context;
  new_context->table = create_hash_table(TABLE_SZ);
  if (NULL == context) {
    new_context->ip = NEW(new_context->ip, int)
    *(new_context->ip) = 0;
  } else {
    new_context->ip = context->ip;
  }
  return new_context;
}

Context *context_close(Context *context) {
  Context *parent_context = context->parent;
  free_table(context->table, object_delete);
  if (TRUE == context->new_ip) {
    free(context->ip);
  }
  free(context);
  return parent_context;
}

void context_set(const char id[], Object val, Context *context) {
  Object *old_val = context_lookup_unchecked(id, context);
  Object *new_val;

  if (NULL != old_val) {
    *old_val = val;
  } else {
    new_val = NEW(new_val, Object)
    *new_val = val;
    insert(context->table, id, new_val);
  }
}

Object *context_lookup_unchecked(const char id[], Context *context) {
  if (NULL == context)
    return NULL;

  Object *val = get(context->table, id);

  if (NULL == val && FALSE == context->new_ip) {
    return context_lookup_unchecked(id, context->parent);
  }

  return val;
}

Object *context_lookup(const char id[], Context *context) {
  //printf("ID: '%s'\n", id);
  Object *val = context_lookup_unchecked(id, context);
  if (!val) {
    return &NONE_OBJECT;
  }
  return val;
}