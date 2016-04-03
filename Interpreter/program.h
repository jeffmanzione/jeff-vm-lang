/*
 * program.h
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include <stdio.h>

#include "instruction.h"

#define INS_DELIM     " \t\n"
#define COMMENT_CH    ';'

#define ADVANCE(pch)    pch = strtok(NULL, INS_DELIM)

typedef struct {
  char word[MAX_LINE_LEN];
  int word_len;
  int new_index;
} Word;
int read_word_prog(Word *, char line[], int index);


int64_t get_int(char *pch);

int load_instructions(FILE *in, InstructionMemory *ins_mem);
int load_bytecode(FILE *in, InstructionMemory *ins_mem);
