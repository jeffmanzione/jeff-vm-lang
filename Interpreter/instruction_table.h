/*
 * instruction_table.h
 *
 *  Created on: Apr 3, 2016
 *      Author: Jeff
 */

#ifndef INSTRUCTION_TABLE_H_
#define INSTRUCTION_TABLE_H_

#include "instruction.h"

typedef struct _Word Word;

#define COMMENT_CH    ';'
#define MAX_INS_STR_REP_LEN  8
#define INITIAL_IT_SIZE      128

#define READ_INS(ins) it_read_instruction(word, line, &ins, &ids, ins_mem->index, &strs, &ins_indices)

#define INC_TABLE() { global_it.num_ins++; \
                      if (global_it.num_ins >= global_it.table_size) { \
                        RENEW(global_it.instruction_ids, (global_it.table_size += INITIAL_IT_SIZE), InsEntry ) \
                      } \
                    }

#define REGISTER_Def(op_n, str_id, type_n) {global_it.instruction_ids[op_n].op = op_n; \
                                        strncpy(global_it.instruction_ids[op_n].id,  str_id, MAX_INS_STR_REP_LEN); \
                                        global_it.instruction_ids[op_n].id[MAX_INS_STR_REP_LEN] = '\0'; \
                                        global_it.instruction_ids[op_n].type = type_n; \
                                        hashtable_insert(global_it.ins_ht, str_id, (Object *) &global_it.instruction_ids[op_n]); \
                                        INC_TABLE();}
#define REGISTER(op, str_id) REGISTER_Def(op, str_id, I_NONE)
#define REGISTER_Val(op, str_id) REGISTER_Def(op, str_id, I_VAL)
#define REGISTER_Adr(op, str_id) REGISTER_Def(op, str_id, I_ADR)
#define REGISTER_Id(op, str_id)  REGISTER_Def(op, str_id, I_ID)

typedef enum {
  I_VAL, I_NONE, I_ADR, I_ID,
} InsType;

typedef struct {
  Op op;
  char id[MAX_INS_STR_REP_LEN + 1];
  InsType type;
} InsEntry;

typedef struct _InstructionTable {
  InsEntry *instruction_ids;
  Hashtable *ins_ht;
  size_t num_ins;

  // Internal
  size_t table_size;
} InstructionTable;

InstructionTable global_it;


void it_init();

void it_finalize();

void it_read_instruction(Word word, char line[], Instruction *ins, char ***ids,
    int index, Queue *strs, Queue *ins_indices);

InsType it_instruction_type(Op op);

int read_word_prog(Word *, char line[], int index);
int64_t get_int(char *pch);


#endif /* INSTRUCTION_TABLE_H_ */
