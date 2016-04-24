/*
 * namespace.c
 *
 *  Created on: Apr 16, 2016
 *      Author: Jeff
 */

#include "namespace.h"

Namespace *namespace_create(const char name[], Instruction * const ins,
    size_t ins_len) {
  Namespace *space = NEW(space, Namespace)

  /* Name */{
    strncpy(space->name, name, ID_SZ);
    space->name[ID_SZ] = '\0';
  }

  /*Instructions */{
    space->ins = ins;
    space->ins_len = ins_len;
  }

  /* Classes */{
    space->classes_ht = hashtable_create(DEFAULT_CLASS_ARRAY_SZ);
    space->class_array_capacity = DEFAULT_CLASS_ARRAY_SZ;
    space->classes = NEW_ARRAY(space->classes, DEFAULT_CLASS_ARRAY_SZ, Object)
    space->classes_len = 0;
    queue_init(&space->classes_q);
  }

  /* Children */{
    space->children = NEW_ARRAY(space->children, DEFAULT_CHILD_ARR_SZ,
        Namespace *)
    space->children_array_capacity = DEFAULT_CHILD_ARR_SZ;
    space->children_len = 0;
  }

  return space;
}

void namespace_add_child(Namespace * const space, Namespace * const child) {
  if (space->children_len == space->children_array_capacity) {
    space->children_array_capacity += DEFAULT_CHILD_ARR_SZ;
    space->children = RENEW(space->children, space->children_array_capacity,
        Namespace *)
  }
  space->children[space->children_len++] = child;
  hashtable_insert(space->classes_ht, child->name, child);
}

void namespace_add_class(Namespace * const space, Class * const class) {
  if (space->classes_len == space->class_array_capacity) {
    space->class_array_capacity += DEFAULT_CLASS_ARRAY_SZ;
    space->classes = RENEW(space->classes, space->class_array_capacity, Object)
  }

  Object int_obj;
  int_obj.type = INTEGER;
  int_obj.int_value = space->classes_len;

  Object *obj = &space->classes[space->classes_len++];
  obj->type = COMPOSITE;
  obj->comp = class;

  char *class_name = object_to_string(*composite_get(class, "name"));
  hashtable_insert(space->classes_ht, class_name, obj);
  queue_add(&space->classes_q, obj);
  composite_set(class, WHICH_MEMBER, int_obj);

  free(class_name);
}

Object namespace_search(Namespace * const space, Queue * const exts) {

}

void namespace_free(Namespace * const space) {
  free(space->children);
  hashtable_free(space->classes_ht, do_nothing);
  free(space->classes);
  queue_shallow_delete(&space->classes_q);
  free(space->ins);
  free(space);
}
