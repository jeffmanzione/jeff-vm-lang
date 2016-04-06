/*
 * instruction.h
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <stddef.h>
#include <stdint.h>

#include "hashtable.h"
#include "shared.h"

typedef struct _Instruction Instruction;

#include "context.h"
#include "stack.h"

#define MEM_SZ                 4096
#define DEFAULT_CLASS_ARRAY_SZ 16

#define WHICH_MEMBER           "_which_"


#define NUM_VAL(obj) ((INTEGER == obj.type)   ? obj.int_value : \
                     ((FLOATING == obj.type)  ? obj.float_value : \
                     ((CHARACTER == obj.type) ? obj.char_value : \
                     ((ARRAY == obj.type)     ? (int64_t) (int) obj.array : \
                     ((COMPOSITE == obj.type) ? (int64_t) (int) obj.comp : \
                     0LL)))))

#define BIN_A(op, new, n_suffix, first, second) \
    new.n_suffix = NUM_VAL(first) op NUM_VAL(second);

#define BIN_INT(op, new, first, second) \
    new.int_value = (int64_t) (NUM_VAL(first) op NUM_VAL(second)); new.type = INTEGER; \
    new.type = INTEGER; break;

#define BIN_CHAR(op, new, first, second) \
    new.char_value = (int64_t) (NUM_VAL(first) op NUM_VAL(second)); new.type = CHARACTER; \
    if (0 != new.char_value) { new.type = CHARACTER; } else { new = NONE_OBJECT; } break;

#define BIN_INT_FORCED(op, new, first, second) \
    new.int_value = (int64_t) (((int64_t)NUM_VAL(first)) op ((int64_t)NUM_VAL(second))); \
    new.type = INTEGER; break;

#define BIN_BOOL(op, new, first, second) \
    new.int_value = (int64_t) (((int64_t)NUM_VAL(first)) op ((int64_t)NUM_VAL(second))); \
    if (0 != new.int_value) { new.type = INTEGER; } else { new = NONE_OBJECT; } break;

#define BIN_FLOAT(op, new, first, second) \
    new.float_value = (float96_t) (NUM_VAL(first) op NUM_VAL(second)); new.type = FLOATING; break;

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
  IS,
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
  // Composite Objects
  ONEW,
  OCALL,
  SCALL,
  ORET, // Deprecated
  OGET,
  CLSG, // Class get
  // Reference instructions
  RSET,
  DREF,
  SWAP,
  // Useful low-level instructions
  HASH,
  ISI,
  ISF,
  ISC,
  ISO,
  ISA,
  IST,
  TUPL,
} Op;

typedef struct _Instruction {
  Op   op;
  Type type;
  union {
    int64_t   int_val;
    float96_t float_val;
    int       adr;
    char      id[ID_SZ];
  };
} Instruction;

typedef struct {
  Instruction *ins;
  int          index;
  size_t       num_ins;

  Object      *classes;
  Hashtable   *classes_ht;
  size_t       num_classes, class_array_capacity;
} InstructionMemory;

#include "instruction_table.h"
//typedef struct _InstructionTable InstructionTable;
extern InstructionTable global_it;
#define INSTRUCTIONS(x) global_it.instruction_ids[x].id

int execute(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);

int instructions_init(InstructionMemory *instructs, size_t capacity);
void instructions_finalize(InstructionMemory *instructs);
void instructions_insert_class(InstructionMemory *instructs, Composite *class);
int instructions_get_class_by_name(InstructionMemory *instructs,
    const char class_name[]);
Object instructions_get_class_by_id(InstructionMemory *instructs, int id);
Object instructions_get_class_object_by_name(InstructionMemory *instructs,
    const char class_name[]);

typedef struct _ProgramState {
  InstructionMemory *i_mem;
  Context **context;
  Stack *stack;
} ProgramState;

ProgramState program_state(InstructionMemory *, Context **, Stack *);

#endif /* INSTRUCTION_H_ */
