/*
 * interpreter.h
 *
 *  Created on: Jan 6, 2016
 *      Author: Jeff
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdio.h>

#include "instruction.h"

void load_program(const char in_name[], const char tmp_name[],
    const char out_name[]);

void handle_sourcecode(InstructionMemory *i_mem, FILE *file,
    const char tmp_name[], const char out_name[]);
void handle_inscode(InstructionMemory *i_mem, FILE *file, const char out_name[]);
void handle_bytecode(InstructionMemory *i_mem, FILE *file,
    const char out_name[]);

#endif /* INTERPRETER_H_ */
