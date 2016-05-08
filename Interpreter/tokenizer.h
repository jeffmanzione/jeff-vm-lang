/*
 * tokenizer.h
 *
 *  Created on: Jan 6, 2016
 *      Author: Jeff
 */

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <stdio.h>

#include "shared.h"
#include "queue.h"

#define CODE_DELIM      " \t"
#define CODE_COMMENT_CH ';'

typedef enum {
  WORD, INT, FLOAT,

  // For strings.
  STR,

  // Structural symbols
  LPAREN,
  RPAREN,
  LBRCE,
  RBRCE,
  LBRAC,
  RBRAC,

  // Math symbols
  PLUS,
  MINUS,
  STAR,
  FSLASH,
  BSLASH,
  PERCENT,

  // Binary/bool symbols
  AMPER,
  PIPE,
  CARET,
  TILDE,

  // Following not used yet
  EXCLAIM,
  QUESTION,
  AT,
  POUND,

  // Equivalence
  LTHAN,
  GTHAN,
  EQUALS,

  // Others
  ENDLINE,
  SEMICOLON,
  COMMA,
  COLON,
  PERIOD,

  // Specials
  LARROW,
  RARROW,
  INC,
  DEC,
  LTHANEQ,
  GTHANEQ,
  EQUIV

} TokenType;

#define DEFAULT_NUM_LINES   128

typedef struct _FileInfo FileInfo;

typedef struct {
  char      line_text[MAX_LINE_LEN + 1];
  int       line_num;
  FileInfo *parent;
} LineInfo;

LineInfo line_info(FileInfo *fi, char line_text[], int line_num);

typedef struct _FileInfo {
  char     *name;
  FILE     *fp;
  LineInfo *lines;
  size_t    num_lines;
  size_t    array_len;
} FileInfo;

FileInfo file_info(const char fn[]);
FileInfo file_info_file(FILE *tmp_file);

void file_info_finalize(FileInfo fi);

void file_info_append(FileInfo *fi, char line_text[]);

LineInfo *file_info_lookup(FileInfo *fi, int line_num);

typedef struct {
  TokenType type;
  int       col, line;
  size_t    len;
  char      text[ID_SZ];
} Token;

void tokenize(FileInfo *fi, Queue *queue);

Token *token_create(TokenType type, int line, int col, char text[]);
Token *token_copy(Token tok);
#endif /* TOKENIZER_H_ */
