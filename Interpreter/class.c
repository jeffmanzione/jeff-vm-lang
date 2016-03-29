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

Composite *class_class = NULL;

void class_init(InstructionMemory *instructs) {
  class_class = composite_class_new("Class");
  class_class->class = class_class;

  Object class_obj;
  class_obj.type = COMPOSITE;
  class_obj.comp = class_class;

  composite_set(class_class, "class", class_obj);

  instructions_insert_class(instructs, class_class);
}

void class_finalize() {

}

Composite *composite_class_new(const char class_name[]) {
  Object fields, methods;

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

  class->methods = create_hash_table(DEFAULT_COMPOSITE_HT_SZ);

  return class;
}

void composite_class_add_field(Composite *class, const char field_name[]) {
  Object *fields = composite_get(class, "fields");
  array_enqueue(fields->array, string_create(field_name));
}

void composite_class_add_method(Composite *class, const char method_name[],
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

void composite_class_print_sumary(const Composite *class) {
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

Composite *composite_class_load_bin(FILE *stream, InstructionMemory *ins_mem) {
  char buff[ID_SZ];
  read_word_from_stream(stream, buff);
  Composite *class = composite_class_new(buff);

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
    Object *int_obj = NEW(int_obj, Object)
    int_obj->type = INTEGER;
    int_obj->int_value = adr;

    insert(class->methods, buff, int_obj);
  }

  return class;
}

Composite *composite_class_load_src(char src[]) {
  Composite *class;
  char buff[MAX_LINE_LEN];
  char buff2[MAX_LINE_LEN];

  char *start = src + strlen("class "); // advance to start of class name.
  char *end = start;
  advance_to_next(&end, ':');
  fill_str(buff, start, end);

  class = composite_class_new(buff);

  start = ++end;
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
  }

  return class;
}

void composite_class_save_src(FILE *file, Composite *class) {
  char *class_name = object_to_string(*composite_get(class, "name"));
  fprintf(file, "class %s:fields{", class_name);
  free(class_name);
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

void composite_class_save_bin(FILE *file, Composite *class,
    InstructionMemory *ins_mem) {
  char *class_name = object_to_string(*composite_get(class, "name"));
  fwrite(class_name, strlen(class_name) + 1, 1, file);
  free(class_name);
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
    int num_args = deref(array_get(arr, 1)).int_value;
    fwrite(method_name, strlen(method_name) + 1, 1, file);
    fwrite(&num_args, sizeof(int), 1, file);
    int adr = get(class->methods, method_name)->int_value;
    fwrite(&adr, sizeof(int), 1, file);
    free(method_name);
  }
  fwrite(&n, 1, 1, file);
}

void composite_class_delete(Composite *class) {

}

Composite *composite_new(/*Class*/Composite *class) {
  Composite *obj = NEW(obj, Composite)
  obj->class = class;
  obj->fields = create_hash_table(DEFAULT_COMPOSITE_HT_SZ);

  Object class_obj;
  class_obj.type = COMPOSITE;
  class_obj.comp = class;

  composite_set(obj, "class", class_obj);

  obj->methods = NULL;
  return obj;
}

void composite_delete(Composite *composite) {
  free_table(composite->fields, object_delete);
  if (composite->methods) {
    free_table(composite->methods, object_delete);
  }
  free(composite);
}

void composite_set(Composite *composite, const char field_name[], Object value) {
  Object *ptr = NEW(ptr, Object)
  ;
  *ptr = value;
  insert(composite->fields, field_name, ptr);
}

Object *composite_get(const Composite *composite, const char field_name[]) {
  return get(composite->fields, field_name);
}
