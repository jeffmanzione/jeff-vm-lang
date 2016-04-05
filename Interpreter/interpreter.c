/*
 * interpreter.c
 *
 *  Created on: Jan 17, 2016
 *      Author: Jeff
 */

#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>

#include "class.h"
#include "context.h"
#include "hashtable.h"
#include "instruction.h"
#include "instruction_table.h"
#include "parser.h"
#include "program.h"
#include "queue.h"
#include "shared.h"
#include "stack.h"
#include "tokenizer.h"

void load_program(const char in_name[], const char tmp_name[],
    const char out_name[]) {
  InstructionMemory i_mem;
  Stack stack;
  Context *context;
  FILE *file;

  context = context_open(NULL);

  it_init();

  instructions_init(&i_mem, MEM_SZ);
  stack_init(&stack, MEM_SZ);
  class_init(&i_mem);
  FileInfo fi;
  if (ends_with(in_name, ".jl")) {
    fi = file_info(in_name);
    handle_sourcecode(&i_mem, &fi, tmp_name, out_name);
    file_info_finalize(fi);
  } else if (ends_with(in_name, ".jm")) {
    fi = file_info(in_name);
    handle_inscode(&i_mem, &fi, out_name);
    file_info_finalize(fi);
  } else {
    file = fopen(in_name, "rb");
    NULL_CHECK(file, "Could not open file!")
    handle_bytecode(&i_mem, file, out_name);
    fclose(file);
  }
  while (!execute(i_mem.ins[*(context->ip)], &i_mem, &context, &stack))
    ;

  context_close(context);

  it_finalize();
  instructions_finalize(&i_mem);
  stack_finalize(&stack);
  class_finalize();

}

void handle_bytecode(InstructionMemory *i_mem, FILE *file,
    const char out_name[]) {
  load_bytecode(file, i_mem);
}

//void write_classes_to_bin_and_del(void *comp_obj) {
//  write_classes_to_bin("", (Object *) comp_obj);
//  object_delete(comp_obj);
//}

void handle_inscode(InstructionMemory *i_mem, FileInfo *fi,
    const char out_name[]) {
  int count;
  unsigned int zero = 0;

  load_instructions(fi->fp, i_mem);

  FILE *file = fopen(out_name, "wb");
  NULL_CHECK(file, "Could not open file!")

  void write_classes_to_bin(const char id[], Object *comp_obj) {
    char cls_start = '#';
    fwrite(&cls_start, 1, 1, file);
    // Save class if not Class class.
    if (class_class != comp_obj->comp) {
      composite_class_save_bin(file, comp_obj->comp, i_mem);
    }
  }

  hashtable_iterate(i_mem->classes_ht, write_classes_to_bin);

  fwrite(&zero, 1, 1, file);

  count = fwrite(i_mem->ins, sizeof(Instruction), i_mem->num_ins, file);

  CHECK(i_mem->num_ins != count, "Failed to write file!");
}

void handle_sourcecode(InstructionMemory *i_mem, FileInfo *fi,
    const char tmp_name[], const char out_name[]) {
  Queue queue;
  FILE *out;
  queue_init(&queue);
  tokenize(fi, &queue);

  out = fopen(tmp_name, "w");

  parse(fi, &queue, out);

  queue_deep_delete(&queue, free);

  fflush(out);
  fclose(out);

  FileInfo cfi = file_info(tmp_name);
  handle_inscode(i_mem, &cfi, out_name);
  file_info_finalize(cfi);
}

