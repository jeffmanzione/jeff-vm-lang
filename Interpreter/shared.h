/*
 * shared.h
 *
 *  Created on: Jan 4, 2016
 *      Author: Jeff
 */

#ifndef SHARED_H_
#define SHARED_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Instruction Instruction;
typedef struct _Object Object;
typedef struct _ArrayList ArrayList;
typedef struct ArrayList Array;
typedef struct _Composite Composite;

#define TRUE      1
#define FALSE     0
#define SUCCESS   1
#define FAILURE  -1

#define ID_SZ     64

#define MAX_LINE_LEN  4096

#define STR_NUM_BUF_SZ 256

#define MATCHES(src,trgt)         (NULL != src && strlen(trgt) == strlen(src) \
                                       && 0 == strncmp(src, trgt, strlen(trgt)))

#define EXIT_WITH_MSG(err_msg)    { printf("%s\n", err_msg); fflush(stdout); exit(-1); }

#define CHECK(exp, err_msg)       if (exp) { EXIT_WITH_MSG(err_msg); }
#define NULL_CHECK(exp, err_msg)  CHECK( (NULL == (exp)), err_msg)

#define NEW_ARRAY(ref, len, type) (type *) calloc(len, sizeof(type)); \
                                  NULL_CHECK(ref, "Failed to allocate memory.")
#define NEW(ref, type)            NEW_ARRAY(ref, 1, type)
#define RENEW(ref, len, type)     (type *) realloc(ref, (len) * sizeof(type)); \
                                  NULL_CHECK(ref, "Failed to allocate memory.")

typedef void (*Deleter)(void *);
typedef void (*Action)(Object *);

//typedef enum {
//  NONE, INTEGER, FLOATING, STRING, ARRAY
//} Type;

typedef enum {
  NONE, CHARACTER, INTEGER, FLOATING, ARRAY, REFERENCE, COMPOSITE,
  // Instruction only
  STRING_INS,
} Type;

typedef struct _Object {
  Type type;
  union {
    int int_value;
    double float_value;
    char char_value;
    Array *array;
    Object *ref;
    Composite *comp;
  };
} Object;

Object NONE_OBJECT;

int is_whitespace(const char c);
int ends_with(const char *str, const char *suffix);
int starts_with(const char *str, const char *prefix);
int contains_char(const char str[], char c);
void read_word_from_stream(FILE *stream, char *buff);
void advance_to_next(char **ptr, const char c);
void fill_str(char buff[], char *start, char *end);

Object to_ref(Object *obj);
Object deref(Object obj);

void append(FILE *head, FILE *tail);

void object_print(const Object obj, FILE *out);
void object_delete(void *);
char *object_to_string(const Object obj);

char *object_string_merge(const Object, const Object);

Object string_create(const char str[]);
char char_unesc(char u);

void method_to_label(const char *class_name, const char method_name[],
    char *label);

void do_nothing();

#endif /* SHARED_H_ */
