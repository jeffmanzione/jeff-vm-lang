/*
 * class.c
 *
 *  Created on: Mar 12, 2016
 *      Author: Jeff
 */

#include "class.h"

#include "array.h"

Composite *composite_class_new(const char class_name[]) {
  Object fields, methods;

  Composite *class = composite_new(NULL);
  composite_set(class, "name", string_create(class_name));

  fields.array = array_create();
  fields.type = ARRAY;
  composite_set(class, "fields", fields);

  methods.array = array_create();
  methods.type = ARRAY;
  composite_set(class, "methods", methods);

  return class;
}

void composite_class_add_field(Composite *class, const char field_name[]) {
  Object *fields = composite_get(class, "fields");
  array_enqueue(fields->array, string_create(field_name));
}

/*Composite *composite_class_load(void *serial_class) {

 }

 void composite_class_save(FILE *file, Composite *class) {

 }*/

void composite_class_delete(Composite *class) {

}

void composite_class_add_method(Composite *class, const char method_name[],
    int num_args) {

}

Composite *composite_new(/*Class*/Composite *class) {
  Composite *obj = NEW(obj, Composite)
  obj->class = class;
  obj->fields = create_hash_table(DEFAULT_COMPOSITE_HT_SZ);
  return obj;
}

void composite_delete(Composite *composite) {
  free_table(composite->fields, object_delete);
  free(composite);
}

void composite_set(Composite *composite, const char field_name[], Object value) {
  Object *ptr = NEW(ptr, Object)
  ;
  *ptr = value;
  insert(composite->fields, field_name, ptr);
}

Object *composite_get(Composite *composite, const char field_name[]) {
  return get(composite->fields, field_name);
}
