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
#include "hashtable.h"
#include "queue.h"
#include "shared.h"

int get_int(char *pch) {
  char *endptr;
  long val = strtol(pch, &endptr, 10);

  CHECK(
      (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0),
      "strtol error")
  CHECK(endptr == pch, "No digits were found")

  return (int) val;
}

double get_double(char *pch) {
  char *endptr;
  double val = strtod(pch, &endptr);

  CHECK(
      (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0),
      "strtol error")
  CHECK(endptr == pch, "No digits were found")

  return val;
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

void goto_main(InstructionMemory *ins_mem, char ** ids) {
  ins_mem->ins[0].op = JUMP;
  ids[0] = NEW_ARRAY(ids[0], ID_SZ, char)
  strncpy(ids[0], "main", 4);
  if (0 == ins_mem->index) {
    ins_mem->index++;
  }
}

int compile_jm(FILE *in, InstructionMemory *ins_mem, char **ids) {
  char line[MAX_LINE_LEN];
  //char *pch;
  Word word;
  char *str;
  int num_ins, adr;
  Instruction ins;

  Queue strs, ins_indices;
  queue_init(&strs);
  queue_init(&ins_indices);

  // holds all indices of labels
  Hashtable *id_table = create_hash_table(TABLE_SZ);

  while (NULL != fgets(line, MAX_LINE_LEN, in)) {

//    printf("%s", line);
//    fflush(stdout);

    memset(&ins, 0, sizeof(Instruction));

    //pch = strtok(line, INS_DELIM);

    //NULL_CHECK(pch, "Invalid Instruction line!")
    if (!read_word_prog(&word, line, 0)) {
      continue;
    }

    while ('@' == word.word[0]) {
      //printf("!!! %s\n", pch);
      //fflush(stdout);
      insert(id_table, word.word + 1, (Object *) ins_mem->index);
      // in case the label is not on the same line as the first ins
      if (!read_word_prog(&word, line, word.new_index)) {
        fgets(line, MAX_LINE_LEN, in);
        read_word_prog(&word, line, 0);
        //pch = strtok(line, INS_DELIM);
      }
    }

    ins.type = INTEGER;

    if (MATCHES(word.word, INSTRUCTIONS[EXIT])) {
      ins.op = EXIT;

    } else if (MATCHES(word.word, INSTRUCTIONS[PUSH])) {
      ins.op = PUSH;
      str = get_value(&ins, &word, line);
      if (STRING_INS == ins.type) {
        ins.op = PUSHM;
        queue_add_front(&strs, str);
        queue_add_front(&ins_indices, (int *) ins_mem->index);
//        printf("%d | \'%s\'\n", ins_mem->index, str);
//        printf("%s\n", "TEST");
//        fflush(stdout);
      }

    } else if (MATCHES(word.word, INSTRUCTIONS[PUSH])) {
      ins.op = POP;

    } else if (MATCHES(word.word, INSTRUCTIONS[ADD])) {
      ins.op = ADD;

    } else if (MATCHES(word.word, INSTRUCTIONS[SUB])) {
      ins.op = SUB;

    } else if (MATCHES(word.word, INSTRUCTIONS[MULT])) {
      ins.op = MULT;

    } else if (MATCHES(word.word, INSTRUCTIONS[DIV])) {
      ins.op = DIV;

    } else if (MATCHES(word.word, INSTRUCTIONS[MOD])) {
      ins.op = MOD;

    } else if (MATCHES(word.word, INSTRUCTIONS[AND])) {
      ins.op = AND;

    } else if (MATCHES(word.word, INSTRUCTIONS[OR])) {
      ins.op = OR;

    } else if (MATCHES(word.word, INSTRUCTIONS[XOR])) {
      ins.op = XOR;

    } else if (MATCHES(word.word, INSTRUCTIONS[IFEQ])) {
      ins.op = IFEQ;
      get_address(&word, line, &ins, &ids, ins_mem->index);

    } else if (MATCHES(word.word, INSTRUCTIONS[JUMP])) {
      ins.op = JUMP;
      get_address(&word, line, &ins, &ids, ins_mem->index);

    } else if (MATCHES(word.word, INSTRUCTIONS[PRINT])) {
      ins.op = PRINT;

    } else if (MATCHES(word.word, INSTRUCTIONS[PRINTN])) {
      ins.op = PRINTN;

    } else if (MATCHES(word.word, INSTRUCTIONS[DUP])) {
      ins.op = DUP;

    } else if (MATCHES(word.word, INSTRUCTIONS[SET])) {
      ins.op = SET;
      get_id(&word, line, ins.id);

    } else if (MATCHES(word.word, INSTRUCTIONS[GET])) {
      ins.op = GET;
      get_id(&word, line, ins.id);

    } else if (MATCHES(word.word, INSTRUCTIONS[OPEN])) {
      ins.op = OPEN;

    } else if (MATCHES(word.word, INSTRUCTIONS[CLOSE])) {
      ins.op = CLOSE;

    } else if (MATCHES(word.word, INSTRUCTIONS[CALL])) {
      ins.op = CALL;
      get_address(&word, line, &ins, &ids, ins_mem->index);
      //printf("TEST '%s'\n", ids[index]);

    } else if (MATCHES(word.word, INSTRUCTIONS[RET])) {
      ins.op = RET;

    } else if (MATCHES(word.word, INSTRUCTIONS[IF])) {
      ins.op = IF;
      get_address(&word, line, &ins, &ids, ins_mem->index);

    } else if (MATCHES(word.word, INSTRUCTIONS[EQ])) {
      ins.op = EQ;

    } else if (MATCHES(word.word, INSTRUCTIONS[LT])) {
      ins.op = LT;

    } else if (MATCHES(word.word, INSTRUCTIONS[LTE])) {
      ins.op = LTE;

    } else if (MATCHES(word.word, INSTRUCTIONS[GT])) {
      ins.op = GT;

    } else if (MATCHES(word.word, INSTRUCTIONS[GTE])) {
      ins.op = GTE;

    } else if (MATCHES(word.word, INSTRUCTIONS[NOT])) {
      ins.op = NOT;

    } else if (MATCHES(word.word, INSTRUCTIONS[TOI])) {
      ins.op = TOI;

    } else if (MATCHES(word.word, INSTRUCTIONS[TOF])) {
      ins.op = TOF;

    } else if (MATCHES(word.word, INSTRUCTIONS[NOP])) {
      ins.op = NOP;

    } else if (MATCHES(word.word, INSTRUCTIONS[IFN])) {
      ins.op = IFN;
      get_address(&word, line, &ins, &ids, ins_mem->index);

    } else if (MATCHES(word.word, INSTRUCTIONS[ANEW])) {
      ins.op = ANEW;

    } else if (MATCHES(word.word, INSTRUCTIONS[AADD])) {
      ins.op = AADD;

    } else if (MATCHES(word.word, INSTRUCTIONS[AGET])) {
      ins.op = AGET;

    } else if (MATCHES(word.word, INSTRUCTIONS[ASET])) {
      ins.op = ASET;

    } else if (MATCHES(word.word, INSTRUCTIONS[AENQ])) {
      ins.op = AENQ;

    } else if (MATCHES(word.word, INSTRUCTIONS[ADEQ])) {
      ins.op = ADEQ;

    } else if (MATCHES(word.word, INSTRUCTIONS[APUSH])) {
      ins.op = APUSH;

    } else if (MATCHES(word.word, INSTRUCTIONS[APOP])) {
      ins.op = APOP;

    } else if (MATCHES(word.word, INSTRUCTIONS[AINS])) {
      ins.op = AINS;

    } else if (MATCHES(word.word, INSTRUCTIONS[AREM])) {
      ins.op = AREM;

    } else if (MATCHES(word.word, INSTRUCTIONS[ALEN])) {
      ins.op = ALEN;

    } else if (MATCHES(word.word, INSTRUCTIONS[ALSH])) {
      ins.op = ALSH;

    } else if (MATCHES(word.word, INSTRUCTIONS[ARSH])) {
      ins.op = ARSH;

    } else if (MATCHES(word.word, INSTRUCTIONS[RSET])) {
      ins.op = RSET;

    } else if (MATCHES(word.word, INSTRUCTIONS[DREF])) {
      ins.op = DREF;

    } else if (MATCHES(word.word, INSTRUCTIONS[FLIP])) {
      ins.op = FLIP;

    } else if (MATCHES(word.word, INSTRUCTIONS[SWAP])) {
      ins.op = SWAP;

    } else {
      printf("%s\n", word.word);
      fflush(stdout);
      EXIT_WITH_MSG("Invalid op!")
    }

    CHECK(read_word_prog(&word, line, word.new_index),
        "Extra characters at end of instruction.")

    //printf("^%d %d\n", ins.op, index);
    //fflush(stdout);
    ins_mem->ins[ins_mem->index++] = ins;
  }

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

  num_ins = ins_mem->index;
  int index;
  for (index = 0; index < num_ins; index++) {
    //printf(">>>%d\n", index);
    //fflush(stdout);
    switch (ins_mem->ins[index].op) {
    case (JUMP):
    case (CALL):
    case (IFEQ):
    case (IF):
    case (IFN):
      //printf("%s\n", ids[index]);
      //fflush(stdout);
      adr = (int) get(id_table, ids[index]);
      //printf("\t%d\n", adr);
      //fflush(stdout);
      CHECK(FAILURE == adr, "No known label.")
      ins_mem->ins[index].adr = adr - 1;
      free(ids[index]);
      break;
    default:
      break;
    }
  }

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

int load_bytecode(FILE *in, InstructionMemory *ins_mem) {
  int num_ins = 0;
  int size;
  while (TRUE) {
    size = fread(&ins_mem->ins[num_ins++], sizeof(Instruction), 1, in);
    if (1 != size) {
      break;
    }
  }
  ins_mem->num_ins = num_ins;

  return num_ins;
}
