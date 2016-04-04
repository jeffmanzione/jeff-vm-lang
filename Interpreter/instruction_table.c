/*
 * instruction_table.c
 *
 *  Created on: Apr 3, 2016
 *      Author: Jeff
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "shared.h"
#include "program.h"
#include "context.h"
#include "hashtable.h"
#include "queue.h"
#include "instruction_table.h"

void register_instructions_here() {
  REGISTER(EXIT, "exit")
  REGISTER_Val(PUSH, "push")
  REGISTER_Val(PUSHM, "pushm")
  REGISTER(POP, "pop")
  REGISTER(ADD, "add")
  REGISTER(SUB, "sub")
  REGISTER(MULT, "mult")
  REGISTER(DIV, "div")
  REGISTER(MOD, "mod")
  REGISTER(AND, "and")
  REGISTER(OR, "or")
  REGISTER(XOR, "xor")
  REGISTER_Adr(IFEQ, "ifeq")
  REGISTER_Adr(JUMP, "jump")
  REGISTER(PRINT, "print")
  REGISTER(PRINTN, "printn")
  REGISTER(DUP, "dup")
  REGISTER_Id(SET, "set")
  REGISTER_Id(GET, "get")
  REGISTER(OPEN, "open")
  REGISTER(CLOSE, "close")
  REGISTER_Adr(CALL, "call")
  REGISTER(RET, "ret")
  REGISTER_Adr(IF, "if")
  REGISTER(EQ, "eq")
  REGISTER(LT, "lt")
  REGISTER(LTE, "lte")
  REGISTER(GT, "gt")
  REGISTER(GTE, "gte")
  REGISTER(NOT, "not")
  REGISTER(TOI, "toi")
  REGISTER(TOF, "tof")
  REGISTER(NOP, "nop")
  REGISTER_Adr(IFN, "ifn")
  REGISTER(ANEW, "anew")
  REGISTER(AADD, "aadd")
  REGISTER(AGET, "aget")
  REGISTER(ASET, "aset")
  REGISTER(AENQ, "aenq")
  REGISTER(ADEQ, "adeq")
  REGISTER(APUSH, "apush")
  REGISTER(APOP, "apop")
  REGISTER(AINS, "ains")
  REGISTER(AREM, "arem")
  REGISTER(ALEN, "alen")
  REGISTER(ALSH, "alsh")
  REGISTER(ARSH, "arsh")
  REGISTER(RSET, "rset")
  REGISTER(DREF, "dref")
  REGISTER(FLIP, "flip")
  REGISTER(SWAP, "swap")
  REGISTER_Id(OGET, "oget")
  REGISTER_Id(OCALL, "ocall")
  REGISTER(ORET, "oret")
  REGISTER(ONEW, "onew")
  REGISTER_Id(SCALL, "scall")
  REGISTER_Adr(CLSG, "clsg")
  REGISTER(IS, "is")
  REGISTER(HASH, "hash")
  REGISTER(ISI, "isi")
  REGISTER(ISF, "isf")
  REGISTER(ISC, "isc")
  REGISTER(ISO, "iso")
  REGISTER(ISA, "isa")
}

int read_word_prog(Word *word, char line[], int index) {
  int word_len = 0;
  int start_index = 0, end_index;

  if (COMMENT_CH == line[index] || '\0' == line[index] || '\n' == line[index]) {
    word->word[0] = '\0';
    word->word_len = 0;
    return FALSE;
  }

  while (is_whitespace(line[index])) {
    index++;
    start_index++;
  }

  if (COMMENT_CH == line[index] || '\0' == line[index] || '\n' == line[index]) {
    word->word[0] = '\0';
    word_len = 0;
    return FALSE;
  }

  start_index = index;

  if ('\'' == line[index]) {
    index++;
    while ('\'' != line[index]) {
      index++;
    }
    end_index = index + 1;

  } else {
    while (!is_whitespace(line[index]) && '\n' != line[index]
        && '\0' != line[index] && COMMENT_CH != line[index]) {
      index++;
    }
    end_index = index;
  }

  word->word_len = word_len = end_index - start_index;

  strncpy(word->word, line + start_index, word_len);
  word->word[word_len] = '\0';
  word->new_index = end_index;

  return TRUE;

}

int64_t get_int(char *pch) {
  char *endptr;
  int64_t val = strtoll(pch, &endptr, 10);

  CHECK(
      (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0),
      "strtol error")
  CHECK(endptr == pch, "No digits were found")

  return val;
}

float96_t get_double(char *pch) {
  char *endptr;
  float96_t val = strtold(pch, &endptr);

  CHECK(
      (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0),
      "strtol error")
  CHECK(endptr == pch, "No digits were found")

  return val;
}

char *get_string(char *pch) {
  char tmp[4096], *result;
  int word_i = 0;
  pch++;
  while ('\'' != *pch) {
    tmp[word_i++] = *pch;
    pch++;
  }
  tmp[word_i] = '\0';

  result = NEW_ARRAY(result, word_i + 1, char)
  strncpy(result, tmp, word_i);

  return result;
}

char *get_value(Instruction *ins, Word *word, char line[]) {
  CHECK(!read_word_prog(word, line, word->new_index),
      "Instruction lacks argument.");

  if ('N' == word->word[0]) {
    ins->type = NONE;
  } else if ('\'' == word->word[0]) {
    ins->type = STRING_INS;
    return get_string(word->word);
  } else if (ends_with(word->word, "f") || contains_char(word->word, '.')) {
    ins->float_val = get_double(word->word);
    ins->type = FLOATING;
  } else {
    ins->int_val = get_int(word->word);
    ins->type = INTEGER;
  }

  return NULL;
}

void get_id(Word *word, char line[], char *dest) {
  CHECK(!read_word_prog(word, line, word->new_index),
      "Invalid line. Expected reference.")
  strncpy(dest, word->word, ID_SZ);
}

void get_address(Word *word, char line[], Instruction *ins, char ***ids,
    int index) {
  long val;
  //ADVANCE(pch);

  //printf("ZZZ %d %s\n", index, pch);
  //fflush(stdout);
  CHECK(!read_word_prog(word, line, word->new_index),
      "Instruction lacks argument.");
  //printf("BEF %p\n", *ids);
  *ids = RENEW(*ids, index + 1, char *)
  (*ids)[index] = NEW_ARRAY((*ids)[index], ID_SZ, char)
  if ('0' <= word->word[0] && '9' >= word->word[0]) {
    // is number
    val = strtol(word->word, NULL, 10);
    ins->adr = (int) val;
  } else {
    ins->adr = -1;
    strncpy((*ids)[index], word->word, ID_SZ);
  }
  //printf("AFT %p\n", *ids);
}

void it_init() {
  global_it.table_size = INITIAL_IT_SIZE;
  global_it.num_ins = 0;
  global_it.instruction_ids = NEW_ARRAY(global_it.instruction_ids,
      global_it.table_size, InsEntry)
  global_it.ins_ht = hashtable_create(TABLE_SZ * 8);
  register_instructions_here();

}

void it_finalize() {
  global_it.table_size = INITIAL_IT_SIZE;
  global_it.num_ins = 0;
  free(global_it.instruction_ids);
  hashtable_free(global_it.ins_ht, do_nothing);
}

void it_read_instruction(Word word, char line[], Instruction *ins, char ***ids,
    int index, Queue *strs, Queue *ins_indices) {
  InsEntry *entry = (InsEntry *) hashtable_lookup(global_it.ins_ht, word.word);
  NULL_CHECK(entry,
      "Attempted to read instruction name, but was not registered.")

  ins->op = entry->op;
  char *str;
  switch (entry->type) {
    case I_VAL:
      str = get_value(ins, &word, line);
      if (STRING_INS == ins->type) {
        ins->op = PUSHM;
        queue_add_front(strs, str);
        queue_add_front(ins_indices, (int *) index);
      }
      break;
    case I_ADR:
      get_address(&word, line, ins, ids, index);
      break;
    case I_ID:
      get_id(&word, line, ins->id);
      break;
    case I_NONE:
    default:
      break;
  }

}
