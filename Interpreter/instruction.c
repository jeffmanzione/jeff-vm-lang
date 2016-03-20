/*
 * instruction.c
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include "instruction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

char *INSTRUCTIONS[] = { "nop", "exit", "push", "pushm", "pop", "flip", "set",
    "get", "open", "close", "jump", "call", "ret", "print", "dup", "not", "add",
    "sub", "mult", "div", "mod", "printn", "and", "or", "xor", "eq", "lt",
    "lte", "gt", "gte", "if", "ifn", "ifeq", "toi", "tof", "anew", "aadd",
    "aget", "aset", "aenq", "adeq", "apush", "apop", "ains", "arem", "alen",
    "alsh", "arsh", "ccall", "onew", "ocall", "oret", "oget", "rset", "dref",
    "swap" };

void execute_exit(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_unary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_unary_ref(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_binary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_var_len(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);

void execute_array(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_array_shift(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_array_unary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_array_binary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_array_ternary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);

void execute_unary_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_bin_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_lookup(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_frame(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);

int instructions_init(InstructionMemory *instructs, size_t capacity) {
  instructs->ins = (Instruction *) calloc(capacity, sizeof(Instruction));
  instructs->num_ins = capacity;
  instructs->index = 0;
  return capacity;
}

void instructions_finalize(InstructionMemory *instructs) {
  free(instructs->ins);
}

int execute(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  //printf(">>> %s(%d)\n", INSTRUCTIONS[ins.op], stack->sp);
  //fflush(stdout);
  Object val, tmp;
  Object first, second;
  //char *str;
  switch (ins.op) {
    case (EXIT):
      execute_exit(ins, ins_mem, context, stack);
      return TRUE;
    case (PUSH):
      if (INTEGER == ins.type) {
        val.int_value = ins.int_val;
      } else if (FLOATING == ins.type) {
        val.float_value = ins.float_val;
      } else if (NONE == ins.type) {
        val = NONE_OBJECT;
      } else {
        EXIT_WITH_MSG("NOT IMPLEMENTED PUSH!");
      }
      val.type = ins.type;
      push_stack(stack, val);
      break;
    case (PUSHM):
      // TODO
//    str = (char *) (ins_mem->ins + ins.adr);
//    val.type = STRING;
//    val.str = NEW_ARRAY(val.str, strlen(str) + 1, char)
//    strncpy(val.str, str, strlen(str));

      val = string_create((char *) (ins_mem->ins + ins.adr));

      push_stack(stack, val);
      //printf("%s\n", str);
      break;
    case (POP):
      deref(pop_stack(stack));
      break;
    case (FLIP):
      first = pop_stack(stack);
      second = pop_stack(stack);
      push_stack(stack, first);
      push_stack(stack, second);
      break;
    case (SWAP):
      first = pop_stack(stack);
      second = pop_stack(stack);
      CHECK(!(REFERENCE == first.type && REFERENCE == second.type),
          "Both entries must be references for swap.")
      tmp = *first.ref;
      *first.ref = *second.ref;
      *second.ref = tmp;
      break;
    case (JUMP):
    case (CALL):
      execute_addr(ins, ins_mem, context, stack);
      break;
    case (IF):
    case (IFN):
      execute_unary_addr(ins, ins_mem, context, stack);
      break;
    case (ADD):
    case (SUB):
    case (MULT):
    case (DIV):
    case (MOD):
    case (AND):
    case (OR):
    case (XOR):
    case (EQ):
    case (LT):
    case (LTE):
    case (GT):
    case (GTE):
      execute_binary(ins, ins_mem, context, stack);
      break;
    case (PRINTN):
      execute_var_len(ins, ins_mem, context, stack);
      break;
    case (IFEQ):
      execute_bin_addr(ins, ins_mem, context, stack);
      break;
    case (PRINT):
    case (NOT):
    case (TOI):
    case (TOF):
    case (NOP):
      execute_unary(ins, ins_mem, context, stack);
      break;
    case (DUP):
    case (DREF):
      execute_unary_ref(ins, ins_mem, context, stack);
      break;
    case (OPEN):
    case (CLOSE):
    case (RET):
      execute_frame(ins, ins_mem, context, stack);
      break;
    case (SET):
    case (RSET):
    case (GET):
      execute_lookup(ins, ins_mem, context, stack);
      break;
    case (ANEW):
    case (AADD):
    case (ALSH):
    case (ARSH):
    case (AGET):
    case (ASET):
    case (AENQ):
    case (ADEQ):
    case (APUSH):
    case (APOP):
    case (AINS):
    case (AREM):
    case (ALEN):
      execute_array(ins, ins_mem, context, stack);
      break;
    default:
      printf(">>> %d\n", ins.op);
      EXIT_WITH_MSG("Unknown Instruction!")
  }
  *((*context)->ip) = *((*context)->ip) + 1;
  return FALSE;
}

void execute_frame(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  switch (ins.op) {
    case (OPEN):
      *context = context_open(*context);
      break;
    case (CLOSE):
    case (RET):
      *context = context_close(*context);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_unary!")
  }
}

void execute_exit(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object val = deref(pop_stack(stack));

  printf("Exit status: %d\n", val.int_value);
  fflush(stdout);
}

void execute_unary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object val = deref(pop_stack(stack));
  switch (ins.op) {
    case (NOT):
      val.int_value = !val.int_value;
      push_stack(stack, val);
      break;
    case (PRINT):
      object_print(val, stdout);
      //printf("\n");
      fflush(stdout);
      break;
    case (TOI):
      val.int_value = (int) NUM_VAL(val);
      val.type = INTEGER;
      push_stack(stack, val);
      break;
    case (TOF):
      val.float_value = (double) NUM_VAL(val);
      val.type = FLOATING;
      push_stack(stack, val);
      break;
    case (NOP):
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_unary!")
  }
}

void execute_unary_ref(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object val = pop_stack(stack);
  switch (ins.op) {
    case (DUP):
      push_stack(stack, val);
      push_stack(stack, val);
      break;
    case (DREF):
      push_stack(stack, deref(val));
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_unary!")
  }
}

void execute_binary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object new;
  Object second = deref(pop_stack(stack));
  Object first = deref(pop_stack(stack));
  switch (ins.op) {
    case (ADD):
      /*if (STRING == second.type || STRING == first.type) {
       new.type = STRING;
       new.str = object_string_merge(first, second);
       free(first.str);
       free(second.str);
       } else { */
      BIN(+, ins, new, first, second)
      //}
      break;
    case (SUB):
      BIN(-, ins, new, first, second)
    case (MULT):
      BIN(*, ins, new, first, second)
    case (DIV):
      BIN(/, ins, new, first, second)
    case (MOD):
      BIN_INT_FORCED(%, new, first, second)
    case (AND):
      BIN_INT_FORCED(&, new, first, second)
    case (OR):
      BIN_INT_FORCED(|, new, first, second)
    case (XOR):
      BIN_INT_FORCED(^, new, first, second)
    case (EQ):
      BIN_INT(==, new, first, second)
    case (LT):
      BIN_INT(<, new, first, second)
    case (LTE):
      BIN_INT(<=, new, first, second)
    case (GT):
      BIN_INT(>, new, first, second)
    case (GTE):
      BIN_INT(>=, new, first, second)
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_binary!")
  }
  push_stack(stack, new);
}

void execute_var_len_help(int num_args, Stack *stack) {
  Object val;

  if (1 == num_args) {
    object_print(deref(pop_stack(stack)), stdout);
    fflush(stdout);
    return;
  }

  val = deref(pop_stack(stack));

  execute_var_len_help(num_args - 1, stack);

  printf(" ");
  object_print(val, stdout);

  fflush(stdout);
}

void execute_var_len(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object len = deref(pop_stack(stack));
  CHECK(INTEGER != len.type,
      "Can only use int to specify length for var_len instruction")

  if (0 >= len.int_value) {
    return;
  }

  switch (ins.op) {
    case (PRINTN):
      execute_var_len_help(len.int_value, stack);
      printf("\n");
      fflush(stdout);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_var_len!")
  }
}

Array * const get_array(Stack *stack) {
  Object array_obj = deref(pop_stack(stack));
  CHECK(ARRAY != array_obj.type, "Tried to manipulate something not an array.")
  return array_obj.array;
}

void execute_array(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object new, second;
  Object array_obj;
  switch (ins.op) {
    case (ANEW): // [ ... ]
      new.array = array_create();
      new.type = ARRAY;
      push_stack(stack, new);
      break;
    case (AADD):
      second = deref(pop_stack(stack));
      array_obj = deref(peek_stack(stack));
      //printf("type=%d\n", second.type);
      CHECK(ARRAY != array_obj.type,
          "Tried to manipulate something not an array.")
      array_enqueue(array_obj.array, second);
      break;
    case (ALSH):
    case (ARSH):
      execute_array_shift(ins, ins_mem, context, stack);
      break;
    case (ALEN): // |arr|
    case (ADEQ): // arr => x
    case (APOP): // x <= arr
      execute_array_unary(ins, ins_mem, context, stack);
      break;
    case (AGET): // arr[i]
    case (AENQ): // arr <= x
    case (APUSH): // x => arr
    case (AREM): // x <- arr[i]
      execute_array_binary(ins, ins_mem, context, stack);
      break;
    case (ASET): // arr[i] = x
    case (AINS): // arr[i] <- x
      execute_array_ternary(ins, ins_mem, context, stack);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_array!")
  }
}

void execute_array_shift(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object second = deref(pop_stack(stack));
  Object first = deref(pop_stack(stack));
  CHECK(ARRAY != second.type || ARRAY != first.type,
      "Cannot use <</>> with non int/int or array.")
  switch (ins.op) {
    case (ALSH):
      array_enqueue(first.array, array_pop(second.array));
      break;
    case (ARSH):
      array_push(second.array, array_dequeue(first.array));
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_binary_shift!")
  }
}

void execute_array_unary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Array * const array = get_array(stack);
  Object new;
  switch (ins.op) {
    case (ALEN): // |arr|
      new.int_value = array_size(array);
      new.type = INTEGER;
      break;
    case (ADEQ):
      new = array_dequeue(array);
      break;
    case (APOP):
      new = array_pop(array);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_array_unary!")
  }
  push_stack(stack, new);
}

void execute_array_binary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object new;
//  1  2
// arr[i]

  Object second = deref(pop_stack(stack));
  Array * const array = get_array(stack);

  switch (ins.op) {
    case (AENQ):
      array_enqueue(array, second);
      return;
    case (APUSH):
      array_push(array, second);
      return;
    case (AGET): // arr[i]
      CHECK(NONE != second.type && INTEGER != second.type,
          "Tried to index an array with something not an integer.")
      new = array_get(array, second.int_value);
      break;
    case (AREM): // x <- arr[i]
      CHECK(NONE != second.type && INTEGER != second.type,
          "Tried to index an array with something not an integer.")
      new = array_remove(array, second.int_value);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_array_binary!")
  }
  push_stack(stack, new);
}

void execute_array_ternary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object third = deref(pop_stack(stack));
  Object second = deref(pop_stack(stack));
  Array * const array = get_array(stack);

  CHECK(NONE != second.type && INTEGER != second.type,
      "Tried to index an array with something not an integer.")

  switch (ins.op) {
    case (ASET): // arr[i] = x
      array_set(array, second.int_value, third);
      break;
    case (AINS): // arr[i] <- x
      array_insert(array, second.int_value, third);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_array_ternary!")
  }
}

void call_context(const Instruction ins, InstructionMemory *ins_mem,
    Context **context) {
  int adr;
  *context = context_open(*context);
  (*context)->ip = NEW((*context)->ip, int)
  (*context)->new_ip = TRUE;
  adr = ins.adr;
  CHECK(FAILURE == adr, "No known label.");
  *((*context)->ip) = adr;
}

void execute_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  switch (ins.op) {
    case (JUMP):
      *((*context)->ip) = ins.adr;
      break;
    case (CALL):
      call_context(ins, ins_mem, context);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_addr!")
  }
}

void execute_unary_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object val = deref(pop_stack(stack));
  switch (ins.op) {
    case (IF):
      if (NONE != val.type) {
        *((*context)->ip) = ins.adr;
      }
      break;
    case (IFN):
      if (NONE == val.type) {
        *((*context)->ip) = ins.adr;
      }
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_unary!")
  }
}

void execute_bin_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object second = deref(pop_stack(stack));
  Object first = deref(pop_stack(stack));
  switch (ins.op) {
    case (IFEQ):
      if (first.int_value == second.int_value) {
        *((*context)->ip) = ins.adr;
      }
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_bin_addr!")
  }
}

void execute_lookup(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object *val_ptr;
  Object val, ref;
  switch (ins.op) {
    case (GET):
      val_ptr = context_lookup(ins.id, *context);
      if (NONE == val_ptr->type) {
        val = NONE_OBJECT;
      } else {
        val.type = REFERENCE;
        val.ref = val_ptr;
      }
      push_stack(stack, val);
      //printf("Getting value of '%s' which is of %d.\n", ins.id, val_ptr->type);
      break;
    case (SET):
      val = deref(pop_stack(stack));
      context_set(ins.id, val, *context);
      //printf("Setting value of '%s' which is of %d.\n", ins.id, val.type);
      break;
    case (RSET):
      val = deref(pop_stack(stack));
      ref = pop_stack(stack);
      if (NONE == ref.type) {
        EXIT_WITH_MSG("Attempting to assign value to null pointer.")
      } else {
        *(ref.ref) = val;
      }
      //printf("RSET to type %d.\n", val.type);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_lookup!")
  }
}

