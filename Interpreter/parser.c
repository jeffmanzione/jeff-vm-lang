/*
 * parser.c
 *
 *  Created on: Jan 9, 2016
 *      Author: Jeff
 */

#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "hashtable.h"
#include "shared.h"
#include "tokenizer.h"

Hashtable *fun_names;
Hashtable *classes;
FILE *top;

void remove_if_present(Queue *queue, TokenType type) {
  Token *tok;
  NULL_CHECK(queue, "Queue was NULL!");
  if (0 == queue->size) {
    return;
  }

  tok = queue_peek(queue);
  if (type == tok->type) {
    queue_remove(queue);
    free(tok);
  }
}

int nextIsWordAndRemove(Queue *queue, char word[]) {
  Token *tok;
  NULL_CHECK(queue, "Queue was NULL!");
  if (1 > queue->size) {
    return FALSE;
  }

  tok = queue_peek(queue);

  if (WORD != tok->type || !MATCHES(word, tok->text)) {
    return FALSE;
  }

  queue_remove(queue);
  free(tok);
  return TRUE;
}

int nextIsAndRemove(Queue *queue, TokenType type) {
  Token *tok;
  NULL_CHECK(queue, "Queue was NULL!");
  if (1 > queue->size) {
    return FALSE;
  }
  tok = queue_peek(queue);

  if (type != tok->type) {
    return FALSE;
  }

  queue_remove(queue);
  free(tok);
  return TRUE;
}

int nextTwoAreAndRemove(Queue *queue, TokenType first, TokenType second) {
  Token *tok_1, *tok_2;
  NULL_CHECK(queue, "Queue was NULL!");
  if (2 > queue->size) {
    return FALSE;
  }

  tok_1 = queue_peek(queue);
  if (first != tok_1->type) {
    return FALSE;
  }

  queue_remove(queue);

  tok_2 = queue_peek(queue);
  if (second != tok_2->type) {
    queue_add_front(queue, tok_1);
    return FALSE;
  }

  queue_remove(queue);
  free(tok_1);
  free(tok_2);

  return TRUE;
}

int nextThreeAreAndRemove(Queue *queue, TokenType first, TokenType second,
    TokenType third) {
  Token *tok_1, *tok_2, *tok_3;
  NULL_CHECK(queue, "Queue was NULL!");
  if (3 > queue->size) {
    return FALSE;
  }

  tok_1 = queue_peek(queue);
  if (first != tok_1->type) {
    return FALSE;
  }

  queue_remove(queue);

  tok_2 = queue_peek(queue);
  if (second != tok_2->type) {
    queue_add_front(queue, tok_1);
    return FALSE;
  }

  queue_remove(queue);

  tok_3 = queue_peek(queue);
  if (third != tok_3->type) {
    queue_add_front(queue, tok_2);
    queue_add_front(queue, tok_1);
    return FALSE;
  }

  queue_remove(queue);
  free(tok_1);
  free(tok_2);
  free(tok_3);
  return TRUE;
}

int nextIsAndSecondIsntAndRemove(Queue *queue, TokenType first,
    TokenType notSecond) {
  Token *tok_1, *tok_2;
  NULL_CHECK(queue, "Queue was NULL!");
  if (1 > queue->size) {
    return FALSE;
  }

  tok_1 = queue_peek(queue);
  if (first != tok_1->type) {
    return FALSE;
  }

  queue_remove(queue);

  if (0 == queue->size) {
    return TRUE;
  }

  tok_2 = queue_peek(queue);
  if (notSecond == tok_2->type) {
    queue_add_front(queue, tok_1);
    return FALSE;
  }

  free(tok_1);

  return TRUE;
}

char *KEYWORDS[] = { DEF_KEYWORD, FUN_KEYWORD, IF_KEYWORD, ELSE_KEYWORD,
WHILE_KEYWORD, FOR_KEYWORD, BREAK_KEYWORD, RETURN_KEYWORD, AS_KEYWORD,
NONE_KEYWORD, TRUE_KEYWORD, FALSE_KEYWORD, FUN_KEYWORD, CLASS_KEYWORD,
FIELD_KEYWORD, IMPORT_KEYWORD, NEW_KEYWORD };

int is_keyword(const char word[]) {
  int i;
  for (i = 0; i < sizeof(KEYWORDS) / sizeof(char *); i++) {
    if (MATCHES(word, KEYWORDS[i])) {
      return TRUE;
    }
  }
  return FALSE;
}

void write_classes_and_del(void *comp_obj) {
  composite_class_save_src(top, ((Object *) comp_obj)->comp);
  fprintf(top, "\n");
  // TODO
  //object_delete(comp_obj);
}

void parse(Queue *queue, FILE *out) {
  // printf("parse()\n"); fflush(stdout);
  top = out;
  fun_names = create_hash_table(TABLE_SZ);
  classes = create_hash_table(TABLE_SZ);

  FILE *tmp = tmpfile();

  parse_top_level(queue, tmp);

  free_table(fun_names, do_nothing);
  free_table(classes, write_classes_and_del);
  fprintf(out, "\n");

  append(out, tmp);
}

void parse_top_level(Queue *queue, FILE *out) {
  // printf("parse_top_level()\n"); fflush(stdout);

  while (0 < queue->size) {
    remove_if_present(queue, ENDLINE);
    if (0 < queue->size) {
      parse_elements(queue, out);
    } else {
      return;
    }
  }
}

void parse_elements(Queue *queue, FILE *out) {
  Token *tok = queue_peek(queue);
  remove_if_present(queue, ENDLINE);

  if (nextIsWordAndRemove(queue, CLASS_KEYWORD)) {
    parse_class(queue, out);
  } else if (nextIsWordAndRemove(queue, DEF_KEYWORD)
      || nextIsWordAndRemove(queue, FUN_KEYWORD)) {
    parse_fun(queue, out);
  } else if (nextIsWordAndRemove(queue, IMPORT_KEYWORD)) {
    char fn[MAX_LINE_LEN];
    Token fn_token = *((Token *) queue_peek(queue));
    Queue child_queue;
    FILE *child_file;
    fn[0] = '\0';
    CHECK(!nextIsAndRemove(queue, STR),
        "Expected string after keyword "IMPORT_KEYWORD".")

    strncpy(fn, fn_token.text + 1, strlen(fn_token.text) - 2);
    fn[strlen(fn_token.text) - 2] = '\0';
    child_file = fopen(fn, "r");
    NULL_CHECK(child_file, "Could not find imported source!")
    //printf("Import file >%s<\n", fn);
    //fflush(stdout);

    queue_init(&child_queue);

    tokenize(child_file, &child_queue);
    parse_top_level(&child_queue, out);
    fclose(child_file);
  } else {
    printf("WAS %d\nLine %d(%d)\n", tok->type, tok->line, tok->col);
    fflush(stdout);
    EXIT_WITH_MSG("Expected "DEF_KEYWORD" or "CLASS_KEYWORD".")
  }
}

void parse_class(Queue *queue, FILE *out) {
  Token class_name;
  Composite *class;
  Object *class_obj;

  class_name = *((Token *) queue_peek(queue));

  CHECK(!nextIsAndRemove(queue, WORD), "Expected class name.");

  class = composite_class_new(class_name.text);
  class_obj = NEW(class_obj, Object)
  class_obj->type = COMPOSITE;
  class_obj->comp = class;

  parse_class_body(queue, out, class, class_name.text);
  //composite_class_print_sumary(class);
  insert(classes, class_name.text, class_obj);
}

void parse_class_body(Queue *queue, FILE *out, Composite *class,
    const char class_name[]) {
  remove_if_present(queue, ENDLINE);

  if (!nextIsAndRemove(queue, LBRCE)) {
    parse_class_item(queue, out, class, class_name);
    return;
  }

  remove_if_present(queue, ENDLINE);

  while (RBRCE != ((Token *) queue_peek(queue))->type) {
    parse_class_item(queue, out, class, class_name);
    remove_if_present(queue, ENDLINE);
  }

  remove_if_present(queue, ENDLINE);
  CHECK(!nextIsAndRemove(queue, RBRCE), "Expected }");
}

void parse_class_item(Queue *queue, FILE *out, Composite *class,
    const char class_name[]) {
  Token *tok = queue_peek(queue);
  if (nextIsWordAndRemove(queue, FIELD_KEYWORD)) {
    parse_class_field(queue, out, class, class_name);
  } else if (nextIsWordAndRemove(queue, DEF_KEYWORD)
      || nextIsWordAndRemove(queue, FUN_KEYWORD)) {
    parse_class_method(queue, out, class, class_name);
  } else {
    printf("WAS %d\nLine %d(%d)\n", tok->type, tok->line, tok->col);
    fflush(stdout);
    EXIT_WITH_MSG("Expected "DEF_KEYWORD" or "FIELD_KEYWORD".")
  }
}

void parse_class_field(Queue *queue, FILE *out, Composite *class,
    const char class_name[]) {
  Token *field_name;
  do {
    field_name = queue_remove(queue);
    NULL_CHECK(field_name, "Unexpected end of file!");

    composite_class_add_field(class, field_name->text);
    free(field_name);

  } while (nextIsAndRemove(queue, COMMA));
}

void parse_class_method(Queue *queue, FILE *out, Composite *class,
    const char class_name[]) {
  Token def_name;
  int num_args;
  def_name = *((Token *) queue_peek(queue));
  char label[ID_SZ];
  label[0] = '\0';

  CHECK(!nextIsAndRemove(queue, WORD), "Expected fun name.");

  remove_if_present(queue, ENDLINE);

  method_to_label(class_name, def_name.text, label);

  insert(fun_names, label, (Object *) sizeof(Object));
  write_label(label, out);

  num_args = parse_fun_arguments(queue, out);

  composite_class_add_method(class, def_name.text, num_args);

  remove_if_present(queue, ENDLINE);

  parse_body(queue, out);

  if (MATCHES(NEW_KEYWORD, def_name.text)) {
    write_ins_id(GET, "self", out);
  }

  write_ins_default(RET, out);

  fprintf(out, "\n");
  remove_if_present(queue, ENDLINE);
}

void parse_fun(Queue *queue, FILE *out) {
// printf("parse_fun()\n"); fflush(stdout);
  Token tok, def_name;

  def_name = *((Token *) queue_peek(queue));

  CHECK(!nextIsAndRemove(queue, WORD), "Expected fun name.");

  insert(fun_names, def_name.text, (Object *) sizeof(Object));

  remove_if_present(queue, ENDLINE);

// if the following is true, the this is a function predef.
  tok = *((Token *) queue_peek(queue));
  if (WORD == tok.type
      && (MATCHES(tok.text, DEF_KEYWORD) || MATCHES(tok.text, FUN_KEYWORD)
          || MATCHES(tok.text, CLASS_KEYWORD))) {
    //printf("TOK '%s'\n", tok->text);
    return;
  }

  write_label(def_name.text, out);

  parse_fun_arguments(queue, out);

  remove_if_present(queue, ENDLINE);

  parse_body(queue, out);

  if (0 == strcmp(MAIN_FUNCTION, def_name.text)) {
    write_ins_value(PUSH, 1, out);
    write_ins_default(EXIT, out);
  } else {
    write_ins_default(RET, out);
  }
  fprintf(out, "\n");
  remove_if_present(queue, ENDLINE);
}

int parse_fun_arguments_helper(Queue *queue, FILE *out) {
/// printf("parse_fun_arguments_helper()\n"); fflush(stdout);
  Token tok_id = *((Token *) queue_peek(queue));
  int num_args = 1;
  if (RPAREN == tok_id.type) {
    return 0;
  }
//printf("ARG NAME = %s\n", tok->text);
  CHECK(!nextIsAndRemove(queue, WORD), "Expected arg name.");

  if (nextIsAndRemove(queue, COMMA)) {
    num_args = 1 + parse_fun_arguments_helper(queue, out);
  }

  write_ins_id(SET, tok_id.text, out);

  return num_args;
}

int parse_fun_arguments(Queue *queue, FILE *out) {
// printf("parse_fun_arguments()\n"); fflush(stdout);
  int num_args;
  if (!nextIsAndRemove(queue, LPAREN)) {
    return 0;
  }

  num_args = parse_fun_arguments_helper(queue, out);
  CHECK(!nextIsAndRemove(queue, RPAREN), "Expected )");
  return num_args;
}

void parse_body(Queue *queue, FILE *out) {
//  printf("parse_body()\n"); fflush(stdout);
  remove_if_present(queue, ENDLINE);

  if (!nextIsAndRemove(queue, LBRCE)) {
    parse_line(queue, out);
    return;
  }

  remove_if_present(queue, ENDLINE);

  while (RBRCE != ((Token *) queue_peek(queue))->type) {
    parse_line(queue, out);
  }

  remove_if_present(queue, ENDLINE);
  CHECK(!nextIsAndRemove(queue, RBRCE), "Expected }");
}

void parse_line(Queue *queue, FILE *out) {
//  printf("parse_line()\n"); fflush(stdout);

  parse_exp(queue, out);

  remove_if_present(queue, ENDLINE);

//else if (RBRCE != tok->type) {
//  EXIT_WITH_MSG("Expected ENDLINE or } at end of line.");
//}
}

void parse_exp(Queue *queue, FILE *out) {
// printf("parse_exp()\n"); fflush(stdout);
  parse_exp_tuple(queue, out);
}

int parse_exp_tuple(Queue *queue, FILE *out) {
// printf("parse_exp_tuple()\n"); fflush(stdout);
  parse_exp_for(queue, out);

  if (!nextIsAndRemove(queue, COMMA)) {
    return 1;
  }
  return 1 + parse_exp_tuple(queue, out);
}

void parse_exp_for(Queue *queue, FILE *out) {
// printf("parse_exp_for()\n"); fflush(stdout);
  int num = queue->size;

  if (!nextIsWordAndRemove(queue, FOR_KEYWORD)) {
    parse_exp_while(queue, out);
    return;
  }

//write_ins_default(OPEN, out);

  FILE *cond, *aft;
  void parse_for_args() {
    cond = tmpfile();
    aft = tmpfile();

    parse_exp_for(queue, out);

    if (!nextIsAndRemove(queue, COMMA)) {
      EXIT_WITH_MSG("Missing first ',' in for initializer.");
    }
    parse_exp_for(queue, cond);

    if (!nextIsAndRemove(queue, COMMA)) {
      EXIT_WITH_MSG("Missing second ',' comma in for initializer.");
    }
    parse_exp_for(queue, aft);
  }

  if (nextIsAndRemove(queue, LPAREN)) {
    parse_for_args();

    if (!nextIsAndRemove(queue, RPAREN)) {
      EXIT_WITH_MSG("Expected ')' after for initializer.");
    }
  } else {
    parse_for_args();
  }

  write_label_num("for", num, out);

  append(out, cond);
  fclose(cond);

  write_ins_id_num(IFN, "end", num, out);

  remove_if_present(queue, ENDLINE);

  parse_body(queue, out);

  remove_if_present(queue, ENDLINE);

  append(out, aft);
  fclose(aft);

  write_ins_id_num(JUMP, "for", num, out);
  write_label_num("end", num, out);
//write_ins_default(CLOSE, out);

  remove_if_present(queue, ENDLINE);
}

void parse_exp_while(Queue *queue, FILE *out) {
// printf("parse_exp_while()\n"); fflush(stdout);

  int num = queue->size;
  if (!nextIsWordAndRemove(queue, WHILE_KEYWORD)) {
    parse_exp_if(queue, out);
    return;
  }

//write_ins_default(OPEN, out);

  write_label_num("while", num, out);

  parse_exp(queue, out);

  write_ins_id_num(IFN, "end", num, out);

  remove_if_present(queue, ENDLINE);

  parse_body(queue, out);

  write_ins_id_num(JUMP, "while", num, out);

  write_label_num("end", num, out);

//write_ins_default(CLOSE, out);

  remove_if_present(queue, ENDLINE);
}

void parse_exp_if(Queue *queue, FILE *out) {
//  printf("parse_exp_if()\n"); fflush(stdout);
  int num = queue->size;
  if (!nextIsWordAndRemove(queue, IF_KEYWORD)) {
    parse_exp_assign(queue, out);
    return;
  }

//write_ins_default(OPEN, out);

  parse_exp_if(queue, out);

  write_ins_id_num(IFN, "else", num, out);

  remove_if_present(queue, ENDLINE);

  parse_body(queue, out);

  write_ins_id_num(JUMP, "end", num, out);

  write_label_num("else", num, out);

  remove_if_present(queue, ENDLINE);

  if (nextIsWordAndRemove(queue, ELSE_KEYWORD)) {
    remove_if_present(queue, ENDLINE);
    parse_body(queue, out);
  }

  remove_if_present(queue, ENDLINE);

  write_label_num("end", num, out);
//write_ins_default(CLOSE, out);
}

//void parse_exp_assign(Queue *queue, FILE *out) {
//  // printf("parse_exp_assign()\n"); fflush(stdout);
//  Token tok_first = *((Token *) queue_peek(queue)), tok_second, tok_third;
//
//  if (!is_keyword(tok_first.text) && nextIsAndRemove(queue, WORD)) {
//
//    tok_second = *((Token *) queue_peek(queue));
//    if (WORD == tok_second.type && !is_keyword(tok_second.text)) {
//      parse_exp_assign(queue, out);
//      goto set_id;
//    } else {
////      if (nextIsAndRemove(queue, LBRAC)) {
////        write_ins_id(GET, tok_first->text, out);
////
////        parse_exp(queue, out);
////
////        CHECK(!nextIsAndRemove(queue, RBRAC), "Expected ].");
////
////        if (!nextIsAndSecondIsntAndRemove(queue, EQUALS, EQUALS)) {
////          write_ins_default(AGET, out);
////          return;
////        }
////
////        parse_exp(queue, out);
////
////        write_ins_default(ASET, out);
////        return;
////      }
//
//      if (!nextIsAndRemove(queue, EQUALS)) {
//        queue_add_front(queue, token_copy(tok_first));
//        parse_exp_assign_array(queue, out);
//        return;
//      }
//
//      tok_third = *((Token *) queue_peek(queue));
//
//      if (EQUALS == tok_third.type || GTHAN == tok_third.type) {
//        queue_add_front(queue, token_copy(tok_second));
//        queue_add_front(queue, token_copy(tok_first));
//        parse_exp_assign_array(queue, out);
//        return;
//      }
//
//    }
//  } else {
//    parse_exp_assign_array(queue, out);
//    return;
//  }
//
//  parse_exp_assign(queue, out);
//
//  set_id: write_ins_id(SET, tok_first.text, out);
//}

void parse_exp_assign_multi_lhs(Queue *queue, FILE *out) {
// recurse
  Token var_name = *((Token *) queue_peek(queue));
  if (nextTwoAreAndRemove(queue, WORD, COMMA)) {
    parse_exp_assign_multi_lhs(queue, out);
    write_ins_id(SET, var_name.text, out);
  } else if (nextTwoAreAndRemove(queue, WORD, RBRCE)) {
    write_ins_id(SET, var_name.text, out);
  } else {
    parse_exp_assign_array(queue, out);
    write_ins_default(FLIP, out);
    write_ins_default(RSET, out);
    if (nextIsAndRemove(queue, COMMA)) {
      parse_exp_assign_multi_lhs(queue, out);
    } else {
      CHECK(!nextIsAndRemove(queue, RBRCE), "Expected } to close LHS assign")
    }
  }

}

void parse_exp_assign(Queue *queue, FILE *out) {
// printf("parse_exp_assign()\n"); fflush(stdout);
  Token var_name = *((Token *) queue_peek(queue));

  if (nextIsAndRemove(queue, LBRCE)) {
    FILE *hold = tmpfile();

    parse_exp_assign_multi_lhs(queue, hold);

    CHECK(!nextIsAndRemove(queue, EQUALS), "Expected = after LHS assign")

    parse_exp_assign(queue, out);

    append(out, hold);
    fclose(hold);

    return;
  }

  if (nextIsAndRemove(queue, WORD)) {

    if (!nextIsAndRemove(queue, EQUALS)) {
      queue_add_front(queue, token_copy(var_name));
    } else {
      parse_exp_assign(queue, out);
      write_ins_id(SET, var_name.text, out);
      return;
    }
  }

  parse_exp_assign_array(queue, out);

  if (!nextIsAndRemove(queue, EQUALS)) {
    return;
  }

  parse_exp_assign(queue, out);

  write_ins_default(RSET, out);

}

void parse_exp_assign_array(Queue *queue, FILE *out) {
// printf("parse_exp_assign_array()"); fflush(stdout);
  FILE *hold = tmpfile();
  parse_exp_or(queue, hold);

  if (nextTwoAreAndRemove(queue, LARROW, GTHAN)) {
    append(out, hold);
    parse_exp_or(queue, out);
    write_ins_default(SWAP, out);

  } else if (nextThreeAreAndRemove(queue, LTHAN, EQUALS, COLON)) {
    append(out, hold);
    parse_exp_or(queue, out);
    write_ins_default(APOP, out);
    write_ins_default(RSET, out);

  } else if (nextIsAndRemove(queue, LARROW)) {
    append(out, hold);

    parse_exp_or(queue, out);
    CHECK(!nextIsAndRemove(queue, COLON), "Expected : after array in <- exp.")
    parse_exp(queue, out);

    write_ins_default(AREM, out);
    write_ins_default(RSET, out);

  } else if (nextThreeAreAndRemove(queue, EQUALS, GTHAN, COLON)) {
    parse_exp_or(queue, out);

    append(out, hold);

    write_ins_default(APUSH, out);

  } else if (nextIsAndRemove(queue, RARROW)) {
    parse_exp_or(queue, out);

    CHECK(!nextIsAndRemove(queue, COLON), "Expected : after array in -> exp.")
    parse_exp(queue, out);

    append(out, hold);

    write_ins_default(AINS, out);

  } else if (nextThreeAreAndRemove(queue, COLON, LTHAN, EQUALS)) {
    append(out, hold);

    parse_exp_or(queue, out);
    write_ins_default(AENQ, out);

  } else if (nextThreeAreAndRemove(queue, COLON, EQUALS, GTHAN)) {
    append(out, hold);
    Token var_name = *((Token *) queue_peek(queue));
    CHECK(!nextIsAndRemove(queue, WORD), "Expected var name after :=>")
    write_ins_default(ADEQ, out);
    write_ins_id(SET, var_name.text, out);

  } else if (nextIsAndRemove(queue, COLON)) {
    append(out, hold);
    parse_exp_or(queue, out);

    if (nextIsAndRemove(queue, RARROW)) {
      parse_exp(queue, out);

      write_ins_default(AREM, out);
      write_ins_default(RSET, out);

    } else if (nextIsAndRemove(queue, LARROW)) {
      parse_exp(queue, out);

      write_ins_default(AINS, out);

    } else {
      printf("Was %d\n", ((Token *) queue_peek(queue))->line);
      EXIT_WITH_MSG("Expected ->, <-, or = after a:x expression")
    }

  } else {
    append(out, hold);
  }

  fclose(hold);
}

void parse_exp_or(Queue *queue, FILE *out) {
//printf("parse_exp_or()\n"); fflush(stdout);
  parse_exp_xor(queue, out);

  Token tok = *((Token *) queue_peek(queue));

  if (!nextIsAndRemove(queue, PIPE)) {
    return;
  }

  Token *next = queue_peek(queue);

  if (RPAREN == next->type || RBRAC == next->type || RBRCE == next->type
      || COMMA == next->type || PLUS == next->type || MINUS == next->type
      || STAR == next->type || FSLASH == next->type) {
    queue_add_front(queue, token_copy(tok));
    return;
  }

  parse_exp_or(queue, out);

  write_ins_default(OR, out);

}

void parse_exp_xor(Queue *queue, FILE *out) {
//printf("parse_exp_xor()\n"); fflush(stdout);
  parse_exp_and(queue, out);

  if (!nextIsAndRemove(queue, CARET)) {
    return;
  }

  parse_exp_xor(queue, out);

  write_ins_default(XOR, out);
}

void parse_exp_and(Queue *queue, FILE *out) {
//printf("parse_exp_and()\n"); fflush(stdout);
  parse_exp_eq(queue, out);

  if (!nextIsAndRemove(queue, AMPER)) {
    return;
  }

  parse_exp_and(queue, out);

  write_ins_default(AND, out);

}

void parse_exp_eq(Queue *queue, FILE *out) {
//printf("parse_exp_eq()\n"); fflush(stdout);
  parse_exp_lt_gt(queue, out);

  if (!nextIsAndRemove(queue, EQUIV)) {
    return;
  }

  parse_exp_eq(queue, out);

  write_ins_default(EQ, out);
}

void parse_exp_lt_gt(Queue *queue, FILE *out) {
//printf("parse_exp_lt_gt()\n"); fflush(stdout);
  Token tok_first;
  Op op;
  parse_exp_add_sub(queue, out);

  tok_first = *((Token*) queue_peek(queue));

  switch (tok_first.type) {
    case (LTHAN):
      op = LT;
      break;
    case (GTHAN):
      op = GT;
      break;
    case (LTHANEQ):
      op = LTE;
      break;
    case (GTHANEQ):
      op = GTE;
      break;
    default:
      return;
  }

  queue_remove(queue);

  parse_exp_lt_gt(queue, out);

  write_ins_default(op, out);

}

void parse_exp_add_sub(Queue *queue, FILE *out) {
//printf("parse_exp_add_sub()\n"); fflush(stdout);
  Token tok, *tmp, next;
  parse_exp_mult_div(queue, out);

  tok = *((Token *) queue_peek(queue));

  switch (tok.type) {
    case (PLUS):
    case (MINUS):
      tmp = queue_remove(queue);
      next = *((Token *) queue_peek(queue));
      if (GTHAN == next.type) {
        queue_add_front(queue, tmp);
        return;
      }
      free(tmp);
      break;
    default:
      return;
  }

  parse_exp_add_sub(queue, out);

  switch (tok.type) {
    case (PLUS):
      write_ins_default(ADD, out);
      break;
    case (MINUS):
      write_ins_default(SUB, out);
      break;
    default:
      return;
  }
}

void parse_exp_mult_div(Queue *queue, FILE *out) {
//printf("parse_exp_mult_div()\n"); fflush(stdout);
  Token tok;
  parse_exp_array_transfer(queue, out);

  tok = *((Token *) queue_peek(queue));

  switch (tok.type) {
    case (STAR):
    case (FSLASH):
    case (PERCENT):
      free(queue_remove(queue));
      break;
    default:
      return;
  }

  parse_exp_mult_div(queue, out);

  switch (tok.type) {
    case (STAR):
      write_ins_default(MULT, out);
      break;
    case (FSLASH):
      write_ins_default(DIV, out);
      break;
    case (PERCENT):
      write_ins_default(MOD, out);
      break;
    default:
      return;
  }
}

void parse_exp_array_transfer(Queue *queue, FILE *out) {
//printf("parse_exp_mult_div()\n"); fflush(stdout);
  Token tok_1 = *((Token *) queue_peek(queue)), tok_2;

  if (nextIsAndRemove(queue, WORD)) {

    if (nextThreeAreAndRemove(queue, LTHAN, LTHAN, COLON)) {
      tok_2 = *((Token *) queue_peek(queue));

      CHECK(!nextIsAndRemove(queue, WORD), "Shift must be used with arrays.")

      write_ins_id(GET, tok_1.text, out);
      write_ins_id(GET, tok_2.text, out);
      write_ins_default(ALSH, out);

    } else if (nextThreeAreAndRemove(queue, COLON, GTHAN, GTHAN)) {
      tok_2 = *((Token *) queue_peek(queue));

      CHECK(!nextIsAndRemove(queue, WORD), "Shift must be used with arrays.")

      write_ins_id(GET, tok_1.text, out);
      write_ins_id(GET, tok_2.text, out);
      write_ins_default(ARSH, out);
    } else {
      queue_add_front(queue,
          token_create(tok_1.type, tok_1.line, tok_1.col, tok_1.text));
      parse_exp_casting(queue, out);
    }
  } else {
    parse_exp_casting(queue, out);
  }

}

void parse_exp_casting(Queue *queue, FILE *out) {
//printf("parse_exp_casting()\n"); fflush(stdout);
  Token tok;
  parse_exp_unary(queue, out);

  tok = *((Token *) queue_peek(queue));

  if (WORD != tok.type || !MATCHES(AS_KEYWORD, tok.text)) {
    return;
  }

  free(queue_remove(queue));

  tok = *((Token *) queue_peek(queue));

  CHECK(!nextIsAndRemove(queue, WORD), "Expect type to cast.")

  if (MATCHES(TYPE_INT_KEYWORD, tok.text)) {
    write_ins_default(TOI, out);
  } else if (MATCHES(TYPE_FLOAT_KEYWORD, tok.text)) {
    write_ins_default(TOF, out);
  } else {
    EXIT_WITH_MSG("Expect type to cast. (2)");
  }

}

//void parse_exp_unary(Queue *queue, FILE *out) {
////printf("parse_exp_unary()\n");
//  Token *tok = queue_peek(queue), *tok_next1, *tok_next2;
//
//  if (nextIsAndRemove(queue, MINUS)) {
//    tok_next1 = queue_peek(queue);
//
//    if (nextIsAndRemove(queue, MINUS)) {
//      tok_next2 = queue_peek(queue);
//      if (nextIsAndRemove(queue, WORD)) {
//
//        write_ins_id(GET, tok_next2->text, out);
//        write_ins_value(PUSH, 1, out);
//        write_ins_default(SUB, out);
//        write_ins_id(SET, tok_next2->text, out);
//        write_ins_id(GET, tok_next2->text, out);
//
//        return;
//      } else {
//        EXIT_WITH_MSG("Unexpected token. Expected '+'.")
//      }
//    } else if (INT == tok_next1->type) {
//      int val = (int) strtol(tok_next1->text, NULL, 10);
//      write_ins_value(PUSH, -val, out);
//      queue_remove(queue);
//      free(tok_next1);
//    } else if (FLOAT == tok_next1->type) {
//      double val_d = strtod(tok_next1->text, NULL);
//      write_ins_value_float(PUSH, -val_d, out);
//      queue_remove(queue);
//      free(tok_next1);
//    } else if (GTHAN != tok_next1->type) {
//      parse_exp_obj_item(queue, out);
//      write_ins_value(PUSH, -1, out);
//      write_ins_default(MULT, out);
//    } else {
//      queue_add_front(queue, tok);
//      return;
//    }
//
//  } else if (nextIsAndRemove(queue, TILDE)) {
//    parse_exp_unary(queue, out);
//    write_ins_default(NOT, out);
//  } else if (nextIsAndRemove(queue, PLUS)) {
//    if (nextIsAndRemove(queue, PLUS)) {
//      tok_next2 = queue_peek(queue);
//      if (nextIsAndRemove(queue, WORD)) {
//
//        write_ins_id(GET, tok_next2->text, out);
//        write_ins_value(PUSH, 1, out);
//        write_ins_default(ADD, out);
//        write_ins_id(SET, tok_next2->text, out);
//        write_ins_id(GET, tok_next2->text, out);
//
//        return;
//      } else {
//        EXIT_WITH_MSG("Unexpected token. Expected '+'.")
//      }
//    } else {
//      printf("Was %s\n", ((Token *) queue_peek(queue))->text);
//      EXIT_WITH_MSG("Unexpected token. Expected word or number.")
//    }
//
//    parse_exp_obj_item(queue, out);
//
//  } else if (WORD == tok->type) {
//    queue_remove(queue);
//    tok_next1 = queue_peek(queue);
//    if (PLUS == tok_next1->type) {
//      queue_remove(queue);
//      tok_next2 = queue_peek(queue);
//      if (PLUS == tok_next2->type) {
//        queue_remove(queue);
//
//        write_ins_id(GET, tok->text, out);
//        write_ins_id(GET, tok->text, out);
//        write_ins_value(PUSH, 1, out);
//        write_ins_default(ADD, out);
//        write_ins_id(SET, tok->text, out);
//
//        free(tok);
//        free(tok_next1);
//        free(tok_next2);
//
//        return;
//      } else {
//        queue_add_front(queue, tok_next1);
//        queue_add_front(queue, tok);
//      }
//    } else if (MINUS == tok_next1->type) {
//      queue_remove(queue);
//      tok_next2 = queue_peek(queue);
//      if (MINUS == tok_next2->type) {
//        queue_remove(queue);
//
//        write_ins_id(GET, tok->text, out);
//        write_ins_id(GET, tok->text, out);
//        write_ins_value(PUSH, 1, out);
//        write_ins_default(SUB, out);
//        write_ins_id(SET, tok->text, out);
//
//        free(tok);
//        free(tok_next1);
//        free(tok_next2);
//
//        return;
//      } else {
//        queue_add_front(queue, tok_next1);
//        queue_add_front(queue, tok);
//      }
//    } else {
//      queue_add_front(queue, tok);
//    }
//
//    parse_exp_obj_item(queue, out);
//
//  } else {
//    parse_exp_obj_item(queue, out);
//  }
//
//  // Array subscripting
//  while (nextIsAndRemove(queue, LBRAC)) {
//    parse_exp(queue, out);
//
//    CHECK(!nextIsAndRemove(queue, RBRAC), "Expected ].");
//
//    write_ins_default(AGET, out);
//  }
//
//}

void parse_exp_unary(Queue *queue, FILE *out) {
//printf("parse_exp_unary()\n");
  Token *tok_next1;

  if (nextIsAndRemove(queue, DEC)) {
    parse_exp_unary(queue, out);
    //write_ins_id(GET, tok_next2->text, out);
    write_ins_default(DUP, out);
    write_ins_default(DUP, out);
    write_ins_value(PUSH, 1, out);
    write_ins_default(SUB, out);
    write_ins_default(RSET, out);

  } else if (nextIsAndRemove(queue, MINUS)) {
    tok_next1 = queue_peek(queue);

    if (INT == tok_next1->type) {
      int val = (int) strtol(tok_next1->text, NULL, 10);
      write_ins_value(PUSH, -val, out);
    } else if (FLOAT == tok_next1->type) {
      double val_d = strtod(tok_next1->text, NULL);
      write_ins_value_float(PUSH, -val_d, out);
    } else if (GTHAN != tok_next1->type) {
      queue_remove(queue);
      free(tok_next1);
      parse_exp_obj_item(queue, out);
      write_ins_value(PUSH, -1, out);
      write_ins_default(MULT, out);
      return;
    } else {
      parse_exp_obj_item(queue, out);
      return;
    }

    queue_remove(queue);
    free(tok_next1);

  } else if (nextIsAndRemove(queue, TILDE)) {
    parse_exp_unary(queue, out);
    write_ins_default(NOT, out);
  } else if (nextIsAndRemove(queue, INC)) {
    parse_exp_unary(queue, out);
    //write_ins_id(GET, tok_next2->text, out);
    write_ins_default(DUP, out);
    write_ins_default(DUP, out);
    write_ins_value(PUSH, 1, out);
    write_ins_default(ADD, out);
    write_ins_default(RSET, out);

  } else {
    parse_exp_obj_item(queue, out);

    if (nextIsAndRemove(queue, INC)) {
      write_ins_default(DUP, out);
      write_ins_default(DREF, out);
      write_ins_default(FLIP, out);
      write_ins_default(DUP, out);
      write_ins_value(PUSH, 1, out);
      write_ins_default(ADD, out);
      write_ins_default(RSET, out);

    } else if (nextIsAndRemove(queue, DEC)) {
      write_ins_default(DUP, out);
      write_ins_default(DREF, out);
      write_ins_default(FLIP, out);
      write_ins_default(DUP, out);
      write_ins_value(PUSH, 1, out);
      write_ins_default(MINUS, out);
      write_ins_default(RSET, out);
    }
  }

// Array subscripting
  while (nextIsAndRemove(queue, LBRAC)) {
    parse_exp(queue, out);

    CHECK(!nextIsAndRemove(queue, RBRAC), "Expected ].");

    write_ins_default(AGET, out);
  }

}

//void parse_exp_obj_item(Queue *queue, FILE *out) {
////printf("parse_exp_obj_item()\n");
////fflush(stdout);
//
//  FILE *tmp = tmpfile();
//
//  Token item;
//
//  parse_exp_parens(queue, tmp);
//
//  if (!nextIsAndRemove(queue, PERIOD)) {
//    append(out, tmp);
//    fclose(tmp);
//    return;
//  }
//
//  item = *((Token *) queue_peek(queue));
//
//  CHECK(!nextIsAndRemove(queue, WORD), "Expected object field to be word.")
//
//  if (nextIsAndRemove(queue, LPAREN)) {
//    Token *tok_next = queue_peek(queue);
//    if (RPAREN != tok_next->type) {
//      parse_exp_tuple(queue, out);
//    }
//
//    CHECK(!nextIsAndRemove(queue, RPAREN), "Expected ). 2");
//
//    append(out, tmp);
//    if (MATCHES(item.text, NEW_KEYWORD)) {
//      write_ins_default(ONEW, out);
//    } else {
//      write_ins_id(OCALL, item.text, out);
//    }
//
//  } else {
//    append(out, tmp);
//    write_ins_id(OGET, item.text, out);
//  }
//
//  fclose(tmp);
//}

void parse_exp_obj_item(Queue *queue, FILE *out) {
//printf("parse_exp_obj_item()\n");
//fflush(stdout);

  FILE *tmp = tmpfile();

  Token item;

  parse_exp_parens(queue, tmp);

  if (!nextIsAndRemove(queue, PERIOD)) {
    append(out, tmp);
    fclose(tmp);
    return;
  }

  item = *((Token *) queue_peek(queue));

  CHECK(!nextIsAndRemove(queue, WORD), "Expected object field to be word.")

  if (nextIsAndRemove(queue, LPAREN)) {
    Token *tok_next = queue_peek(queue);
    if (RPAREN != tok_next->type) {
      parse_exp_tuple(queue, out);
    }

    CHECK(!nextIsAndRemove(queue, RPAREN), "Expected ). 2");

    append(out, tmp);
    if (MATCHES(item.text, NEW_KEYWORD)) {
      write_ins_default(ONEW, out);
    } else {
      write_ins_id(OCALL, item.text, out);
    }

  } else {
    append(out, tmp);
    write_ins_id(OGET, item.text, out);
  }

  fclose(tmp);
}




void parse_exp_parens(Queue *queue, FILE *out) {
//printf("parse_exp_parens()\n");
  if (nextIsAndRemove(queue, LPAREN)) {
    parse_exp(queue, out);
//printf("Was %s\n", ((Token *) queue_peek(queue))->text);
    CHECK(!nextIsAndRemove(queue, RPAREN), "Expected ). 1");
  } else if (nextIsAndRemove(queue, PIPE)) {
    parse_exp(queue, out);
//printf("WAS '%d' for |\n", ((Token *) queue_peek(queue))->type);
    CHECK(!nextIsAndRemove(queue, PIPE), "Expected | to close |x|.");
    write_ins_default(ALEN, out);
  } else {
    parse_exp_array_dec(queue, out);
  }

}

void parse_exp_array_dec(Queue *queue, FILE *out) {
//printf("parse_exp_tuple()\n");

  Token *tok;

  if (!nextIsAndRemove(queue, LBRAC)) {
    parse_exp_num_or_id(queue, out);
    return;
  }

  write_ins_default(ANEW, out);

  while (RBRAC != (tok = queue_peek(queue))->type) {
    parse_exp_for(queue, out);
    write_ins_default(AADD, out);
    if (!nextIsAndRemove(queue, COMMA)) {
      break;
    }
  }

  CHECK(!nextIsAndRemove(queue, RBRAC), "Expected ].");
}

void parse_exp_num_or_id(Queue *queue, FILE *out) {
//printf("parse_exp_num_or_id()\n");
  Token *tok = queue_peek(queue), *tok_next;
  int val;
  double val_d;
  if (WORD == tok->type) {

    if (is_keyword(tok->text)) {
      queue_remove(queue);
      if (MATCHES(RETURN_KEYWORD, tok->text)) {
        free(tok);
        if (0 == queue->size) {
          write_ins_default(RET, out);
          return;
        }

        tok_next = queue_peek(queue);
        if (RBRCE != tok_next->type && RPAREN != tok_next->type
            && ENDLINE != tok_next->type) {
          parse_exp(queue, out);
          remove_if_present(queue, ENDLINE);
        }
        remove_if_present(queue, ENDLINE);
        write_ins_default(RET, out);

      } else if (MATCHES(NONE_KEYWORD,
          tok->text) || MATCHES(FALSE_KEYWORD, tok->text)) {
        free(tok);
        write_ins_none(out);
      } else if (MATCHES(NONE_KEYWORD,
          tok->text) || MATCHES(TRUE_KEYWORD, tok->text)) {
        free(tok);
        write_ins_value(PUSH, 1, out);
      } else {
        queue_add_front(queue, tok);
      }

      return;
    } else if (get(classes, tok->text)) {
      Composite *class = get(classes, tok->text)->comp;
      queue_remove(queue);
      char *class_name = object_to_string(*composite_get(class, "name"));
      write_ins_id(CLSG, class_name, out);
      free(class_name);
    } else {
      queue_remove(queue);
      if (nextIsAndRemove(queue, LPAREN)) {
        tok_next = queue_peek(queue);
        int num_args = 0;
        if (RPAREN != tok_next->type) {
          num_args = parse_exp_tuple(queue, out);
        }

        //printf("WASSS '%s'\n", ((Token *)queue_peek(queue))->text);
        CHECK(!nextIsAndRemove(queue, RPAREN), "Expected ). 3");

        if (MATCHES(PRINT_FUNCTION, tok->text)) {
          if (num_args > 1) {
            write_ins_value(PUSH, num_args, out);
            write_ins_default(PRINTN, out);
          } else {
            write_ins_default(PRINT, out);
          }
        } else if (MATCHES(EXIT_FUNCTION, tok->text)) {
          write_ins_default(EXIT, out);
        } else {
          write_ins_id(CALL, tok->text, out);
        }

      } else if (NULL != get(fun_names, tok->text)) {
        write_ins_id(CALL, tok->text, out);
      } else {
        write_ins_id(GET, tok->text, out);
      }
    }

  } else if (INT == tok->type) {
    val = (int) strtol(tok->text, NULL, 10);
    write_ins_value(PUSH, val, out);
    queue_remove(queue);
  } else if (FLOAT == tok->type) {
    val_d = strtod(tok->text, NULL);
    write_ins_value_float(PUSH, val_d, out);
    queue_remove(queue);
  } else if (STR == tok->type) {
    write_ins_value_str(PUSH, tok->text, out);
    queue_remove(queue);
  } else {
    printf("WAS %d\nLine %d(%d)\n", tok->type, tok->line, tok->col);
    EXIT_WITH_MSG("Unexpected token. Expected word or number. 1")
  }

  free(tok);
}

void write_ins_default(Op op, FILE *out) {
  fprintf(out, "%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[op]);
  fflush(out);
}

void write_ins_value(Op op, int val, FILE *out) {
  fprintf(out, "%*c%s%*c%d\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[op],
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS[op]), ' ', val);
  fflush(out);
}

void write_ins_none(FILE *out) {
  fprintf(out, "%*c%s%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[PUSH],
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS[PUSH]), ' ', "None");
  fflush(out);
}

void write_ins_value_float(Op op, double val, FILE *out) {
  fprintf(out, "%*c%s%*c%f\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[op],
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS[op]), ' ', val);
  fflush(out);
}

void write_ins_value_str(Op op, char string[], FILE *out) {
  fprintf(out, "%*c%s%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[op],
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS[op]), ' ', string);
  fflush(out);
}

void write_ins_address(Op op, int adr, FILE *out) {
  fprintf(out, "%*c%s%*c%d\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[op],
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS[op]), ' ', adr);
  fflush(out);
}

void write_ins_id(Op op, char id[], FILE *out) {
  fprintf(out, "%*c%s%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[op],
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS[op]), ' ', id);
  fflush(out);
}

void write_ins_id_num(Op op, char id[], int num, FILE *out) {
  fprintf(out, "%*c%s%*c%s_%d\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS[op],
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS[op]), ' ', id, num);
  fflush(out);
}

void write_label(char label[], FILE *out) {
  fprintf(out, "@%s\n", label);
  fflush(out);
}

void write_label_num(char label[], int num, FILE *out) {
  fprintf(out, "@%s_%d\n", label, num);
  fflush(out);
}
