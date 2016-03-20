/*
 * instruction.h
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <stddef.h>

#include "shared.h"

typedef struct _Instruction Instruction;

#include "context.h"
#include "stack.h"

#define MEM_SZ   4096

#define NUM_VAL(obj) ((INTEGER == obj.type) ? obj.int_value : \
                     ((FLOATING == obj.type) ? obj.float_value : \
                     ((CHARACTER == obj.type) ? obj.char_value : \
                     0)))

#define BIN_A(op, new, n_suffix, first, second) \
    new.n_suffix = NUM_VAL(first) op NUM_VAL(second);

#define BIN_INT(op, new, first, second) \
    new.int_value = (int) (NUM_VAL(first) op NUM_VAL(second)); new.type = INTEGER; \
    if (0 != new.int_value) { new.type = INTEGER; } else { new = NONE_OBJECT; } break;

#define BIN_CHAR(op, new, first, second) \
    new.char_value = (int) (NUM_VAL(first) op NUM_VAL(second)); new.type = CHARACTER; \
    if (0 != new.char_value) { new.type = CHARACTER; } else { new = NONE_OBJECT; } break;

#define BIN_INT_FORCED(op, new, first, second) \
    new.int_value = (int) (((int)NUM_VAL(first)) op ((int)NUM_VAL(second))); \
    if (0 != new.int_value) { new.type = INTEGER; } else { new = NONE_OBJECT; } break;

#define BIN_FLOAT(op, new, first, second) \
    new.float_value = (double) (NUM_VAL(first) op NUM_VAL(second)); new.type = FLOATING; break;

#define BIN(operator, ins, new, first, second)  \
    if (FLOATING == first.type || FLOATING == second.type) {BIN_FLOAT(operator, new, first, second);} \
    else if (INTEGER == first.type || INTEGER == second.type) {BIN_INT(operator, new, first, second);} \
    else if (CHARACTER == first.type || CHARACTER == second.type) {BIN_CHAR(operator, new, first, second);} \
    else { EXIT_WITH_MSG("NOT IMPLEMENTED!!!\n"); } break;

typedef enum {
  // Basic
  NOP,
  EXIT,
  PUSH,
  PUSHM,
  POP,
  FLIP,
  // Set/get
  SET,
  GET,
  // Context switches
  OPEN,
  CLOSE,
  JUMP,
  CALL,
  RET,
  // Unary
  PRINT,
  DUP,
  NOT,
  // Binary num
  ADD,
  SUB,
  MULT,
  DIV,
  MOD,
  // Arg len specified
  PRINTN,
  // Boolean
  AND,
  OR,
  XOR,
  // Equality
  EQ,
  LT,
  LTE,
  GT,
  GTE,
  // IF
  IF,
  IFN,
  IFEQ,
  // Casting
  TOI,
  TOF,
  // Arrays
  ANEW,
  AADD,
  AGET,
  ASET,
  AENQ,
  ADEQ,
  APUSH,
  APOP,
  AINS,
  AREM,
  ALEN,
  ALSH,
  ARSH,
  // TODO: Implement c function calls
  // C stuff
  CCALL,
  // TODO: Implement composite handles
  // Composite Objects
  ONEW,
  OCALL,
  ORET,
  OGET,
  // Reference instructions
  RSET,
  DREF,
  SWAP,
} Op;

extern char *INSTRUCTIONS[];

typedef struct _Instruction {
  Op op;
  Type type;
  union {
    int int_val;
    double float_val;
    int adr;
    char id[ID_SZ];
  };
} Instruction;

typedef struct {
  Instruction *ins;
  int index;
  int num_ins;
} InstructionMemory;

int execute(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);

int instructions_init(InstructionMemory *instructs, size_t capacity);
void instructions_finalize(InstructionMemory *instructs);

#endif /* INSTRUCTION_H_ */
