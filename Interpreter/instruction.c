/*
 * instruction.c
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include "instruction.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "class.h"

//char *INSTRUCTIONS[] = { "nop", "exit", "push", "pushm", "pop", "flip", "set",
//    "get", "is", "open", "close", "jump", "call", "ret", "print", "dup", "not",
//    "add", "sub", "mult", "div", "mod", "printn", "and", "or", "xor", "eq",
//    "lt", "lte", "gt", "gte", "if", "ifn", "ifeq", "toi", "tof", "anew", "aadd",
//    "aget", "aset", "aenq", "adeq", "apush", "apop", "ains", "arem", "alen",
//    "alsh", "arsh", "ccall", "onew", "ocall", "scall", "oret", "oget", "clsg",
//    "rset", "dref", "swap", "hash", "isi", "isf", "isc", "iso", "isa" };

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

void execute_composites(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_composites_id(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);

void execute_unary_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_bin_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_lookup(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_frame(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_tuple(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);
void execute_fun_ptr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack);

void ocall_fun(const char fun_name[], InstructionMemory *ins_mem,
    Context **context, Stack *stack, Object comp_obj, int num_args);

Object stack_pull(Stack *stack) {
  return deref(stack_pop(stack));
}

int instructions_init(InstructionMemory *instructs, size_t capacity) {
  instructs->ins = (Instruction *) calloc(capacity, sizeof(Instruction));
  instructs->num_ins = capacity;
  instructs->index = 0;
  instructs->classes_ht = hashtable_create(TABLE_SZ);
  instructs->class_array_capacity = DEFAULT_CLASS_ARRAY_SZ;
  instructs->classes = NEW_ARRAY(instructs->classes, DEFAULT_CLASS_ARRAY_SZ,
      Object)
  instructs->num_classes = 0;
  queue_init(&instructs->classes_queue);
  return capacity;
}

void instructions_finalize(InstructionMemory *instructs) {
  free(instructs->ins);
  if (NULL != instructs->classes_ht) {
    // TODO
    hashtable_free(instructs->classes_ht, do_nothing);
  }
  if (NULL != instructs->classes) {
    free(instructs->classes);
  }

  queue_shallow_delete(&instructs->classes_queue);
}

void instructions_insert_class(InstructionMemory *instructs, Composite *class) {
  if (instructs->num_classes == instructs->class_array_capacity) {
    instructs->class_array_capacity += DEFAULT_CLASS_ARRAY_SZ;
    instructs->classes = RENEW(instructs->classes,
        instructs->class_array_capacity, Object)
  }

  Object int_obj;
  int_obj.type = INTEGER;
  int_obj.int_value = instructs->num_classes;

  Object *obj = &instructs->classes[instructs->num_classes++];
  obj->type = COMPOSITE;
  obj->comp = class;

  char *class_name = object_to_string(*composite_get(class, "name"));
  hashtable_insert(instructs->classes_ht, class_name, obj);
  queue_add(&instructs->classes_queue, obj);
  composite_set(class, WHICH_MEMBER, int_obj);

  free(class_name);
  //composite_class_print_sumary(class);
}

int instructions_get_class_by_name(InstructionMemory *instructs,
    const char class_name[]) {

  Object *comp_obj = hashtable_lookup(instructs->classes_ht, class_name);

  NULL_CHECK(comp_obj, "Class could not be found upon lookup!")

  return deref(*composite_get(comp_obj->comp, WHICH_MEMBER)).int_value;
}

Object instructions_get_class_by_id(InstructionMemory *instructs, int id) {

  CHECK(instructs->num_classes <= id,
      "Bad class specified in bytecode. Beyond limit.")

  return instructs->classes[id];
}

Object instructions_get_class_object_by_name(InstructionMemory *instructs,
    const char class_name[]) {
  NULL_CHECK(instructs, "InstructionMemory * was null!");

  return instructions_get_class_by_id(instructs,
      instructions_get_class_by_name(instructs, class_name));
}

int execute(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
//  printf(">>> %s(%d)\n", INSTRUCTIONS(ins.op), stack->sp);
//  fflush(stdout);
  Object val, tmp;
  Object first, second;
//char *str;
  switch (ins.op) {
    case (NOP):
      break;
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
      stack_push(stack, val);
      break;
    case (PUSHM):
      // TODO
//    str = (char *) (ins_mem->ins + ins.adr);
//    val.type = STRING;
//    val.str = NEW_ARRAY(val.str, strlen(str) + 1, char)
//    strncpy(val.str, str, strlen(str));

      val = string_create((char *) (ins_mem->ins + ins.adr));

      stack_push(stack, val);
      //printf("%s\n", str);
      break;
    case (POP):
      stack_pull(stack);
      break;
    case (FLIP):
      first = stack_pop(stack);
      second = stack_pop(stack);
      stack_push(stack, first);
      stack_push(stack, second);
      break;
    case (SWAP):
      first = stack_pop(stack);
      second = stack_pop(stack);
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
    case (IS):
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
    case (HASH):
    case (ISI):
    case (ISF):
    case (ISC):
    case (ISO):
    case (ISA):
    case (IST):
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
    case (ONEW):
    case (SCALL):
    case (OCALL):
    case (ORET):
    case (OGET):
    case (CLSG):
      execute_composites(ins, ins_mem, context, stack);
      break;
    case (TUPL):
    case (TGET):
    case (IGET):
      execute_tuple(ins, ins_mem, context, stack);
      break;
    case (PGET):
    case (PCALL):
      execute_fun_ptr(ins, ins_mem, context, stack);
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
  Object val = stack_pull(stack);

  printf("Exit status: %"PRId64"\n", val.int_value);
  fflush(stdout);
}

void execute_tuple(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object val = stack_pull(stack), tup, new;

  Object get_my_obj() {
    return stack_pull(stack);
  }

  CHECK(INTEGER != val.type, "All indexable ops should have a size specified")
  switch (ins.op) {
    case (TUPL):
      tup = object_tuple_get(val.int_value, get_my_obj);
      stack_push(stack, tup);
      break;
    case (IGET): // indexable[i]
    case (TGET):
      tup = stack_pull(stack);
      if (TUPLE == tup.type) {
        CHECK(val.int_value >= tup.tuple_size,
            "Cannot index out of tuple bounds for TGET")
        stack_push(stack, tup.tuple_elements[val.int_value]);
      } else if (ARRAY == tup.type) {
        new = array_get(tup.array, val.int_value);
        stack_push(stack, new);
      } else
        EXIT_WITH_MSG("Expected type tuple for TGET")

      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_tuple!")
  }
}

void execute_unary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object val = stack_pull(stack);
  switch (ins.op) {
    case (NOT):
      if (NONE == val.type) {
        val = TRUE_OBJECT;
      } else {
        val = NONE_OBJECT;
      }
      stack_push(stack, val);
      break;

    case (PRINT):
      if (COMPOSITE == val.type) {
        if (composite_has_method(val.comp, "to_s", 0)) {
          *((*context)->ip) = *((*context)->ip) - 1;
          ocall_fun("to_s", ins_mem, context, stack, val, 0);
        } else {
          fprintf(stdout, "(Object)");
        }
      } else {
        object_print(val, stdout);
      }
      //printf("\n");
      fflush(stdout);
      break;
    case (HASH):
      val.int_value = hash_code(val, program_state(ins_mem, context, stack));
      val.type = INTEGER;
      stack_push(stack, val);
      break;
    case (TOI):
      val.int_value = (int64_t) NUM_VAL(val);
      val.type = INTEGER;
      stack_push(stack, val);
      break;
    case (TOF):
      val.float_value = (float96_t) NUM_VAL(val);
      val.type = FLOATING;
      stack_push(stack, val);
      break;
    case (ISI):
      val = (INTEGER == val.type) ? TRUE_OBJECT : NONE_OBJECT;
      stack_push(stack, val);
      break;
    case (ISF):
      val = (FLOATING == val.type) ? TRUE_OBJECT : NONE_OBJECT;
      stack_push(stack, val);
      break;
    case (ISC):
      val = (CHARACTER == val.type) ? TRUE_OBJECT : NONE_OBJECT;
      stack_push(stack, val);
      break;
    case (ISO):
      val = (COMPOSITE == val.type) ? TRUE_OBJECT : NONE_OBJECT;
      stack_push(stack, val);
      break;
    case (ISA):
      val = (ARRAY == val.type) ? TRUE_OBJECT : NONE_OBJECT;
      stack_push(stack, val);
      break;
    case (IST):
      val = (TUPLE == val.type) ? TRUE_OBJECT : NONE_OBJECT;
      stack_push(stack, val);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_unary!")
  }
}

void execute_unary_ref(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object val = stack_pop(stack);
  switch (ins.op) {
    case (DUP):
      stack_push(stack, val);
      stack_push(stack, val);
      //printf("duped val = ");object_print(val, stdout); printf("\n");
      break;
    case (DREF):
      stack_push(stack, deref(val));
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_unary!")
  }
}

void execute_binary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object new;
  Object second = stack_pull(stack);
  Object first = stack_pull(stack);
  switch (ins.op) {
    case (ADD):
      if (ARRAY == first.type && ARRAY == second.type) {
        new.type = ARRAY;
        new.array = array_create();
        int i;
        for (i = 0; i < array_size(first.array); i++) {
          array_enqueue(new.array, deref(array_get(first.array, i)));
        }
        for (i = 0; i < array_size(second.array); i++) {
          array_enqueue(new.array, deref(array_get(second.array, i)));
        }
      } else {
        BIN(+, ins, new, first, second)
      }
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
      BIN_BOOL(&, new, first, second)
    case (OR):
      BIN_BOOL(|, new, first, second)
    case (XOR):
      BIN_BOOL(^, new, first, second)
    case (EQ):
      if (COMPOSITE == first.type
          && composite_has_method(first.comp, "eq", 1)) {
        stack_push(stack, second);
        ocall_fun("eq", ins_mem, context, stack, first, 1);
        return;
      } else {
        new = equals(first, second, program_state(ins_mem, context, stack));
      }
      break;
    case (IS):
      //BIN_INT(==, new, first, second)
      new = obj_is_a(second, first, program_state(ins_mem, context, stack));
      break;
    case (LT):
      BIN_BOOL(<, new, first, second)
    case (LTE):
      BIN_BOOL(<=, new, first, second)
    case (GT):
      BIN_BOOL(>, new, first, second)
    case (GTE):
      BIN_BOOL(>=, new, first, second)
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_binary!")
  }
  stack_push(stack, new);
}

void execute_var_len_help(int num_args, Stack *stack) {
  Object val;

  if (1 == num_args) {
    object_print(stack_pull(stack), stdout);
    fflush(stdout);
    return;
  }

  val = stack_pull(stack);

  execute_var_len_help(num_args - 1, stack);

  printf(" ");
  object_print(val, stdout);

  fflush(stdout);
}

void execute_var_len(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object len = stack_pull(stack);
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
  Object array_obj = stack_pull(stack);
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
      stack_push(stack, new);
      break;
    case (AADD):
      second = stack_pull(stack);
      array_obj = deref(stack_peek(stack));
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
    case (IGET): // arr[i]
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
  Object second = stack_pull(stack);
  Object first = stack_pull(stack);
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
  stack_push(stack, new);
}

void execute_array_binary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object new;
//  1  2
// arr[i]

  Object second = stack_pull(stack);
  Array * const array = get_array(stack);

  switch (ins.op) {
    case (AENQ):
      array_enqueue(array, second);
      return;
    case (APUSH):
      array_push(array, second);
      return;
    case (AREM): // x <- arr[i]
      CHECK(NONE != second.type && INTEGER != second.type,
          "Tried to index an array with something not an integer.")
      new = array_remove(array, second.int_value);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_array_binary!")
  }
  stack_push(stack, new);
}

void execute_array_ternary(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object third = stack_pull(stack);
  Object second = stack_pull(stack);
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

void call_context_adr_with_parent(Address address, InstructionMemory *ins_mem,
    Context **context, Context *parent) {
  int adr;
  *context = context_open_with_parent(*context, parent);
  (*context)->ip = NEW((*context)->ip, int)
  adr = address.index;
  CHECK(FAILURE == adr, "No known label.");
  *((*context)->ip) = adr;
}

void call_context_adr(Address address, InstructionMemory *ins_mem,
    Context **context) {
  call_context_adr_with_parent(address, ins_mem, context, *context);
  (*context)->new_ip = TRUE;
}

void call_context(const Instruction ins, InstructionMemory *ins_mem,
    Context **context) {
  Address address = { 0, ins.adr };
  call_context_adr(address, ins_mem, context);
}

void execute_addr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  switch (ins.op) {
    case (JUMP):
      //printf("INS[44]: %s\n", INSTRUCTIONS[ins_mem->ins[44].op]); fflush(stdout);
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
  Object val = stack_pull(stack);
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
  Object second = stack_pull(stack);
  Object first = stack_pull(stack);
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
      stack_push(stack, val);
      //printf("Getting value of '%s' which is of %d.\n", ins.id, val_ptr->type); fflush(stdout);
      break;
    case (SET):
      val = stack_pull(stack);
      //printf("\nSETTING %s %d\n", ins.id, val.type);
      context_set(ins.id, val, *context);
      //printf("!!! %d\n", context_lookup(ins.id, *context)->type);
      break;
    case (RSET):
      val = stack_pull(stack);
      ref = stack_pop(stack);
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

void execute_composites(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object obj;
  switch (ins.op) {
    case (ORET):
      *context = context_close(*context);
      break;
    case (CLSG):
      obj = instructions_get_class_by_id(ins_mem, ins.int_val);
      stack_push(stack, obj);
      break;
    case (OCALL):
    case (OGET):
    case (ONEW):
    case (SCALL):
      execute_composites_id(ins, ins_mem, context, stack);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_composites!")
  }
}

void ocall_fun_helper(const char fun_name[], InstructionMemory *ins_mem,
    Context **context, Stack *stack, Object comp_obj, Class *class,
    int expected_num_args) {

  //printf("Calling %p\n", get(comp_obj.comp->class->methods, fun_name));
  //fflush(stdout);

  int is_new = MATCHES(fun_name, "new");
  int method_exists = TRUE;

  MethodInfo *mi = hashtable_lookup(class->methods, fun_name);

  while (NULL == mi || expected_num_args != mi->num_args) {
    Object *super = composite_get(class, "super");

    if (NONE == super->type) {
      method_exists = FALSE;
      if (!is_new || expected_num_args > 0) {
        EXIT_WITH_MSG("Object does not support specified method with that "
            "number of arguments!")
      }
      break;
    }
    class = super->comp;
    mi = hashtable_lookup(class->methods, fun_name);
  }
  if (method_exists) {
    int adr;
    *context = context_open(*context);
    (*context)->ip = NEW((*context)->ip, int)
    (*context)->new_ip = TRUE;
    context_set("self", comp_obj, *context);
    context_set("super", comp_obj, *context);
    adr = mi->address;
    CHECK(FAILURE == adr, "No known label.");
    *((*context)->ip) = adr;
  } else {
    stack_push(stack, comp_obj);
  }
}

void ocall_fun(const char fun_name[], InstructionMemory *ins_mem,
    Context **context, Stack *stack, Object comp_obj, int num_args) {
  ocall_fun_helper(fun_name, ins_mem, context, stack, comp_obj,
      comp_obj.comp->class, num_args);
}

void ocall_context(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack, Object comp_obj, int num_args) {
  ocall_fun(ins.id, ins_mem, context, stack, comp_obj, num_args);
}

void scall_context(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack, Object comp_obj, int num_args) {
  ocall_fun_helper(ins.id, ins_mem, context, stack, comp_obj,
      composite_get(comp_obj.comp->class, "super")->comp, num_args);
}

void execute_composites_id(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object to_get;
  Object args_obj;
  Object new, *tmp;
  int num_args = 0;
  Composite *comp;
//char fun_name[ID_SZ];

  if (OGET != ins.op) {
    args_obj = stack_pull(stack);
    CHECK(INTEGER != args_obj.type, "num_args specifier must be an integer!")
    CHECK(0 > args_obj.int_value, "num_args specifier must be >= 0!")
    num_args = args_obj.int_value;
  }

  Object obj = stack_pull(stack);

  CHECK(COMPOSITE != obj.type,
      "Can only perform composite operation on composite.")
  comp = obj.comp;

  if (OGET != ins.op && 1 == num_args && 0 < stack->sp) {
    Object peek = deref(stack_peek(stack));
    if (TUPLE == peek.type) {
      num_args = peek.tuple_size;
    }
  }

  switch (ins.op) {
    case (ONEW):
      //CHECK(TRUE != (obj.comp->is_class), "Tried to new something not a class!")
      new.type = COMPOSITE;
      new.comp = composite_new(obj.comp);
      ocall_fun("new", ins_mem, context, stack, new, num_args);
      break;
    case (OCALL):
      ocall_context(ins, ins_mem, context, stack, obj, num_args);
      break;
    case (SCALL):
      scall_context(ins, ins_mem, context, stack, obj, num_args);
      break;
    case (OGET):
      tmp = composite_get_even_if_not_present(comp, ins.id);
      NULL_CHECK(tmp, "Class does not support this field!")
      to_get = to_ref(tmp);

      stack_push(stack, to_get);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_composites_id!")
  }
}

void execute_fun_ptr(const Instruction ins, InstructionMemory *ins_mem,
    Context **context, Stack *stack) {
  Object obj;
  switch (ins.op) {
    case (PGET):
      obj.type = FUNCTION;
      obj.address.namespace_id = 0;
      obj.address.index = ins.adr;
      obj.parent_context = *context;
      stack_push(stack, obj);
      break;
    case (PCALL):
      obj = stack_pull(stack);
      CHECK(FUNCTION != obj.type,
          "Cannot treat a non-function pointer as a function.")
      call_context_adr_with_parent(obj.address, ins_mem, context,
          obj.parent_context);
      break;
    default:
      EXIT_WITH_MSG("Unexpected instruction for execute_fun_ptr!")

  }
}

ProgramState program_state(InstructionMemory *i_mem, Context **context,
    Stack *stack) {
  ProgramState state;
  state.i_mem = i_mem;
  state.context = context;
  state.stack = stack;

  return state;
}

