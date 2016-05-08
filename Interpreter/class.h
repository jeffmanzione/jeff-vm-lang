/*
 * class.h
 *
 *  Created on: Mar 12, 2016
 *      Author: Jeff
 */

#ifndef CLASS_H_
#define CLASS_H_

#include <stdint.h>
#include <stdio.h>

typedef struct _Composite Composite;
typedef struct _Composite Class;

#include "hashtable.h"
#include "instruction.h"
#include "object.h"

#define DEFAULT_COMPOSITE_HT_SZ 128

#define CLASS_CLASS_NAME  "Class"
#define OBJECT_CLASS_NAME  "Object"

extern Class *class_class;
extern Class *object_class;

typedef struct _Composite {
  int is_class;
  Composite *class;
  Hashtable *methods;

  Hashtable *fields;
} Composite;

typedef struct {
  uint16_t num_args;
  uint64_t address; // Not known until right before execution.
} MethodInfo;

// Serialized forms

void class_init(InstructionMemory *instructs);
void class_finalize();

Class *composite_class_new(const char class_name[], Class *super_class);
Class *composite_class_load(void *serial_class);
//void composite_class_save(FILE *file, Composite *class);
void composite_class_delete(Class *class);
void composite_class_add_field(Class *class, const char field_name[]);
void composite_class_add_method(Class *class, const char method_name[],
    int num_args);
void composite_class_print_sumary(const Class *class);

Composite *composite_class_load_bin(FILE *stream, InstructionMemory *ins_mem);
Composite *composite_class_load_src(char src[], InstructionMemory *ins_mem);
void composite_class_save_src(FILE *file, Class *class);
void composite_class_save_bin(FILE *file, Class *class,
    InstructionMemory *ins_mem);

Composite *composite_new(/*Class*/Composite *class);
void composite_delete(Composite *composite);
void composite_set(Composite *composite, const char field_name[], Object value);
Object *composite_get(const Composite *composite, const char field_name[]);
Object *composite_get_even_if_not_present(Composite *composite,
    const char field_name[]);
bool composite_has_field(const Composite *composite, const char field_name[]);
bool composite_has_method(const Composite *composite,
    const char method_name[], int num_args);
#endif /* CLASS_H_ */
