/*
 * program.c
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include "program.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "context.h"
#include "class.h"
#include "hashtable.h"
#include "queue.h"
#include "shared.h"
#include "instruction_table.h"

void goto_main(InstructionMemory *ins_mem, char ** ids) {
  ins_mem->ins[0].op = JUMP;
  ids[0] = NEW_ARRAY(ids[0], ID_SZ, char)
  strncpy(ids[0], "main", 4);
  if (0 == ins_mem->index) {
    ins_mem->index++;
  }
}

// Ex: class Doge:fields{age,breed},methods{new(2),speak(0)}
void read_class(char line[], InstructionMemory *ins_mem) {

  Composite *class = composite_class_load_src(line, ins_mem);

  instructions_insert_class(ins_mem, class);
  //composite_class_print_sumary(class);
}

void set_method_address(InstructionMemory *ins_mem, char fun_id[]) {
  char *start, *end;
  char class_name[ID_SZ], fun_name[ID_SZ];
  fun_name[0] = '\0';
  start = fun_id + 1;
  end = start;
  advance_to_next(&end, '_');
  fill_str(class_name, start, end);
  start = ++end;
  strcat(fun_name, start);
  //printf("%s.%s()\n", class_name, fun_name);

  Object class = instructions_get_class_by_id(ins_mem,
      instructions_get_class_by_name(ins_mem, class_name));

  Object *int_obj = NEW(int_obj, Object)
  int_obj->type = INTEGER;
  int_obj->int_value = ins_mem->index - 1;
  hashtable_insert(class.comp->methods, fun_name, int_obj);
}

int compile_jm(FILE *in, InstructionMemory *ins_mem, char **ids) {
  char line[MAX_LINE_LEN];
//char *pch;
  Word word;
  //char *str;
  int num_ins, adr;
  Instruction ins;

  Queue strs, ins_indices;
  queue_init(&strs);
  queue_init(&ins_indices);

// holds all indices of labels
  Hashtable *id_table = hashtable_create(TABLE_SZ);

  while (NULL != fgets(line, MAX_LINE_LEN, in)) {

//    printf("%s", line);
//    fflush(stdout);

    if (starts_with(line, "class")) {
      read_class(line, ins_mem);
      continue;
    }

    memset(&ins, 0, sizeof(Instruction));

    //pch = strtok(line, INS_DELIM);

    //NULL_CHECK(pch, "Invalid Instruction line!")
    if (!read_word_prog(&word, line, 0)) {
      continue;
    }

    while ('@' == word.word[0]) {
      //printf("!!! %s\n", pch);
      //fflush(stdout);
      if ('_' == word.word[1]) {
        set_method_address(ins_mem, word.word + 1);
        //printf("Function: '%s'\n", word.word + 1);
      } else {
        hashtable_insert(id_table, word.word + 1, (Object *) ins_mem->index);
        // in case the label is not on the same line as the first ins
      }
      if (!read_word_prog(&word, line, word.new_index)) {
        fgets(line, MAX_LINE_LEN, in);
        read_word_prog(&word, line, 0);
        //pch = strtok(line, INS_DELIM);
      }
    }

    ins.type = INTEGER;

    READ_INS(ins);
//    CHECK(read_word_prog(&word, line, word.new_index),
//        "Extra characters at end of instruction.")

    //printf("^%d %d\n", ins.op, index);
    //fflush(stdout);
    ins_mem->ins[ins_mem->index++] = ins;
  }

  num_ins = ins_mem->index;


  while (0 < strs.size) {
    char *string = queue_remove(&strs);
    int ins_index = (int) queue_remove(&ins_indices);
    int ins_ind;
//    printf("%d '%s'\n", ins_index, string);
//    fflush(stdout);
    ins_ind = ((strlen(string) + 1) / sizeof(Instruction))
        + (((strlen(string) + 1) % sizeof(Instruction) == 0) ? 0 : 1);
//    printf("%d %d %d\n", sizeof(Instruction), strlen(string), ins_ind);
    ins_mem->ins[ins_index].adr = ins_mem->index;
    strncpy((char *) (ins_mem->ins + ins_mem->index), string,
        strlen(string) + 1);
    ins_mem->ins[ins_index].adr = ins_mem->index;
    ins_mem->index += ins_ind;

    free(string);
  }

  int index;
  for (index = 0; index < num_ins; index++) {
//    printf("%d off %d\n", index, num_ins); fflush(stdout);
//    printf(">>>%s\n", INSTRUCTIONS(ins_mem->ins[index].op));
//    fflush(stdout);
    switch (ins_mem->ins[index].op) {
      case (JUMP):
      case (CALL):
      case (IFEQ):
      case (IF):
      case (IFN):
//        printf("%s\n", ids[index]);
//        fflush(stdout);
        adr = (int) hashtable_lookup(id_table, ids[index]);
        //printf("\t%d\n", adr);
        //fflush(stdout);
        CHECK(FAILURE == adr, "No known label.")
        ins_mem->ins[index].adr = adr - 1;
        free(ids[index]);
        break;
      case (CLSG):
        adr = instructions_get_class_by_name(ins_mem, ids[index]);
        CHECK(FAILURE == adr, "No known label.")
        ins_mem->ins[index].int_val = adr;
        free(ids[index]);
        break;
      default:
        break;
    }
//    printf("<<<END\n"); fflush(stdout);
  }
//  printf("A\n"); fflush(stdout);


  free(id_table);

  queue_deep_delete(&strs, free);
  queue_shallow_delete(&ins_indices);

  return num_ins;
}

int load_instructions(FILE *in, InstructionMemory *ins_mem) {
  char **ids = NEW_ARRAY(ids, 1, char *)

  goto_main(ins_mem, ids);

  int num_ins = compile_jm(in, ins_mem, ids);

  free(ids);

  return num_ins;
}

void read_class_bin(FILE *file, InstructionMemory *ins_mem) {
  Composite *class = composite_class_load_bin(file, ins_mem);
  instructions_insert_class(ins_mem, class);
// TODO
}

int load_bytecode(FILE *in, InstructionMemory *ins_mem) {
  int num_ins = 0;
  int size;

  while (TRUE) {
    if ('\0' == fgetc(in)) {
      break;
    }
    read_class_bin(in, ins_mem);
  }

  while (TRUE) {
    size = fread(&ins_mem->ins[num_ins++], sizeof(Instruction), 1, in);
    //printf("%d, Ins: %s\n", size, INSTRUCTIONS[ins_mem->ins[num_ins - 1].op]);
    //fflush(stdout);
    if (1 != size) {
      break;
    }
  }
  ins_mem->num_ins = num_ins;

  return num_ins;
}
