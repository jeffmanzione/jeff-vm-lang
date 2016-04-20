/*
 * interpreter.h
 *
 *  Created on: Jan 6, 2016
 *      Author: Jeff
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdio.h>

#include "command_line.h"
#include "instruction.h"
#include "tokenizer.h"

void load_program(CL_ProcessInfo *clpi);

void handle_langcode(InstructionMemory *i_mem, FileInfo *fi,
    FILE *source_file);
void handle_sourcecode(InstructionMemory *i_mem, FileInfo *fi,
    FILE *bin_file);
void handle_bytecode(InstructionMemory *i_mem, FILE *bin_file);

#endif /* INTERPRETER_H_ */
