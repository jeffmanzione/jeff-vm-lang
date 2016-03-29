/*
 * class.h
 *
 *  Created on: Mar 12, 2016
 *      Author: Jeff
 */

#ifndef CLASS_H_
#define CLASS_H_

#include <stdio.h>

#include "hashtable.h"
#include "instruction.h"
#include "shared.h"

#define DEFAULT_COMPOSITE_HT_SZ 128

typedef struct _Composite Composite;

typedef struct _Composite {
  int is_class;
  Composite *class;
  Hashtable *methods;

  Hashtable *fields;
} Composite;

// Serialized forms

void class_init(InstructionMemory *instructs);
void class_finalize();

Composite *composite_class_new(const char class_name[]);
Composite *composite_class_load(void *serial_class);
//void composite_class_save(FILE *file, Composite *class);
void composite_class_delete(Composite *class);
void composite_class_add_field(Composite *class, const char field_name[]);
void composite_class_add_method(Composite *class, const char method_name[],
    int num_args);
void composite_class_print_sumary(const Composite *class);

Composite *composite_class_load_bin(FILE *stream, InstructionMemory *ins_mem);
Composite *composite_class_load_src(char src[]);
void composite_class_save_src(FILE *file, Composite *class);
void composite_class_save_bin(FILE *file, Composite *class,
    InstructionMemory *ins_mem);

Composite *composite_new(/*Class*/Composite *class);
void composite_delete(Composite *composite);
void composite_set(Composite *composite, const char field_name[], Object value);
Object *composite_get(const Composite *composite, const char field_name[]);
#endif /* CLASS_H_ */
