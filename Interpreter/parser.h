/*
 * parser.h
 *
 *  Created on: Jan 9, 2016
 *      Author: Jeff
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>

#include "class.h"
#include "instruction.h"
#include "queue.h"

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

#define AS_KEYWORD         "as"
#define TYPE_INT_KEYWORD   "Int"
#define TYPE_FLOAT_KEYWORD "Float"
#define NONE_KEYWORD       "None"
#define TRUE_KEYWORD       "True"
#define FALSE_KEYWORD      "False"

#define MAIN_FUNCTION      "main"
#define PRINT_FUNCTION     "print"
#define EXIT_FUNCTION      "exit"

extern char *KEYWORDS[];

#define FIRST_COL_INDEX  8
#define SECOND_COL_INDEX 16

void parse(Queue *, FILE *);

void parse_top_level(Queue *, FILE *);
void parse_elements(Queue *, FILE *);

void parse_class(Queue *, FILE *);
void parse_class_body(Queue *, FILE *, Composite *, const char *);
void parse_class_item(Queue *, FILE *, Composite *, const char *);
void parse_class_field(Queue *, FILE *, Composite *, const char *);
void parse_class_method(Queue *, FILE *, Composite *, const char *);

void parse_fun(Queue *, FILE *);
int parse_fun_arguments(Queue *, FILE *);
void parse_body(Queue *, FILE *);
void parse_line(Queue *, FILE *);

void parse_exp(Queue *, FILE *);
int parse_exp_tuple(Queue *, FILE *);
void parse_exp_for(Queue *, FILE *);
void parse_exp_while(Queue *, FILE *);
void parse_exp_if(Queue *, FILE *);
void parse_exp_assign(Queue *, FILE *);
void parse_exp_assign_array(Queue *, FILE *);
void parse_exp_or(Queue *, FILE *);
void parse_exp_xor(Queue *, FILE *);
void parse_exp_and(Queue *, FILE *);
void parse_exp_eq(Queue *, FILE *);
void parse_exp_lt_gt(Queue *, FILE *);
void parse_exp_add_sub(Queue *, FILE *);
void parse_exp_mult_div(Queue *, FILE *);
void parse_exp_array_transfer(Queue *, FILE *);
void parse_exp_casting(Queue *, FILE *);
void parse_exp_unary(Queue *, FILE *);
void parse_exp_parens(Queue *, FILE *);
void parse_exp_obj_item(Queue *, FILE *out);

void parse_exp_array_dec(Queue *, FILE *);

void parse_exp_num_or_id(Queue *, FILE *);

void write_ins_default(Op op, FILE *);
void write_ins_none(FILE *);
void write_ins_value(Op op, int val, FILE *);
void write_ins_value_float(Op op, double val, FILE *);
void write_ins_value_str(Op op, char string[], FILE *);
void write_ins_address(Op op, int adr, FILE *);
void write_ins_id(Op op, char id[], FILE *);
void write_ins_id_num(Op op, char id[], int num, FILE *);
void write_label(char id[], FILE *);
void write_label_num(char id[], int num, FILE *);

#endif /* PARSER_H_ */
