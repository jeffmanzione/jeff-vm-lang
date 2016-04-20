/*
 * tokenizer.c
 *
 *  Created on: Jan 6, 2016
 *      Author: Jeff
 */

#include "tokenizer.h"

#include <string.h>

LineInfo line_info(FileInfo *fi, char line_text[], int line_num) {
  LineInfo li;
  strncpy(li.line_text, line_text, MAX_LINE_LEN);
  li.line_num = line_num;
  li.parent = fi;
  return li;
}

FileInfo file_info(const char fn[]) {
  FileInfo fi;
  fi.name = strdup(fn);
  fi.fp = fopen(fn, "r");
  NULL_CHECK(fi.fp, "Could not open file!")
  fi.num_lines = 0;
  fi.array_len = DEFAULT_NUM_LINES;
  fi.lines = NEW_ARRAY(fi.lines, fi.array_len, LineInfo)
  return fi;
}

FileInfo file_info_file(FILE *tmp_file) {
  FileInfo fi;
  fi.name = strdup("TEMPORARY_FILE.jm");
  fi.fp = tmp_file;
  NULL_CHECK(fi.fp, "Could not open file!")
  fi.num_lines = 0;
  fi.array_len = DEFAULT_NUM_LINES;
  fi.lines = NEW_ARRAY(fi.lines, fi.array_len, LineInfo)
  return fi;
}

void file_info_finalize(FileInfo fi) {
  free(fi.name);
  free(fi.lines);
  fclose(fi.fp);
}

void file_info_append(FileInfo *fi, char line_text[]) {
  fi->lines[fi->num_lines++] = line_info(fi, line_text, fi->num_lines);
  if (fi->num_lines >= fi->array_len) {
    fi->array_len += DEFAULT_NUM_LINES;
    fi->lines = RENEW(fi->lines, fi->array_len, LineInfo)
  }
}

LineInfo *file_info_lookup(FileInfo *fi, int line_num) {
  if (line_num < 1 || line_num > fi->num_lines) {
    return NULL;
  }
  return &fi->lines[line_num - 1];
}

Token *token_create(TokenType type, int line, int col, char text[]) {
  Token *tok = NEW(tok, Token)
  tok->type = type;
  tok->line = line;
  tok->col = col;
  tok->len = strlen(text);
  strncpy(tok->text, text, tok->len);
  return tok;
}

Token *token_copy(Token tok) {
  Token *token = NEW(token, Token)

  *token = tok;

  return token;
}

int is_special_char(const char c) {
  switch (c) {
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case '+':
    case '-':
    case '*':
    case '/':
    case '\\':
    case '%':
    case '&':
    case '|':
    case '^':
    case '~':
    case '!':
    case '?':
    case '@':
    case '#':
    case '<':
    case '>':
    case '=':
    case ',':
    case ':':
    case '.':
    case '\'':
      return TRUE;
    default:
      return FALSE;
  }
}

int is_numeric(const char c) {
  return ('0' <= c && '9' >= c);
}

int is_number(const char c) {
  return is_numeric(c) || '.' == c;
}

int is_alphabetic(const char c) {
  return ('A' <= c && 'Z' >= c) || ('a' <= c && 'z' >= c);
}

int is_alphanumeric(const char c) {
  return is_numeric(c) || is_alphabetic(c) || '_' == c;
}

TokenType resolve_type(const char word[], int word_len, int *in_comment) {
  TokenType type;
  if (0 == word_len) {
    type = ENDLINE;
    *in_comment = FALSE;
  } else {
    switch (word[0]) {
      case '(':
        type = LPAREN;
        break;
      case ')':
        type = RPAREN;
        break;
      case '{':
        type = LBRCE;
        break;
      case '}':
        type = RBRCE;
        break;
      case '[':
        type = LBRAC;
        break;
      case ']':
        type = RBRAC;
        break;
      case '+':
        type = word[1] == '+' ? INC : PLUS;
        break;
      case '-':
        type = word[1] == '>' ? RARROW : (word[1] == '-' ? DEC : MINUS);
        break;
      case '*':
        type = STAR;
        break;
      case '/':
        type = FSLASH;
        break;
      case '\\':
        type = BSLASH;
        break;
      case '%':
        type = PERCENT;
        break;
      case '&':
        type = AMPER;
        break;
      case '|':
        type = PIPE;
        break;
      case '^':
        type = CARET;
        break;
      case '~':
        type = TILDE;
        break;
      case '!':
        type = EXCLAIM;
        break;
      case '?':
        type = QUESTION;
        break;
      case '@':
        type = AT;
        break;
      case '#':
        type = POUND;
        break;
      case '<':
        type = word[1] == '-' ? LARROW : (word[1] == '=' ? LTHANEQ : LTHAN);
        break;
      case '>':
        type = word[1] == '=' ? GTHANEQ : GTHAN;
        break;
      case '=':
        type = word[1] == '=' ? EQUIV : EQUALS;
        break;
      case ',':
        type = COMMA;
        break;
      case ':':
        type = COLON;
        break;
      case '.':
        type = PERIOD;
        break;
      case '\'':
        type = STR;
        break;
      case CODE_COMMENT_CH:
        type = SEMICOLON;
        *in_comment = TRUE;
        break;
      default:
        if (('0' <= word[0] && '9' >= word[0]) || '.' == word[0]) {
          if (ends_with(word, "f") || contains_char(word, '.')) {
            type = FLOAT;
          } else {
            type = INT;
          }
        } else {
          type = WORD;
        }
    }
  }
  return type;
}

int is_start_complex(const char c) {
  switch (c) {
    case '+':
    case '-':
    case '<':
    case '>':
    case '=':
      return TRUE;
    default:
      return FALSE;
  }
}

int is_second_complex(const char c) {
  switch (c) {
    case '+':
    case '-':
    case '>':
    case '<':
    case '=':
      return TRUE;
    default:
      return FALSE;
  }
}

int read_word(char **ptr, char word[], int *word_len) {
  char *index = *ptr, *tmp;
  int wrd_ln = 0;

  if (0 == index[0]) {
    word[0] = '\0';
    *word_len = 0;
    tmp = *ptr;
    return 0;
  }

  while (is_whitespace(index[0])) {
    index++;
  }

  if (is_special_char(index[0]) || CODE_COMMENT_CH == index[0]) {
    if (is_start_complex(index[0]) && is_second_complex(index[1])) {
      word[wrd_ln++] = index[0];
      index++;
    }
    word[wrd_ln++] = index[0];
    index++;
  } else {
    if ('\n' != index[0]) {
      word[wrd_ln++] = index[0];
      index++;

      if (is_number(word[0]) || is_alphanumeric(word[0])) {
        if (is_number(word[0])) {
          while (is_number(index[0]) || 'f' == index[0]) {
            word[wrd_ln++] = index[0];
            index++;
          }
        } else {
          while (is_alphanumeric(index[0])) {
            word[wrd_ln++] = index[0];
            index++;
          }
        }
      }
    }
  }

  word[wrd_ln] = '\0';
  *word_len = wrd_ln;
  tmp = *ptr;
  *ptr = index;

//printf("\t'%s' %d %d\n", word, wrd_ln, *ptr - tmp);

  return *ptr - tmp;
}

int read_string(const char line[], char **index, char *word) {
  int word_i = 1;
  word[0] = '\'';
  while ('\'' != **index) {
    word[word_i++] = **index;
    (*index)++;
  }
  word[word_i++] = **index;
  (*index)++;
  word[word_i] = '\0';
  return word_i;
}

void tokenize(FileInfo *fi, Queue *queue) {
  NULL_CHECK(queue, "queue was invalid for tokenize()")

  char line[MAX_LINE_LEN];
  char word[MAX_LINE_LEN];
  char *index;
  int in_comment = FALSE;
  TokenType type;
  int line_num = 0, col_num = 0, word_len = 0, chars_consumed;
  Token *tok;
  while (NULL != fgets(line, MAX_LINE_LEN, fi->fp)) {
    line_num++;
    col_num = 1;
    index = line;

    file_info_append(fi, line);

    while (TRUE) {
      //printf("Test\n");fflush(stdout);
      chars_consumed = read_word(&index, word, &word_len);

      type = resolve_type(word, word_len, &in_comment);

      col_num += chars_consumed;

      if (in_comment) {
        continue;
      }

      if (STR == type) {
        read_string(line, &index, word);
      }

      //printf("%d\n", type);
      //fflush(stdout);

      if (ENDLINE != type
          || (queue->size != 0 && ENDLINE != ((Token *) queue_last(queue))->type)) {

        tok = token_create(type, line_num, col_num - strlen(word), word);
        queue_add(queue, tok);

        //printf("'%s'\n", ((Token *) queue_last(queue))->text); fflush(stdout);
      }

      if (0 == chars_consumed) {
        break;
      }
    }

  }

}
