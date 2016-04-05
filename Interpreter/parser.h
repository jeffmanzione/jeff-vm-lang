/*
 * parser.h
 *
 *  Created on: Jan 9, 2016
 *      Author: Jeff
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdint.h>
#include <stdio.h>

#include "hashtable.h"
#include "instruction.h"
#include "queue.h"
#include "shared.h"
#include "tokenizer.h"

#define DASH_STRING "----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"
#define CARET_STRING "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"

#define FUN_KEYWORD        "function"
#define DEF_KEYWORD        "def"
#define IF_KEYWORD         "if"
#define ELSE_KEYWORD       "else"
#define WHILE_KEYWORD      "while"
#define FOR_KEYWORD        "for"
#define BREAK_KEYWORD      "break"
#define RETURN_KEYWORD     "return"
#define CLASS_KEYWORD      "class"
#define FIELD_KEYWORD      "field"
#define IMPORT_KEYWORD     "import"
#define NEW_KEYWORD        "new"
#define SELF_KEYWORD       "self"
#define SUPER_KEYWORD      "super"


#define AS_KEYWORD         "as"
#define IS_KEYWORD         "is"
#define ISNT_KEYWORD       "isnt"
#define TYPE_INT_KEYWORD   "Int"
#define TYPE_FLOAT_KEYWORD "Float"
#define TYPE_CHAR_KEYWORD  "Char"
#define TYPE_ARRAY_KEYWORD "Array"
#define TYPE_OBJ_KEYWORD   "Object"

#define NONE_KEYWORD       "None"
#define TRUE_KEYWORD       "True"
#define FALSE_KEYWORD      "False"

#define MAIN_FUNCTION      "main"
#define PRINT_FUNCTION     "print"
#define EXIT_FUNCTION      "exit"
#define HASH_FUNCTION      "hash_uint32_t_"

extern char *KEYWORDS[];

#define FIRST_COL_INDEX  8
#define SECOND_COL_INDEX 16

typedef struct {
    Queue     *tok_q;
    Hashtable *fun_names;
    Hashtable *classes;
    Queue      classes_queue;
    FileInfo  *fi_in;
    char      *in_name;
    FILE      *top;
} Parser;

void parse(FileInfo *fi, Queue *, FILE *);

void parse_top_level(Parser *, FILE *);
void parse_elements(Parser *, FILE *);

void parse_class(Parser *, FILE *);
void parse_class_body(Parser *, FILE *, Composite *, const char *);
void parse_class_item(Parser *, FILE *, Composite *, const char *);
void parse_class_field(Parser *, FILE *, Composite *, const char *);
void parse_class_method(Parser *, FILE *, Composite *, const char *);

void parse_fun(Parser *, FILE *);
int parse_fun_arguments(Parser *, FILE *);
void parse_body(Parser *, FILE *);
void parse_line(Parser *, FILE *);

void parse_exp(Parser *, FILE *);
int parse_exp_tuple(Parser *, FILE *);
void parse_exp_for(Parser *, FILE *);
void parse_exp_while(Parser *, FILE *);
void parse_exp_if(Parser *, FILE *);
void parse_exp_assign(Parser *, FILE *);
void parse_exp_assign_array(Parser *, FILE *);
void parse_exp_or(Parser *, FILE *);
void parse_exp_xor(Parser *, FILE *);
void parse_exp_and(Parser *, FILE *);
void parse_exp_eq(Parser *, FILE *);
void parse_exp_lt_gt(Parser *, FILE *);
void parse_exp_add_sub(Parser *, FILE *);
void parse_exp_mult_div(Parser *, FILE *);
void parse_exp_array_transfer(Parser *, FILE *);
void parse_exp_casting(Parser *, FILE *);
void parse_exp_unary(Parser *, FILE *);
void parse_exp_subscript(Parser *, FILE *);
void parse_exp_parens(Parser *, FILE *);
void parse_exp_obj_item(Parser *, FILE *);

void parse_exp_array_dec(Parser *, FILE *);

void parse_exp_num_or_id(Parser *, FILE *);

void write_ins_default(Op op, FILE *);
void write_ins_none(FILE *);
void write_ins_value(Op op, int64_t val, FILE *);
void write_ins_value_float(Op op, float96_t val, FILE *);
void write_ins_value_str(Op op, char string[], FILE *);
void write_ins_address(Op op, int adr, FILE *);
void write_ins_id(Op op, char id[], FILE *);
void write_ins_id_num(Op op, char id[], int num, Parser *,FILE *);
void write_label(char id[], FILE *);
void write_label_num(char id[], int num, Parser *, FILE *);

#endif /* PARSER_H_ */
