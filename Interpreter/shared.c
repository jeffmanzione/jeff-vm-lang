/*
 * shared.c
 *
 *  Created on: Jan 5, 2016
 *      Author: Jeff
 */

#include "shared.h"

#include <stddef.h>
#include <string.h>
#include <sys/stat.h>

void do_nothing(void *none) {

}

int is_whitespace(const char c) {
  return ' ' == c || '\t' == c;
}

int ends_with(const char *str, const char *suffix) {
  if (!str || !suffix) {
    return FALSE;
  }

  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);

  if (lensuffix > lenstr) {
    return FALSE;
  }

  return 0 == strncmp(str + lenstr - lensuffix, suffix, lensuffix);
}

int starts_with(const char *str, const char *prefix) {
  if (!str || !prefix) {
    return FALSE;
  }

  size_t lenstr = strlen(str);
  size_t lenprefix = strlen(prefix);

  if (lenprefix > lenstr) {
    return FALSE;
  }

  return 0 == strncmp(str, prefix, lenprefix);
}

void strcrepl(char *src, char from, char to) {
  int i;
  for (i = 0; i < strlen(src); i++) {
    if (from == src[i]) {
      src[i] = to;
    }
  }
}

int contains_char(const char str[], char c) {
  int i;
  for (i = 0; i < strlen(str); i++) {
    if (c == str[i]) {
      return TRUE;
    }
  }

  return FALSE;
}

void read_word_from_stream(FILE *stream, char *buff) {
  int i = 0;
  while ('\0' != (buff[i++] = fgetc(stream)))
    ;
}

void advance_to_next(char **ptr, const char c) {
  while (c != **ptr) {
    (*ptr)++;
  }
}

// Ex: class Doge:fields{age,breed,}methods{new(2),speak(0),}
void fill_str(char buff[], char *start, char *end) {
  buff[0] = '\0';
  strncat(buff, start, end - start);
}

int file_exist(char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

void append(FILE *head, FILE *tail) {
  char buf[BUFSIZ];
  size_t n;

  rewind(tail);
  while ((n = fread(buf, 1, sizeof buf, tail)) > 0) {
    if (fwrite(buf, 1, n, head) != n) {
      EXIT_WITH_MSG("FAILED TO TRANSFER CONTENTS");
    }
  }

  if (ferror(tail)) {
    EXIT_WITH_MSG("FERRROR ON TAIL");
  }
}

char char_unesc(char u) {
  switch (u) {
    case 'a':
      return '\a';
    case 'b':
      return '\b';
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    case '\\':
      return '\\';
    case '\'':
      return '\'';
    case '\"':
      return '\"';
    case '\?':
      return '\?';
    default:
      return u;
  }
}

// Appends the method name in label form to the specified label.
void method_to_label(const char *class_name, const char method_name[],
    char *label) {
  strcat(label, "_");
  strcat(label, class_name);
  strcat(label, "_");
  strcat(label, method_name);
//strcat(label, "_");
}

