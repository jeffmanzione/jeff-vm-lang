/*
 * shared.c
 *
 *  Created on: Jan 5, 2016
 *      Author: Jeff
 */

#include "shared.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "class.h"
#include "array.h"

Object TRUE_OBJECT = (Object ) {INTEGER, {(uint64_t) 1}};

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
    fprintf(out, "%" PRId64, obj.int_value);
  } else if (FLOATING == obj.type) {
    fprintf(out, "%f", (double) obj.float_value);
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
  } else if (TUPLE == obj.type) {
    fprintf(out, "(");

    if (0 < obj.tuple_size) {
      object_print_outer(deref(obj.tuple_elements[0]), out);
      int i;
      for (i = 1; i < obj.tuple_size; i++) {
        fprintf(out, ",");
        object_print_outer(deref(obj.tuple_elements[i]), out);
      }
    }

    fprintf(out, ")");
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

uint32_t hash_code_array(Array *array, ProgramState state) {
  uint32_t hash = 0;
  int i;
  for (i = 0; i < array_size(array); i++) {
    hash = hash_code(deref(array_get(array, i)), state) + (hash << 5) - hash;
  }
  return hash;
}

uint32_t hash_code_composite(Composite *comp, ProgramState state) {
//  unsigned int hash = 0;
//  int i;
//  for (i = 0; i < sizeof(Composite); i++) {
//    hash = ((char *) comp)[i] + (hash << 5) - hash;
//  }
//
//  return hash;
  return (uint32_t) comp;
}

uint32_t hash_code_tuple(const Object obj, ProgramState state) {
  uint32_t hash = 0;
  int i;
  for (i = 0; i < obj.tuple_size; i++) {
    hash ^= hash_code(obj.tuple_elements[i], state);
  }
  return hash;
}

uint32_t hash_code(const Object obj, ProgramState state) {
  uint64_t ul;

  switch (obj.type) {
    case NONE:
      return 0;
    case CHARACTER:
      return (uint32_t) obj.char_value;
    case INTEGER:
      return (uint32_t) obj.int_value;
    case FLOATING:
      memcpy(&ul, &obj.float_value, sizeof(float96_t));
      return (((uint32_t) ul) ^ ((uint32_t) (ul >> 32)));
    case ARRAY:
      return hash_code_array(obj.array, state);
    case COMPOSITE:
      return hash_code_composite(obj.comp, state);
    case TUPLE:
      return hash_code_tuple(obj, state);
    case REFERENCE:
      printf("REF!!!!\n");
      return -1;
    default:
      return -1;
  }
}

Object equals_array(Array *a1, Array *a2, ProgramState state) {
  int i;

  if (a1 == a2) {
    return TRUE_OBJECT;
  }

  if (array_size(a1) != array_size(a2)) {
    return NONE_OBJECT;
  }

  for (i = 0; i < array_size(a1); i++) {
    if (NONE
        == equals(deref(array_get(a1, i)), deref(array_get(a2, i)), state).type) {
      return NONE_OBJECT;
    }
  }
  return TRUE_OBJECT;
}

Object equals_composite(Composite *c1, Composite * c2, ProgramState state) {
  /*if (c1->class != c2->class) {
   return NONE_OBJECT;
   }*/

  int answer = TRUE;

  void equals_elt(const char id[], Object *field_val) {
    Object *c2_v = composite_get(c2, id);
    answer = answer && equals_ptr(c2_v, field_val, state).int_value;
  }

  hashtable_iterate(c1->fields, equals_elt);

  return answer > 0 ? TRUE_OBJECT : NONE_OBJECT;
}

Object equals_tuple(Object o1, Object o2, ProgramState state) {
  if (o1.tuple_size != o2.tuple_size) {
    return NONE_OBJECT;
  }

  int i;
  for (i = 0; i < o1.tuple_size; i++) {
    if (NONE
        == equals(o1.tuple_elements[i], o2.tuple_elements[i], state).type) {
      return NONE_OBJECT;
    }
  }
  return TRUE_OBJECT;
}

Object equals(Object o1, Object o2, ProgramState state) {
  if (hash_code(o1, state) != hash_code(o2, state)) {
    return NONE_OBJECT;
  }

  switch (o1.type) {
    case NONE:
      return NONE_OBJECT;
    case CHARACTER:
      if (o1.char_value == o2.char_value) {
        return TRUE_OBJECT;
      } else {
        return NONE_OBJECT;
      }
    case INTEGER:
      if (o1.int_value == o2.int_value) {
        return TRUE_OBJECT;
      } else {
        return NONE_OBJECT;
      }
    case FLOATING:
      if (o1.float_value == o2.float_value) {
        return TRUE_OBJECT;
      } else {
        return NONE_OBJECT;
      }
    case ARRAY:
      return equals_array(o1.array, o2.array, state);
    case COMPOSITE:
      return equals_composite(o1.comp, o2.comp, state);
    case TUPLE:
      return equals_tuple(o1, o2, state);
    default:
      return NONE_OBJECT;
  }
}

Object equals_ptr(Object *o1, Object *o2, ProgramState state) {
  if (o1 == o2) {
    return TRUE_OBJECT;
  }
  return equals(*o1, *o2, state);
}

Object to_ref(Object *obj) {
  Object ref;
  ref.type = REFERENCE;
  ref.ref = obj;
  return ref;
}

Object obj_is_a(Object type, Object to_test, ProgramState state) {
  Class *target = type.comp;
  Class *test = to_test.comp->class;
  Object *super_obj;
  do {
    Object ans = equals_composite(target, test, state);
    if (NONE != ans.type) {
      return TRUE_OBJECT;
    }

    super_obj = composite_get(test, "super");
    test = super_obj->comp;
  } while (NONE != super_obj->type);

  return NONE_OBJECT;

}

Object deref(Object obj) {
  if (REFERENCE == obj.type) {
    return *obj.ref;
  }
  return obj;
}

Object object_tuple_base(int num_elts) {
  Object tup;
  tup.type = TUPLE;
  tup.tuple_size = num_elts;
  tup.tuple_elements = NEW_ARRAY(tup.tuple_elements, num_elts, Object)
  return tup;
}

Object object_tuple(int64_t num_elts, ...) {
  Object tup = object_tuple_base(num_elts);
  va_list elts;
  va_start(elts, num_elts);
  int i;
  for (i = 0; i < num_elts; i++) {
    tup.tuple_elements[i] = va_arg(elts, Object);
  }
  va_end(elts);
  return tup;
}

Object object_tuple_get(int64_t num_elts, ObjectProducer get_obj) {
  Object tup = object_tuple_base(num_elts);
  int i;
  for (i = 0; i < num_elts; i++) {
    tup.tuple_elements[i] = get_obj();
  }

  return tup;
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

