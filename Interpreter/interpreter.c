/*
 * interpreter.c
 *
 *  Created on: Jan 17, 2016
 *      Author: Jeff
 */

#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>

#include "context.h"
#include "instruction.h"
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

  file = fopen(in_name, "r");

  //printf("sizeof(Instruction) = %d\n", sizeof(Instruction));
  //fflush(stdout);

  NULL_CHECK(file, "Could not open file!")

  instructions_init(&i_mem, MEM_SZ);
  stack_init(&stack, MEM_SZ);

  if (ends_with(in_name, ".jl")) {
    handle_sourcecode(&i_mem, file, tmp_name, out_name);
  } else if (ends_with(in_name, ".jm")) {
    handle_inscode(&i_mem, file, out_name);
  } else {
    handle_bytecode(&i_mem, file, out_name);
  }

  fclose(file);

  while (!execute(i_mem.ins[*(context->ip)], &i_mem, &context, &stack))
    ;

  context_close(context);

  instructions_finalize(&i_mem);
  stack_finalize(&stack);

}

void handle_bytecode(InstructionMemory *i_mem, FILE *file,
    const char out_name[]) {
  load_bytecode(file, i_mem);
}

void handle_inscode(InstructionMemory *i_mem, FILE *file, const char out_name[]) {
  int count;
  load_instructions(file, i_mem);
  fclose(file);

  file = fopen(out_name, "wb");
  NULL_CHECK(file, "Could not open file!")

  count = fwrite(i_mem->ins, sizeof(Instruction), i_mem->num_ins, file);

  CHECK(i_mem->num_ins != count, "Failed to write file!");
}

void handle_sourcecode(InstructionMemory *i_mem, FILE *file,
    const char tmp_name[], const char out_name[]) {
  Queue queue;
  FILE *out;
  queue_init(&queue);
  tokenize(file, &queue);

  out = fopen(tmp_name, "w");

  parse(&queue, out);

  queue_deep_delete(&queue, free);

  fflush(out);
  fclose(out);

  fclose(file);

  file = fopen(tmp_name, "r");
  NULL_CHECK(file, "Could not open file!")

  handle_inscode(i_mem, file, out_name);
}

