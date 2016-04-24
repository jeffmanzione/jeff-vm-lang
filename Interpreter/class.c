/*
 * class.c
 *
 *  Created on: Mar 12, 2016
 *      Author: Jeff
 */

#include "class.h"

#include <stdlib.h>
#include <string.h>

#include "array.h"

Class *class_class = NULL;
Class *object_class = NULL;

void class_init(InstructionMemory *instructs) {
  object_class = composite_class_new(OBJECT_CLASS_NAME, NULL);

  Object class_obj;
  class_obj.type = COMPOSITE;
  class_obj.comp = object_class;

  composite_set(object_class, "class", class_obj);
  instructions_insert_class(instructs, object_class);

  composite_class_add_field(object_class, "class");

  class_class = composite_class_new(CLASS_CLASS_NAME, object_class);
  class_class->class = class_class;

  class_obj.type = COMPOSITE;
  class_obj.comp = class_class;

  composite_set(class_class, "class", class_obj);

  composite_class_add_field(class_class, "name");
  composite_class_add_field(class_class, "fields");
  composite_class_add_field(class_class, "methods");
  composite_class_add_field(class_class, "super");

  instructions_insert_class(instructs, class_class);
}

void class_finalize() {

}

Class *composite_class_new(const char class_name[], Class *super_class) {
  Object fields, methods, super;

  Composite *class = composite_new(class_class);
  class->is_class = TRUE;
  class->class = class_class;
  composite_set(class, "name", string_create(class_name));

  fields.array = array_create();
  fields.type = ARRAY;
  composite_set(class, "fields", fields);

  methods.array = array_create();
  methods.type = ARRAY;
  composite_set(class, "methods", methods);

  super.comp = super_class;
  if (NULL == super.comp) {
    super = NONE_OBJECT;
//    super.type = NONE;
  } else {
    super.type = COMPOSITE;
  }
  composite_set(class, "super", super);

  class->methods = hashtable_create(DEFAULT_COMPOSITE_HT_SZ);

  return class;
}

void composite_class_add_field(Class *class, const char field_name[]) {
  Object *fields = composite_get(class, "fields");
  array_enqueue(fields->array, string_create(field_name));
}

void composite_class_add_method(Class *class, const char method_name[],
    int num_args) {
  Object arr;
  arr.type = ARRAY;
  arr.array = array_create();
  Object num_args_obj;
  num_args_obj.type = INTEGER;
  num_args_obj.int_value = num_args;
  array_enqueue(arr.array, string_create(method_name));
  array_enqueue(arr.array, num_args_obj);
  Object *methods = composite_get(class, "methods");
  array_enqueue(methods->array, arr);
}

void composite_class_print_sumary(const Class *class) {
  char *class_name = object_to_string(*composite_get(class, "name"));
  printf("Class: %s\n  Fields:\n", class_name);
  fflush(stdout);
  free(class_name);
  Array *fields = deref(*composite_get(class, "fields")).array;
  int i;
  for (i = 0; i < array_size(fields); i++) {
    char *field_name = object_to_string(deref(array_get(fields, i)));
    printf("    .%s\n", field_name);
    fflush(stdout);
    free(field_name);
  }
  printf("  Methods:\n");
  fflush(stdout);
  Array *methods = deref(*composite_get(class, "methods")).array;
  for (i = 0; i < array_size(methods); i++) {
    Array *arr = deref(array_get(methods, i)).array;

    char *method_name = object_to_string(deref(array_get(arr, 0)));
    int num_args = deref(array_get(arr, 1)).int_value;
    printf("    .%s(%d)\n", method_name, num_args);
    fflush(stdout);
    free(method_name);
  }
  fflush(stdout);
}

Class *composite_class_load_bin(FILE *stream, InstructionMemory *ins_mem) {
  char buff[ID_SZ], buff2[ID_SZ];
  // Read class name.
  read_word_from_stream(stream, buff);
  // Read super class name.
  read_word_from_stream(stream, buff2);

  Class *class = composite_class_new(buff,
      instructions_get_class_object_by_name(ins_mem, buff2).comp);

  while (TRUE) {
    read_word_from_stream(stream, buff);
    if (0 == strlen(buff)) {
      break;
    }
    composite_class_add_field(class, buff);
  }

  while (TRUE) {

    int num_args, adr;
    read_word_from_stream(stream, buff);

    if (0 == strlen(buff)) {
      break;
    }

    fread(&num_args, sizeof(int), 1, stream);
    composite_class_add_method(class, buff, num_args);
    fread(&adr, sizeof(int), 1, stream);
    MethodInfo *method_info = NEW(method_info, MethodInfo)
    method_info->address = adr;
    method_info->num_args = num_args;
    hashtable_insert(class->methods, buff, method_info);
  }

  return class;
}

Class *composite_class_load_src(char src[], InstructionMemory *ins_mem) {
  Composite *class;
  char buff[MAX_LINE_LEN];
  char buff2[MAX_LINE_LEN];

  char *start = src + strlen("class "); // advance to start of class name.
  char *end = start;
  advance_to_next(&end, ':');
  fill_str(buff, start, end);
  start = ++end;
  advance_to_next(&end, ':');
  fill_str(buff2, start, end);
  start = ++end;

  class = composite_class_new(buff,
      instructions_get_class_object_by_name(ins_mem, buff2).comp);
  //class = composite_class_new(buff, );

  advance_to_next(&end, '{');
  start = ++end;

  while ('}' != *start) {
    advance_to_next(&end, ',');
    fill_str(buff, start, end);
    composite_class_add_field(class, buff);
    start = ++end;
  }
  advance_to_next(&end, '{');
  start = ++end;

  while ('}' != *start) {
    int num_args;
    advance_to_next(&end, '(');
    fill_str(buff, start, end);
    start = ++end;
    advance_to_next(&end, ')');
    fill_str(buff2, start, end);
    num_args = (int) strtol(buff2, NULL, 10);
    advance_to_next(&end, ',');
    start = ++end;
    composite_class_add_method(class, buff, num_args);

    MethodInfo *method_info = NEW(method_info, MethodInfo)
    method_info->num_args = num_args;
    hashtable_insert(class->methods, buff, method_info);

  }

  return class;
}

void composite_class_save_src(FILE *file, Class *class) {
  char *class_name = object_to_string(*composite_get(class, "name"));
  char *super_class_name = object_to_string(
      *composite_get(composite_get(class, "super")->comp, "name"));
  fprintf(file, "class %s:%s:fields{", class_name, super_class_name);
  free(class_name);
  free(super_class_name);
  Array *fields = deref(*composite_get(class, "fields")).array;
  int i;
  for (i = 0; i < array_size(fields); i++) {
    char *field_name = object_to_string(deref(array_get(fields, i)));
    fprintf(file, "%s,", field_name);
    free(field_name);
  }
  fprintf(file, "} methods{");
  Array *methods = deref(*composite_get(class, "methods")).array;
  for (i = 0; i < array_size(methods); i++) {
    Array *arr = deref(array_get(methods, i)).array;
    char *method_name = object_to_string(deref(array_get(arr, 0)));
    int num_args = deref(array_get(arr, 1)).int_value;
    fprintf(file, "%s(%d),", method_name, num_args);
    free(method_name);
  }
  fprintf(file, "}");
}

void composite_class_save_bin(FILE *file, Class *class,
    InstructionMemory *ins_mem) {
  char *class_name = object_to_string(*composite_get(class, "name"));
  fwrite(class_name, strlen(class_name) + 1, 1, file);

  free(class_name);
  class_name = object_to_string(
      *composite_get(composite_get(class, "super")->comp, "name"));
  fwrite(class_name, strlen(class_name) + 1, 1, file);

  unsigned char n = 0;

  Array *fields = deref(*composite_get(class, "fields")).array;
  int i;
  for (i = 0; i < array_size(fields); i++) {
    char *field_name = object_to_string(deref(array_get(fields, i)));
    fwrite(field_name, strlen(field_name) + 1, 1, file);

    free(field_name);
  }
  fwrite(&n, 1, 1, file);

  Array *methods = deref(*composite_get(class, "methods")).array;
  for (i = 0; i < array_size(methods); i++) {
    Array *arr = deref(array_get(methods, i)).array;
    char *method_name = object_to_string(deref(array_get(arr, 0)));

    MethodInfo *mi = hashtable_lookup(class->methods, method_name);
    NULL_CHECK(mi, "MethodInfo for method was not found in table!")
    //printf("%s.%s(%d)\n", class_name, method_name, mi->num_args);
    int num_args = mi->num_args;

    fwrite(method_name, strlen(method_name) + 1, 1, file);
    fwrite(&num_args, sizeof(int), 1, file);
    int adr = mi->address;
    fwrite(&adr, sizeof(int), 1, file);
    free(method_name);
  }
  fwrite(&n, 1, 1, file);
  free(class_name);
}

void composite_class_delete(Class *class) {

}

Composite *composite_new(/*Class*/Composite *class) {
  Composite *obj = NEW(obj, Composite)
  obj->class = class;
  obj->fields = hashtable_create(DEFAULT_COMPOSITE_HT_SZ);

  Object class_obj;
  class_obj.type = COMPOSITE;
  class_obj.comp = class;

  composite_set(obj, "class", class_obj);

  obj->methods = NULL;
  return obj;
}

void composite_delete(Composite *composite) {
  hashtable_free(composite->fields, object_delete);
  if (composite->methods) {
    hashtable_free(composite->methods, free);
  }
  free(composite);
}

void composite_set(Composite *composite, const char field_name[], Object value) {
  Object *ptr = NEW(ptr, Object)
  ;
  *ptr = value;
  hashtable_insert(composite->fields, field_name, ptr);
}

Object *composite_get(const Composite *composite, const char field_name[]) {
  return hashtable_lookup(composite->fields, field_name);
}

Object *composite_get_even_if_not_present(Composite *composite,
    const char field_name[]) {
  Object *tmp = hashtable_lookup(composite->fields, field_name);

  if (NULL == tmp) {
    composite_set(composite, field_name, NONE_OBJECT);
    return composite_get(composite, field_name);
  } else {
    return tmp;
  }

}

int composite_has_field(const Composite *composite, const char field_name[]) {
  return NULL != hashtable_lookup(composite->fields, field_name);
}
