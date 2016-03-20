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

int is_whitespace(const char c) {
  return ' ' == c || '\t' == c;
}

int ends_with(const char *str, const char *suffix) {
  if (!str || !suffix) {
    return 0;
  }

  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);

  if (lensuffix > lenstr) {
    return 0;
  }

  return 0 == strncmp(str + lenstr - lensuffix, suffix, lensuffix);
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

Object deref(Object obj) {
  if (REFERENCE == obj.type) {
    return *obj.ref;
  }
  return obj;
}

char *object_to_string(const Object obj) {
  char buf[STR_NUM_BUF_SZ];
  char *str;
  int len;
  /*if (STRING == obj.type) {
   str = NEW_ARRAY(str, strlen(obj.str) + 1, char)
   strncpy(str, obj.str, strlen(obj.str));
   } else*/if (FLOATING == obj.type) {
    len = sprintf(buf, "%f", obj.float_value);
    str = NEW_ARRAY(str, len + 1, char)
    strncpy(str, buf, len);
  } else if (INTEGER == obj.type) {
    len = sprintf(buf, "%d", obj.int_value);
    str = NEW_ARRAY(str, len + 1, char)
    strncpy(str, buf, len);
  } else if (CHARACTER == obj.type) {
    len = sprintf(buf, "%c", obj.char_value);
    str = NEW_ARRAY(str, len + 1, char)
    strncpy(str, buf, len);
  } else if (NONE == obj.type) {
    len = sprintf(buf, "%s", "(None)");
    str = NEW_ARRAY(str, len + 1, char)
    strncpy(str, buf, len);
  } else {
    EXIT_WITH_MSG("Tried to convert something unexpected to a string.")
  }

  return str;
}

/*Object object_to_string_object(const Object obj) {
 char *tmp = object_to_string(obj);
 Object new;
 new.type = STRING;
 new.str = tmp;

 return new;
 }*/

char *object_string_merge(const Object first, const Object second) {
  char *result, *first_s, *second_s;

  first_s = object_to_string(first);
  second_s = object_to_string(second);

  result = NEW_ARRAY(result, strlen(first_s) + strlen(second_s) + 1, char)

  strcpy(result, first_s);
  strcat(result, second_s);

  free(first_s);
  free(second_s);
  return result;
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
