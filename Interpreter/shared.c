/*
 * shared.c
 *
 *  Created on: Jan 5, 2016
 *      Author: Jeff
 */

#include "shared.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

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

void delete_object_helper(Object *obj) {

  /*if (STRING == obj->type) {
   free(obj->str);
   } else*/if (ARRAY == obj->type) {
    array_delete(obj->array);
  } else if (COMPOSITE == obj->type) {
    composite_delete(obj->comp);
  }

  free(obj);
}

void object_delete(void *obj) {
  NULL_CHECK(obj, "delete_object(null)")

  delete_object_helper((Object *) obj);
}

void object_print_outer(Object obj, FILE *out) {
  //printf("Type: %d\n", obj.type);
  //obj = deref(obj);
  if (INTEGER == obj.type) {
    fprintf(out, "%d", obj.int_value);
  } else if (FLOATING == obj.type) {
    fprintf(out, "%f", obj.float_value);
  } else if (CHARACTER == obj.type) {
    fprintf(out, "%c", obj.char_value);
  } else if (ARRAY == obj.type) {
    fprintf(out, "[");

    if (0 < array_size(obj.array)) {
      object_print_outer(deref(array_get(obj.array, 0)), out);
      int i;
      for (i = 1; i < array_size(obj.array); i++) {
        fprintf(out, ",");
        object_print_outer(deref(array_get(obj.array, i)), out);
      }
    }

    fprintf(out, "]");
  } else if (NONE == obj.type) {
    fprintf(out, "%s", "(None)");
  } /* else if (STRING == obj.type) {
   fprintf(out, "'%s'", obj.str);
   } */else {
    fprintf(out, "?");
    //EXIT_WITH_MSG("NOT IMPLEMENTED PRINT!")
  }
  fflush(out);
}

void object_print(const Object obj, FILE *out) {
  //printf("Type: %d\n", obj.type);
  // if (STRING == obj.type) {
  //   fprintf(out, "%s", obj.str);
  // } else {
  object_print_outer(obj, out);
  //}
  fflush(out);
}

Object to_ref(Object *obj) {
  Object ref;
  ref.type = REFERENCE;
  ref.ref = obj;
  return ref;
}

Object deref(Object obj) {
  if (REFERENCE == obj.type) {
    return *obj.ref;
  }
  return obj;
}

char *object_to_string(const Object obj) {

  char *str;
  if (ARRAY == obj.type) {
    str = NEW_ARRAY(str, array_size(obj.array) + 1, char)
    int i;
    //printf("%d\n", array_size(obj.array));
    for (i = 0; i < array_size(obj.array); i++) {
      Object t = array_get(obj.array, i);
      //printf("t %d\n", t.ref->type);
      //fflush(stdout);
      Object obj = deref(t);

      CHECK(CHARACTER != obj.type,
          "Cannot convert an array that is not of characters to a string!")
      str[i] = obj.char_value;
    }
    str[i] = '\0';
  } else {
    printf("Was a %d\n", obj.type);
    EXIT_WITH_MSG("Tried to convert something unexpected to a string.")
  }

  return str;
}

Object string_create(const char str[]) {
  int i, in_esc = FALSE;
  Object obj, tmp;
  obj.type = ARRAY;
  obj.array = array_create();

  tmp.type = CHARACTER;

  for (i = 0; i < strlen(str); i++) {
    if (!in_esc && '\\' == str[i]) {
      in_esc = TRUE;
      continue;
    } else if (in_esc) {
      tmp.char_value = char_unesc(str[i]);
    } else {
      tmp.char_value = str[i];

    }
    in_esc = FALSE;
    array_enqueue(obj.array, tmp);
  }

  return obj;
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

