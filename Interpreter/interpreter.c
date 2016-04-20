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
#include "command_line.h"
#include "context.h"
#include "instruction.h"
#include "instruction_table.h"
#include "parser.h"
#include "program.h"
#include "queue.h"
#include "shared.h"
#include "stack.h"
#include "tokenizer.h"

void load_program(CL_ProcessInfo *clpi) {
  InstructionMemory i_mem;
  Stack stack;
  Context *context;
  //FILE *file;

  context = context_open(NULL);

  it_init();

  instructions_init(&i_mem, MEM_SZ);
  stack_init(&stack, MEM_SZ);
  class_init(&i_mem);
  FileInfo fi;

  char source_fn[MAX_LINE_LEN];
  FILE *source_file = NULL;

  char bin_fn[MAX_LINE_LEN];
  FILE *bin_file = NULL;

  if (J_LANG == clpi->in.file_type) {
    fi = file_info(clpi->in.fn);

    if (clpi->action & WRITE_SRC) {
      strncpy(source_fn, clpi->in.fn, MAX_LINE_LEN);
      source_fn[strlen(source_fn) - 1] = 'm';
      source_file = fopen(source_fn, "wb+");
    } else {
      source_file = tmpfile();
    }

    handle_langcode(&i_mem, &fi, source_file);

    file_info_finalize(fi);
  }

  if (J_SOURCE == clpi->in.file_type) {
    fi = file_info(clpi->in.fn);
  } else if (J_LANG == clpi->in.file_type) {
    fflush(source_file);
    rewind(source_file);
    fi = file_info_file(source_file);
  }

  if (J_BIN != clpi->in.file_type) {
    if (clpi->action & WRITE_BIN) {
      strncpy(bin_fn, clpi->in.fn, MAX_LINE_LEN);
      bin_fn[strlen(bin_fn) - 1] = 'b';
      bin_file = fopen(bin_fn, "wb+");
    } else {
      bin_file = tmpfile();
    }

    handle_sourcecode(&i_mem, &fi, bin_file);
    file_info_finalize(fi);
  }

  if (J_BIN == clpi->in.file_type) {
    bin_file = fopen(clpi->in.fn, "rb");
  } else {
    rewind(bin_file);
  }

  handle_bytecode(&i_mem, bin_file);

  if (NULL != source_file) {
    fclose(source_file);
  }

  if (NULL != bin_file) {
    fclose(bin_file);
  }

//  char *in_name = clpi->in.fn;
//  char tmp_name[] = "out.jm";
//  char out_name[] = "out.jb";
//
//  if (ends_with(in_name, ".jl")) {
//    fi = file_info(in_name);
//    handle_langcode(&i_mem, &fi, tmp_name, out_name);
//    file_info_finalize(fi);
//  } else if (ends_with(in_name, ".jm")) {
//    fi = file_info(in_name);
//    handle_sourcecode(&i_mem, &fi, out_name);
//    file_info_finalize(fi);
//  } else {
//    file = fopen(in_name, "rb");
//    NULL_CHECK(file, "Could not open file!")
//    handle_bytecode(&i_mem, file, out_name);
//    fclose(file);
//  }

  if (clpi->action & EXECUTE) {
    while (!execute(i_mem.ins[*(context->ip)], &i_mem, &context, &stack))
      ;

  }
  context_close(context);

  it_finalize();
  instructions_finalize(&i_mem);
  stack_finalize(&stack);
  class_finalize();

}

void handle_bytecode(InstructionMemory *i_mem, FILE *file) {
  load_bytecode(file, i_mem);
}

//void write_classes_to_bin_and_del(void *comp_obj) {
//  write_classes_to_bin("", (Object *) comp_obj);
//  object_delete(comp_obj);
//}

void handle_sourcecode(InstructionMemory *i_mem, FileInfo *fi, FILE *bin_file) {
  int count;
  unsigned int zero = 0;

  load_instructions(fi->fp, i_mem);

  NULL_CHECK(bin_file, "Could not open file!")

  void write_classes_to_bin_for_q(void *comp_obj) {
    char cls_start = '#';
    // Save class if not Class class.
    if (class_class != ((Object *) comp_obj)->comp
        && object_class != ((Object *) comp_obj)->comp) {
      fwrite(&cls_start, 1, 1, bin_file);
      composite_class_save_bin(bin_file, ((Object *) comp_obj)->comp, i_mem);
    }
  }

  if (i_mem->num_classes > 0) {
    queue_iterate(&i_mem->classes_queue, write_classes_to_bin_for_q);
  }
  fwrite(&zero, 1, 1, bin_file);

  count = fwrite(i_mem->ins, sizeof(Instruction), i_mem->num_ins, bin_file);

  CHECK(i_mem->num_ins != count, "Failed to write file!");
}

void handle_langcode(InstructionMemory *i_mem, FileInfo *fi, FILE *source_file) {
  Queue queue;

  queue_init(&queue);
  tokenize(fi, &queue);

  parse(fi, &queue, source_file);

  queue_deep_delete(&queue, free);
}

