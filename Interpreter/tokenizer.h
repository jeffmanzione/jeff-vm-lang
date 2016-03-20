/*
 * tokenizer.h
 *
 *  Created on: Jan 6, 2016
 *      Author: Jeff
 */

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <stdio.h>

#include "queue.h"
#include "shared.h"

#define CODE_DELIM      " \t"
#define CODE_COMMENT_CH ';'

typedef enum {
  WORD, INT, FLOAT,

  // For strings.
  STR,

  // Structural symbols
  LPAREN, RPAREN, LBRCE, RBRCE, LBRAC, RBRAC,

  // Math symbols
  PLUS, MINUS, STAR, FSLASH, BSLASH, PERCENT,

  // Binary/bool symbols
  AMPER, PIPE, CARET, TILDE,

  // Following not used yet
  EXCLAIM, QUESTION, AT, POUND,

  // Equivalence
  LTHAN, GTHAN, EQUALS,

  // Others
  ENDLINE, SEMICOLON, COMMA, COLON, PERIOD,

  // Specials
  LARROW, RARROW, INC, DEC, LTHANEQ, GTHANEQ, EQUIV

} TokenType;

typedef struct {
  TokenType type;
  int col, line;
  size_t len;
  char text[ID_SZ];
} Token;

void tokenize(FILE *in, Queue *queue);

Token *token_create(TokenType type, int line, int col, char text[]);
Token *token_copy(Token tok);
#endif /* TOKENIZER_H_ */
