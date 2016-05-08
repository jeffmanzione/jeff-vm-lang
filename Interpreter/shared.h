/*
 * shared.h
 *
 *  Created on: Jan 4, 2016
 *      Author: Jeff
 */

#ifndef SHARED_H_
#define SHARED_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef long double float96_t;
typedef struct _Instruction Instruction;
typedef struct _Object Object;
typedef struct _ArrayList ArrayList;
typedef struct ArrayList Array;
typedef struct _Composite Composite;
typedef struct _ProgramState ProgramState;
typedef struct _Context Context;

typedef enum {
  FALSE, TRUE
} bool;
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

#define NEW_ARRAY(ref, len, type) (type *) calloc(len, sizeof(type)); fflush(stdout); \
                                  NULL_CHECK(ref, "Failed to allocate memory.")
#define NEW(ref, type)            NEW_ARRAY(ref, 1, type)
#define RENEW(ref, len, type)     (type *) realloc(ref, (len) * sizeof(type)); \
                                  NULL_CHECK(ref, "Failed to allocate memory.")

typedef void (*Deleter)(void *);
typedef void (*Q_Action)(void *);
typedef void (*HT_Action)(const char id[], Object *);
typedef Object (*ObjectProducer)();

//typedef enum {
//  NONE, INTEGER, FLOATING, STRING, ARRAY
//} Type;

typedef enum {
  NONE,
  CHARACTER,
  INTEGER,
  FLOATING,
  ARRAY,
  REFERENCE,
  COMPOSITE,
  TUPLE,
  FUNCTION,
  NAMESPACE,
  // Instruction only
  STRING_INS,
} Type;

typedef struct {
  uint32_t namespace_id;
  uint32_t index;
} Address;

int is_whitespace(const char c);
int ends_with(const char *str, const char *suffix);
int starts_with(const char *str, const char *prefix);
int contains_char(const char str[], char c);
void read_word_from_stream(FILE *stream, char *buff);
void advance_to_next(char **ptr, const char c);
void fill_str(char buff[], char *start, char *end);
char char_unesc(char u);

int file_exist(char *filename);
void append(FILE *head, FILE *tail);

void method_to_label(const char *class_name, const char method_name[],
    char *label);

void do_nothing();

void strcrepl(char *src, char from, char to);

#endif /* SHARED_H_ */
