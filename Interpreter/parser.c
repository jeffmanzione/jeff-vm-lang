/*
 * parser.c
 *
 *  Created on: Jan 9, 2016
 *      Author: Jeff
 */

#include "parser.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "class.h"
#include "context.h"

#define PARSER_CHECK(exp, tok, msg) {if (exp) {parser_exit(parser, tok, msg);}}
#define PARSER_NULL_CHECK(exp, tok, msg) {if (NULL == exp) {parser_exit(parser, tok, msg);}}

void parser_exit(Parser *parser, Token *tok, const char msg[]) {
  printf("Parsing Error in file '%s' at line %d, col %d:\n",
      parser->fi_in->name, tok->line, tok->col);
  if (tok->line >= 3) {
    printf("line %*d: %s", 5, tok->line - 2,
        file_info_lookup(parser->fi_in, tok->line - 2)->line_text);
  }
  if (tok->line >= 2) {
    printf("line %*d: %s", 5, tok->line - 1,
        file_info_lookup(parser->fi_in, tok->line - 1)->line_text);
  }
  printf("line %*d: %s", 5, tok->line,
      file_info_lookup(parser->fi_in, tok->line)->line_text);
  printf("See %.*s%.*s\n", 7 + tok->col, DASH_STRING, max(strlen(tok->text), 1),
  CARET_STRING);
  printf("Error message: %s\n", msg);
  exit(-1);
}

void remove_if_present(Parser *parser, TokenType type) {
  Token *tok;
  NULL_CHECK(parser->tok_q, "Unexpected EOF!");
  if (0 == parser->tok_q->size) {
    return;
  }

  tok = queue_peek(parser->tok_q);
  if (type == tok->type) {
    queue_remove(parser->tok_q);
    free(tok);
  }
}

int nextIsWordAndRemove(Parser *parser, char word[]) {
  Token *tok;
  NULL_CHECK(parser->tok_q, "Unexpected EOF!");
  if (1 > parser->tok_q->size) {
    return FALSE;
  }

  tok = queue_peek(parser->tok_q);

  if (WORD != tok->type || !MATCHES(word, tok->text)) {
    return FALSE;
  }

  queue_remove(parser->tok_q);
  free(tok);
  return TRUE;
}

int nextIsAndRemove(Parser *parser, TokenType type) {
  Token *tok;
  NULL_CHECK(parser->tok_q, "Unexpected EOF!");
  if (1 > parser->tok_q->size) {
    return FALSE;
  }
  tok = queue_peek(parser->tok_q);

  if (type != tok->type) {
    return FALSE;
  }

  queue_remove(parser->tok_q);
  free(tok);
  return TRUE;
}

int nextTwoAreAndRemove(Parser *parser, TokenType first, TokenType second) {
  Token *tok_1, *tok_2;
  NULL_CHECK(parser->tok_q, "Unexpected EOF!");
  if (2 > parser->tok_q->size) {
    return FALSE;
  }

  tok_1 = queue_peek(parser->tok_q);
  if (first != tok_1->type) {
    return FALSE;
  }

  queue_remove(parser->tok_q);

  tok_2 = queue_peek(parser->tok_q);
  if (second != tok_2->type) {
    queue_add_front(parser->tok_q, tok_1);
    return FALSE;
  }

  queue_remove(parser->tok_q);
  free(tok_1);
  free(tok_2);

  return TRUE;
}

int nextThreeAreAndRemove(Parser *parser, TokenType first, TokenType second,
    TokenType third) {
  Token *tok_1, *tok_2, *tok_3;
  NULL_CHECK(parser->tok_q, "Unexpected EOF!");
  if (3 > parser->tok_q->size) {
    return FALSE;
  }

  tok_1 = queue_peek(parser->tok_q);
  if (first != tok_1->type) {
    return FALSE;
  }

  queue_remove(parser->tok_q);

  tok_2 = queue_peek(parser->tok_q);
  if (second != tok_2->type) {
    queue_add_front(parser->tok_q, tok_1);
    return FALSE;
  }

  queue_remove(parser->tok_q);

  tok_3 = queue_peek(parser->tok_q);
  if (third != tok_3->type) {
    queue_add_front(parser->tok_q, tok_2);
    queue_add_front(parser->tok_q, tok_1);
    return FALSE;
  }

  queue_remove(parser->tok_q);
  free(tok_1);
  free(tok_2);
  free(tok_3);
  return TRUE;
}

int nextIsAndSecondIsntAndRemove(Parser *parser, TokenType first,
    TokenType notSecond) {
  Token *tok_1, *tok_2;
  NULL_CHECK(parser->tok_q, "Unexpected EOF!");
  if (1 > parser->tok_q->size) {
    return FALSE;
  }

  tok_1 = queue_peek(parser->tok_q);
  if (first != tok_1->type) {
    return FALSE;
  }

  queue_remove(parser->tok_q);

  if (0 == parser->tok_q->size) {
    return TRUE;
  }

  tok_2 = queue_peek(parser->tok_q);
  if (notSecond == tok_2->type) {
    queue_add_front(parser->tok_q, tok_1);
    return FALSE;
  }

  free(tok_1);

  return TRUE;
}

char *KEYWORDS[] = { DEF_KEYWORD, FUN_KEYWORD, IF_KEYWORD, THEN_KEYWORD,
ELSE_KEYWORD, WHILE_KEYWORD, FOR_KEYWORD, BREAK_KEYWORD, RETURN_KEYWORD,
AS_KEYWORD, IS_KEYWORD, NONE_KEYWORD, TRUE_KEYWORD, FALSE_KEYWORD,
FUN_KEYWORD, CLASS_KEYWORD, FIELD_KEYWORD, IMPORT_KEYWORD, NEW_KEYWORD };

int is_keyword(const char word[]) {
  int i;
  for (i = 0; i < sizeof(KEYWORDS) / sizeof(char *); i++) {
    if (MATCHES(word, KEYWORDS[i])) {
      return TRUE;
    }
  }
  return FALSE;
}

void parse(FileInfo *fi, Queue *queue, FILE *out) {
// printf("parse()\n"); fflush(stdout);
  Parser parser;
  parser.tok_q = queue;
  parser.top = out;
  parser.fun_names = hashtable_create(TABLE_SZ);
  parser.classes = hashtable_create(TABLE_SZ);
  parser.fi_in = fi;
  parser.in_name = NULL;

  FILE *tmp = tmpfile();
//FILE *tmp = out;

  queue_init(&parser.classes_queue);

  parse_top_level(&parser, tmp);

  hashtable_free(parser.fun_names, do_nothing);

  void write_classes_and_del(void *comp_obj) {
    composite_class_save_src(parser.top, ((Object *) comp_obj)->comp);
    fprintf(parser.top, "\n");
    // TODO
    //object_delete(comp_obj);
  }

  hashtable_free(parser.classes, do_nothing);
  queue_deep_delete(&parser.classes_queue, write_classes_and_del);
  fprintf(out, "\n");

  append(out, tmp);
}

void parse_top_level(Parser *parser, FILE *out) {
// printf("parse_top_level()\n"); fflush(stdout);

  while (0 < parser->tok_q->size) {
    remove_if_present(parser, ENDLINE);
    if (0 < parser->tok_q->size) {
      parse_elements(parser, out);
    } else {
      return;
    }
  }
}

void parse_elements(Parser *parser, FILE *out) {
  Token *tok = queue_peek(parser->tok_q);
  remove_if_present(parser, ENDLINE);

  if (nextIsWordAndRemove(parser, CLASS_KEYWORD)) {
    parse_class(parser, out);
  } else if (nextIsWordAndRemove(parser, DEF_KEYWORD)
      || nextIsWordAndRemove(parser, FUN_KEYWORD)) {
    parse_fun(parser, out);
  } else if (nextIsWordAndRemove(parser, IMPORT_KEYWORD)) {

    Token fn_token = *((Token *) queue_peek(parser->tok_q));
    char fn[MAX_LINE_LEN];
    Queue child_queue;
    fn[0] = '\0';

    if (nextIsAndRemove(parser, STR)) {
      strncpy(fn, fn_token.text + 1, strlen(fn_token.text) - 2);
      fn[strlen(fn_token.text) - 2] = '\0';

    } else if (nextIsAndRemove(parser, WORD)) {
      strncpy(fn, fn_token.text, strlen(fn_token.text));
      fn[strlen(fn_token.text)] = '\0';
      while (nextIsAndRemove(parser, PERIOD)) {
        strcat(fn, "/");
        Token ext = *((Token *) queue_peek(parser->tok_q));
        PARSER_CHECK(!nextIsAndRemove(parser, WORD), &ext,
            "Expected word after . in module extension.")
        strcat(fn, ext.text);
      }
      strcat(fn, ".jl");
      PARSER_CHECK(!file_exist(fn), &fn_token, "Could not find module.")

    } else {
      parser_exit(parser, &fn_token,
          "Expected string or module after keyword "IMPORT_KEYWORD".");
    }

    FileInfo child_fi = file_info(fn);

    queue_init(&child_queue);

    tokenize(&child_fi, &child_queue);

    Parser child_parser = *parser;
    child_parser.tok_q = &child_queue;
    child_parser.fi_in = &child_fi;
    strcrepl(fn, '/', '_');

    *strchr(fn, '.') = '\0';
    child_parser.in_name = fn;

    parse_top_level(&child_parser, out);

    //TODO Need to fix so don't have to do this
    parser->classes_queue = child_parser.classes_queue;

    file_info_finalize(child_fi);

  } else {
    // This will quit the parser.
    parser_exit(parser, tok, "Expected "DEF_KEYWORD" or "CLASS_KEYWORD".");
  }
}

void parse_class(Parser *parser, FILE *out) {
  Token class_name;
  Composite *class;
  Object *class_obj;

  class_name = *((Token *) queue_peek(parser->tok_q));

  PARSER_CHECK(!nextIsAndRemove(parser, WORD), &class_name,
      "Expected class name.");

  Class *super;

  PARSER_CHECK(NULL != hashtable_lookup(parser->classes, class_name.text),
      &class_name, "Class already exists!")

  if (nextIsAndRemove(parser, COLON)) {
    Token super_class_name = *((Token *) queue_peek(parser->tok_q));
    PARSER_CHECK(!nextIsAndRemove(parser, WORD), &super_class_name,
        "Expected super class name after colon in class declaration.");
    Object *super_obj = hashtable_lookup(parser->classes,
        super_class_name.text);

    PARSER_NULL_CHECK(super_obj, &super_class_name,
        "Could not find class with name.")
    super = super_obj->comp;
  } else {
    super = object_class;
  }
  class = composite_class_new(class_name.text, super);
  class_obj = NEW(class_obj, Object)
  class_obj->type = COMPOSITE;
  class_obj->comp = class;

  parse_class_body(parser, out, class, class_name.text);
//  composite_class_print_sumary(class);
  hashtable_insert(parser->classes, class_name.text, class_obj);
  queue_add(&parser->classes_queue, class_obj);
}

void parse_class_body(Parser *parser, FILE *out, Composite *class,
    const char class_name[]) {
  remove_if_present(parser, ENDLINE);

  if (!nextIsAndRemove(parser, LBRCE)) {
    parse_class_item(parser, out, class, class_name);
    return;
  }

  remove_if_present(parser, ENDLINE);

  while (RBRCE != ((Token *) queue_peek(parser->tok_q))->type) {
    parse_class_item(parser, out, class, class_name);
    remove_if_present(parser, ENDLINE);
  }

  remove_if_present(parser, ENDLINE);
  PARSER_CHECK(!nextIsAndRemove(parser, RBRCE), queue_peek(parser->tok_q),
      "Expected }");
}

void parse_class_item(Parser *parser, FILE *out, Composite *class,
    const char class_name[]) {
  Token *tok = queue_peek(parser->tok_q);
  if (nextIsWordAndRemove(parser, FIELD_KEYWORD)) {
    parse_class_field(parser, out, class, class_name);
  } else if (nextIsWordAndRemove(parser, DEF_KEYWORD)
      || nextIsWordAndRemove(parser, FUN_KEYWORD)) {
    parse_class_method(parser, out, class, class_name);
  } else {
    //printf("WAS %d\nLine %d(%d)\n", tok->type, tok->line, tok->col);
    //fflush(stdout);
    //parser_exit(parser, tok, "Expected "DEF_KEYWORD" or "FIELD_KEYWORD".");
    parser_exit(parser, tok, "Expected "DEF_KEYWORD" or "FIELD_KEYWORD".");

  }
}

void parse_class_field(Parser *parser, FILE *out, Composite *class,
    const char class_name[]) {
  Token *field_name;
  do {
    field_name = queue_remove(parser->tok_q);
    NULL_CHECK(field_name, "Unexpected end of file!");

    composite_class_add_field(class, field_name->text);
    free(field_name);

  } while (nextIsAndRemove(parser, COMMA));
}

void parse_class_method(Parser *parser, FILE *out, Composite *class,
    const char class_name[]) {
  Token def_name;
  int num_args;
  def_name = *((Token *) queue_peek(parser->tok_q));
  char label[ID_SZ];
  label[0] = '\0';

  PARSER_CHECK(!nextIsAndRemove(parser, WORD), queue_peek(parser->tok_q),
      "Expected fun name.");

  remove_if_present(parser, ENDLINE);

  method_to_label(class_name, def_name.text, label);

  PARSER_CHECK(NULL != hashtable_lookup(parser->fun_names, label), &def_name,
      "Class already has member with this method name. "
          "Method overloading is not supported!")

  hashtable_insert(parser->fun_names, label, (Object *) sizeof(Object));
  write_label(label, out);

  num_args = parse_fun_arguments(parser, out);

  composite_class_add_method(class, def_name.text, num_args);

  remove_if_present(parser, ENDLINE);

  if (nextIsAndRemove(parser, COLON)) {
    remove_if_present(parser, ENDLINE);

    PARSER_CHECK(!nextIsAndRemove(parser, LPAREN), queue_peek(parser->tok_q),
        "Expected ( after : in super constructor!")

    int super_num_args = 0;

    if (!nextIsAndRemove(parser, RPAREN)) {
      super_num_args = parse_exp_tuple(parser, out);
    }

    PARSER_CHECK(!nextIsAndRemove(parser, RPAREN), queue_peek(parser->tok_q),
        "Expected ) to end super constructor!")

    write_ins_id(GET, "self", out);
    write_ins_value(PUSH, super_num_args, out);
    write_ins_id(SCALL, NEW_KEYWORD, out);
    write_ins_default(POP, out);
  }

  remove_if_present(parser, ENDLINE);

  parse_body(parser, out);

  if (MATCHES(NEW_KEYWORD, def_name.text)) {
    write_ins_id(GET, "self", out);
  }

  write_ins_default(RET, out);

  fprintf(out, "\n");
  remove_if_present(parser, ENDLINE);
}

void parse_fun(Parser *parser, FILE *out) {
// printf("parse_fun()\n"); fflush(stdout);
  Token tok, def_name;

  def_name = *((Token *) queue_peek(parser->tok_q));

  PARSER_CHECK(!nextIsAndRemove(parser, WORD), &def_name, "Expected fun name.");

  PARSER_CHECK(NULL != hashtable_lookup(parser->fun_names, def_name.text),
      &def_name, "Duplicate function names (i.e., overloading) not allowed!")

  hashtable_insert(parser->fun_names, def_name.text, (Object *) sizeof(Object));

  remove_if_present(parser, ENDLINE);

  // if the following is true, the this is a function predef.
  tok = *((Token *) queue_peek(parser->tok_q));
  if (WORD == tok.type
      && (MATCHES(tok.text, DEF_KEYWORD) || MATCHES(tok.text, FUN_KEYWORD)
          || MATCHES(tok.text, CLASS_KEYWORD))) {
    //printf("TOK '%s'\n", tok->text);
    return;
  }

  write_label(def_name.text, out);

  parse_fun_arguments(parser, out);

  remove_if_present(parser, ENDLINE);

  parse_body(parser, out);

  if (0 == strcmp(MAIN_FUNCTION, def_name.text)) {
    write_ins_value(PUSH, 0, out);
    write_ins_default(EXIT, out);
  } else {
    write_ins_default(RET, out);
  }
  fprintf(out, "\n");
  remove_if_present(parser, ENDLINE);
}

int parse_fun_arguments_helper(Parser *parser, FILE *out, int index) {
/// printf("parse_fun_arguments_helper()\n"); fflush(stdout);
  Token tok_id = *((Token *) queue_peek(parser->tok_q));
  int num_args = 1;
  if (RPAREN == tok_id.type) {
    return 0;
  }
//printf("ARG NAME = %s\n", tok->text);
  PARSER_CHECK(!nextIsAndRemove(parser, WORD), queue_peek(parser->tok_q),
      "Expected arg name.");

  if (nextIsAndRemove(parser, COMMA)) {
    num_args = 1 + parse_fun_arguments_helper(parser, out, index + 1);
  }

  //printf("INDEX=%d, NUM_ARGS=%d\n", index, num_args);
  if (index > 0 || num_args > 1) {
    if (0 < index) {
      write_ins_default(DUP, out);
    }
    // Note that num_args is not necessarily the total number of args at this point.
    write_ins_value(PUSH, index, out);
    //write_ins_default(FLIP, out);
    write_ins_default(IGET, out);
  }
  write_ins_id(SET, tok_id.text, out);

  return num_args;
}

int parse_fun_arguments(Parser *parser, FILE *out) {
// printf("parse_fun_arguments()\n"); fflush(stdout);
  int num_args;
  if (!nextIsAndRemove(parser, LPAREN)) {
    return 0;
  }

  num_args = parse_fun_arguments_helper(parser, out, 0);
  PARSER_CHECK(!nextIsAndRemove(parser, RPAREN), queue_peek(parser->tok_q),
      "Expected )");
  return num_args;
}

void parse_body(Parser *parser, FILE *out) {
//  printf("parse_body()\n"); fflush(stdout);
  remove_if_present(parser, ENDLINE);

  if (!nextIsAndRemove(parser, LBRCE)) {
    parse_line(parser, out);
    return;
  }

  remove_if_present(parser, ENDLINE);

  while (RBRCE != ((Token *) queue_peek(parser->tok_q))->type) {
    parse_line(parser, out);
  }

  remove_if_present(parser, ENDLINE);
  PARSER_CHECK(!nextIsAndRemove(parser, RBRCE), queue_peek(parser->tok_q),
      "Expected }");
}

void parse_line(Parser *parser, FILE *out) {
//  printf("parse_line()\n"); fflush(stdout);

  parse_exp(parser, out);

  remove_if_present(parser, ENDLINE);

//else if (RBRCE != tok->type) {
//  parser_exit(parser, tok, "Expected ENDLINE or } at end of line.");;
//}
}

void parse_exp(Parser *parser, FILE *out) {
// printf("parse_exp()\n"); fflush(stdout);
  parse_exp_tuple(parser, out);
}

int parse_exp_tuple_helper(Parser *parser, FILE *out, int count) {
// printf("parse_exp_tuple()\n"); fflush(stdout);
  parse_exp_anon_fun(parser, out);

  if (!nextIsAndRemove(parser, COMMA)) {
    if (count > 0) {
      write_ins_value(PUSH, count + 1, out);
      write_ins_default(TUPL, out);
    }
    return 1;
  }
  return 1 + parse_exp_tuple_helper(parser, out, count + 1);
}

int parse_exp_tuple(Parser *parser, FILE *out) {
// printf("parse_exp_tuple()\n"); fflush(stdout);
  return parse_exp_tuple_helper(parser, out, 0);
}

void parse_exp_anon_fun(Parser *parser, FILE *out) {
  if (!nextIsAndRemove(parser, AT)) {
    parse_exp_for(parser, out);
    return;
  }

  int num_id = parser->tok_q->size;

  remove_if_present(parser, ENDLINE);

  write_ins_id_num(JUMP, "anon_end", num_id, parser, out);

  write_label_num("anon", num_id, parser, out);

  parse_fun_arguments(parser, out);

  remove_if_present(parser, ENDLINE);

  parse_body(parser, out);

  write_ins_default(RET, out);

  remove_if_present(parser, ENDLINE);

  write_label_num("anon_end", num_id, parser, out);

  write_ins_id_num(PGET, "anon", num_id, parser, out);
}

void parse_exp_for(Parser *parser, FILE *out) {
// printf("parse_exp_for()\n"); fflush(stdout);
  int num = parser->tok_q->size;

  if (!nextIsWordAndRemove(parser, FOR_KEYWORD)) {
    parse_exp_while(parser, out);
    return;
  }

//write_ins_default(OPEN, out);

  FILE *cond, *aft, *body_head;
  void parse_for_args() {
    cond = tmpfile();
    aft = tmpfile();
    body_head = tmpfile();

    Token var_name = *((Token *) queue_peek(parser->tok_q));
    if (nextTwoAreAndRemove(parser, WORD, COLON)) {
      parse_exp_for(parser, out);

      write_ins_id_num(SET, "arr_tmp", num, parser, out);
      write_ins_value(PUSH, 0, out);

      write_ins_default(DUP, cond);
      write_ins_id_num(GET, "arr_tmp", num, parser, cond);
      write_ins_default(ALEN, cond);
      write_ins_default(LT, cond);

      write_ins_default(DUP, body_head);
      write_ins_id_num(GET, "arr_tmp", num, parser, body_head);
      write_ins_default(FLIP, body_head);
      write_ins_default(IGET, body_head);
      write_ins_id(SET, var_name.text, body_head);

      write_ins_value(PUSH, 1, aft);
      write_ins_default(ADD, aft);

    } else {

      parse_exp_for(parser, out);

      if (!nextIsAndRemove(parser, COMMA)) {
        parser_exit(parser, queue_peek(parser->tok_q),
            "Missing first ',' in for initializer.");
        ;
      }
      parse_exp_for(parser, cond);

      if (!nextIsAndRemove(parser, COMMA)) {
        parser_exit(parser, queue_peek(parser->tok_q),
            "Missing second ',' comma in for initializer.");
        ;
      }
      parse_exp_for(parser, aft);
    }
  }

  if (nextIsAndRemove(parser, LPAREN)) {
    parse_for_args();

    if (!nextIsAndRemove(parser, RPAREN)) {
      parser_exit(parser, queue_peek(parser->tok_q),
          "Expected ')' after for initializer.");
      ;
    }
  } else {
    parse_for_args();
  }

  write_label_num("for", num, parser, out);

  append(out, cond);
  fclose(cond);

  write_ins_id_num(IFN, "end", num, parser, out);

  append(out, body_head);
  fclose(body_head);

  remove_if_present(parser, ENDLINE);

  parse_body(parser, out);

  remove_if_present(parser, ENDLINE);

  append(out, aft);
  fclose(aft);

  write_ins_id_num(JUMP, "for", num, parser, out);
  write_label_num("end", num, parser, out);
//write_ins_default(CLOSE, out);

  remove_if_present(parser, ENDLINE);
}

void parse_exp_while(Parser *parser, FILE *out) {
// printf("parse_exp_while()\n"); fflush(stdout);

  int num = parser->tok_q->size;
  if (!nextIsWordAndRemove(parser, WHILE_KEYWORD)) {
    parse_exp_if(parser, out);
    return;
  }

//write_ins_default(OPEN, out);

  write_label_num("while", num, parser, out);

  parse_exp(parser, out);

  write_ins_id_num(IFN, "end", num, parser, out);

  remove_if_present(parser, ENDLINE);

  parse_body(parser, out);

  write_ins_id_num(JUMP, "while", num, parser, out);

  write_label_num("end", num, parser, out);

//write_ins_default(CLOSE, out);

  remove_if_present(parser, ENDLINE);
}

void parse_exp_if(Parser *parser, FILE *out) {
//  printf("parse_exp_if()\n"); fflush(stdout);
  int num = parser->tok_q->size;
  if (!nextIsWordAndRemove(parser, IF_KEYWORD)) {
    parse_exp_assign(parser, out);
    return;
  }

//write_ins_default(OPEN, out);

  parse_exp_if(parser, out);

  write_ins_id_num(IFN, "else", num, parser, out);

  remove_if_present(parser, ENDLINE);

  // Optional then
  nextIsWordAndRemove(parser, THEN_KEYWORD);

  remove_if_present(parser, ENDLINE);

  parse_body(parser, out);

  write_ins_id_num(JUMP, "end", num, parser, out);

  write_label_num("else", num, parser, out);

  remove_if_present(parser, ENDLINE);

  if (nextIsWordAndRemove(parser, ELSE_KEYWORD)) {
    remove_if_present(parser, ENDLINE);
    parse_body(parser, out);
  }

  remove_if_present(parser, ENDLINE);

  write_label_num("end", num, parser, out);
//write_ins_default(CLOSE, out);
}

//void parse_exp_assign(Parser *parser, FILE *out) {
//  // printf("parse_exp_assign()\n"); fflush(stdout);
//  Token tok_first = *((Token *) queue_peek(parser->tok_q)), tok_second, tok_third;
//
//  if (!is_keyword(tok_first.text) && nextIsAndRemove(parser, WORD)) {
//
//    tok_second = *((Token *) queue_peek(parser->tok_q));
//    if (WORD == tok_second.type && !is_keyword(tok_second.text)) {
//      parse_exp_assign(parser, out);
//      goto set_id;
//    } else {
////      if (nextIsAndRemove(parser, LBRAC)) {
////        write_ins_id(GET, tok_first->text, out);
////
////        parse_exp(parser, out);
////
////        PARSER_CHECK(!nextIsAndRemove(parser, RBRAC), "Expected ].");
////
////        if (!nextIsAndSecondIsntAndRemove(parser, EQUALS, EQUALS)) {
////          write_ins_default(AGET, out);
////          return;
////        }
////
////        parse_exp(parser, out);
////
////        write_ins_default(ASET, out);
////        return;
////      }
//
//      if (!nextIsAndRemove(parser, EQUALS)) {
//        queue_add_front(parser->tok_q, token_copy(tok_first));
//        parse_exp_assign_array(parser, out);
//        return;
//      }
//
//      tok_third = *((Token *) queue_peek(parser->tok_q));
//
//      if (EQUALS == tok_third.type || GTHAN == tok_third.type) {
//        queue_add_front(parser->tok_q, token_copy(tok_second));
//        queue_add_front(parser->tok_q, token_copy(tok_first));
//        parse_exp_assign_array(parser, out);
//        return;
//      }
//
//    }
//  } else {
//    parse_exp_assign_array(parser, out);
//    return;
//  }
//
//  parse_exp_assign(parser, out);
//
//  set_id: write_ins_id(SET, tok_first.text, out);
//}

void parse_exp_assign_multi_lhs(Parser *parser, FILE *out, int index) {
// recurse
  Token var_name = *((Token *) queue_peek(parser->tok_q));
  write_ins_default(DUP, out);
  write_ins_value(PUSH, index, out);
//write_ins_default(FLIP, out);
  write_ins_default(IGET, out);
  if (nextTwoAreAndRemove(parser, WORD, COMMA)) {
    write_ins_id(SET, var_name.text, out);
    parse_exp_assign_multi_lhs(parser, out, index + 1);
  } else if (nextTwoAreAndRemove(parser, WORD, RBRCE)) {
    write_ins_id(SET, var_name.text, out);
  } else {
    parse_exp_assign_array(parser, out);
    write_ins_default(FLIP, out);
    write_ins_default(RSET, out);
    if (nextIsAndRemove(parser, COMMA)) {
      parse_exp_assign_multi_lhs(parser, out, index + 1);
    } else {
      PARSER_CHECK(!nextIsAndRemove(parser, RBRCE), queue_peek(parser->tok_q),
          "Expected } to close LHS assign")
    }
  }

}

void parse_exp_assign(Parser *parser, FILE *out) {
// printf("parse_exp_assign()\n"); fflush(stdout);
  Token var_name = *((Token *) queue_peek(parser->tok_q));

  if (nextIsAndRemove(parser, LBRCE)) {
    FILE *hold = tmpfile();

    parse_exp_assign_multi_lhs(parser, hold, 0);

    PARSER_CHECK(!nextIsAndRemove(parser, EQUALS), queue_peek(parser->tok_q),
        "Expected = after LHS assign")

    parse_exp_assign(parser, out);

    append(out, hold);
    fclose(hold);

    return;
  }

  if (nextIsAndRemove(parser, WORD)) {

    if (!nextIsAndRemove(parser, EQUALS)) {
      queue_add_front(parser->tok_q, token_copy(var_name));
    } else {
      parse_exp_assign(parser, out);
      write_ins_id(SET, var_name.text, out);
      return;
    }
  }

  parse_exp_assign_array(parser, out);

  if (!nextIsAndRemove(parser, EQUALS)) {
    return;
  }

  parse_exp_assign(parser, out);

  write_ins_default(RSET, out);

}

void parse_exp_assign_array(Parser *parser, FILE *out) {
// printf("parse_exp_assign_array()"); fflush(stdout);
  FILE *hold = tmpfile();
  parse_exp_or(parser, hold);

  if (nextTwoAreAndRemove(parser, LARROW, GTHAN)) {
    append(out, hold);
    parse_exp_or(parser, out);
    write_ins_default(SWAP, out);

  } else if (nextTwoAreAndRemove(parser, LTHANEQ, COLON)) {
    append(out, hold);
    parse_exp_or(parser, out);
    write_ins_default(APOP, out);
    write_ins_default(RSET, out);

  } else if (nextIsAndRemove(parser, LARROW)) {
    append(out, hold);

    parse_exp_or(parser, out);
    PARSER_CHECK(!nextIsAndRemove(parser, COLON), queue_peek(parser->tok_q),
        "Expected : after array in <- exp.")
    parse_exp(parser, out);

    write_ins_default(AREM, out);
    write_ins_default(RSET, out);

  } else if (nextThreeAreAndRemove(parser, EQUALS, GTHAN, COLON)) {
    parse_exp_or(parser, out);

    append(out, hold);

    write_ins_default(APUSH, out);

  } else if (nextIsAndRemove(parser, RARROW)) {
    parse_exp_or(parser, out);

    PARSER_CHECK(!nextIsAndRemove(parser, COLON), queue_peek(parser->tok_q),
        "Expected : after array in -> exp.")
    parse_exp(parser, out);

    append(out, hold);

    write_ins_default(AINS, out);

  } else if (nextTwoAreAndRemove(parser, COLON, LTHANEQ)) {
    append(out, hold);

    parse_exp(parser, out);
    write_ins_default(AENQ, out);

  } else if (nextThreeAreAndRemove(parser, COLON, EQUALS, GTHAN)) {
    append(out, hold);
    Token var_name = *((Token *) queue_peek(parser->tok_q));
    PARSER_CHECK(!nextIsAndRemove(parser, WORD), &var_name,
        "Expected var name after :=>")
    write_ins_default(ADEQ, out);
    write_ins_id(SET, var_name.text, out);

  } else if (nextIsAndRemove(parser, COLON)) {
    append(out, hold);
    parse_exp_or(parser, out);

    if (nextIsAndRemove(parser, RARROW)) {
      parse_exp(parser, out);

      write_ins_default(AREM, out);
      write_ins_default(RSET, out);

    } else if (nextIsAndRemove(parser, LARROW)) {
      parse_exp(parser, out);

      write_ins_default(AINS, out);

    } else if (nextIsAndRemove(parser, EQUALS)) {
      parse_exp(parser, out);

      write_ins_default(ASET, out);

    } else {
      //printf("Was %d\n", ((Token *) queue_peek(parser->tok_q))->line);
      parser_exit(parser, queue_peek(parser->tok_q),
          "Expected ->, <-, or = after a:x expression");
    }

  } else {
    append(out, hold);
  }

  fclose(hold);
}

void parse_exp_or(Parser *parser, FILE *out) {
//printf("parse_exp_or()\n"); fflush(stdout);
  parse_exp_xor(parser, out);

  Token tok = *((Token *) queue_peek(parser->tok_q));

  if (!nextIsAndRemove(parser, PIPE)) {
    return;
  }

  Token *next = queue_peek(parser->tok_q);

  if (RPAREN == next->type || RBRAC == next->type || RBRCE == next->type
      || COMMA == next->type || PLUS == next->type || MINUS == next->type
      || STAR == next->type || FSLASH == next->type) {
    queue_add_front(parser->tok_q, token_copy(tok));
    return;
  }

  parse_exp_or(parser, out);

  write_ins_default(OR, out);

}

void parse_exp_xor(Parser *parser, FILE *out) {
//printf("parse_exp_xor()\n"); fflush(stdout);
  parse_exp_and(parser, out);

  if (!nextIsAndRemove(parser, CARET)) {
    return;
  }

  parse_exp_xor(parser, out);

  write_ins_default(XOR, out);
}

void parse_exp_and(Parser *parser, FILE *out) {
//printf("parse_exp_and()\n"); fflush(stdout);
  parse_exp_eq(parser, out);

  if (!nextIsAndRemove(parser, AMPER)) {
    return;
  }

  parse_exp_and(parser, out);

  write_ins_default(AND, out);

}

void parse_exp_eq(Parser *parser, FILE *out) {
//printf("parse_exp_eq()\n"); fflush(stdout);
  parse_exp_lt_gt(parser, out);

  if (nextIsAndRemove(parser, EQUIV)) {

    parse_exp_eq(parser, out);

    write_ins_default(EQ, out);
  } else if (nextIsWordAndRemove(parser, IS_KEYWORD)) {
    if (nextIsWordAndRemove(parser, TYPE_INT_KEYWORD)) {
      write_ins_default(ISI, out);
    } else if (nextIsWordAndRemove(parser, TYPE_FLOAT_KEYWORD)) {
      write_ins_default(ISF, out);
    } else if (nextIsWordAndRemove(parser, TYPE_CHAR_KEYWORD)) {
      write_ins_default(ISC, out);
    } else if (nextIsWordAndRemove(parser, TYPE_OBJ_KEYWORD)) {
      write_ins_default(ISO, out);
    } else if (nextIsWordAndRemove(parser, TYPE_ARRAY_KEYWORD)
        || nextIsWordAndRemove(parser, "String")) {
      write_ins_default(ISA, out);
    } else {
      parse_exp_eq(parser, out);
      write_ins_default(IS, out);
    }
  } else if (nextIsWordAndRemove(parser, ISNT_KEYWORD)) {
    parse_exp_eq(parser, out);
    write_ins_default(IS, out);
    write_ins_default(NOT, out);
  } else {
    return;
  }

}

void parse_exp_lt_gt(Parser *parser, FILE *out) {
//printf("parse_exp_lt_gt()\n"); fflush(stdout);
  Token tok_first;
  Op op;
  parse_exp_add_sub(parser, out);

  tok_first = *((Token*) queue_peek(parser->tok_q));

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

  queue_remove(parser->tok_q);

  parse_exp_lt_gt(parser, out);

  write_ins_default(op, out);

}

void parse_exp_add_sub(Parser *parser, FILE *out) {
//printf("parse_exp_add_sub()\n"); fflush(stdout);
  Token tok, *tmp, next;
  parse_exp_mult_div(parser, out);

  tok = *((Token *) queue_peek(parser->tok_q));

  switch (tok.type) {
    case (PLUS):
    case (MINUS):
      tmp = queue_remove(parser->tok_q);
      next = *((Token *) queue_peek(parser->tok_q));
      if (GTHAN == next.type) {
        queue_add_front(parser->tok_q, tmp);
        return;
      }
      free(tmp);
      break;
    default:
      return;
  }

  parse_exp_add_sub(parser, out);

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

void parse_exp_mult_div(Parser *parser, FILE *out) {
//printf("parse_exp_mult_div()\n"); fflush(stdout);
  Token tok;
  parse_exp_array_transfer(parser, out);

  tok = *((Token *) queue_peek(parser->tok_q));

  switch (tok.type) {
    case (STAR):
    case (FSLASH):
    case (PERCENT):
      free(queue_remove(parser->tok_q));
      break;
    default:
      return;
  }

  parse_exp_mult_div(parser, out);

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

void parse_exp_array_transfer(Parser *parser, FILE *out) {
//printf("parse_exp_mult_div()\n"); fflush(stdout);
  Token tok_1 = *((Token *) queue_peek(parser->tok_q)), tok_2;

  if (nextIsAndRemove(parser, WORD)) {

    if (nextThreeAreAndRemove(parser, LTHAN, LTHAN, COLON)) {
      tok_2 = *((Token *) queue_peek(parser->tok_q));

      PARSER_CHECK(!nextIsAndRemove(parser, WORD), queue_peek(parser->tok_q),
          "Shift must be used with arrays.")

      write_ins_id(GET, tok_1.text, out);
      write_ins_id(GET, tok_2.text, out);
      write_ins_default(ALSH, out);

    } else if (nextThreeAreAndRemove(parser, COLON, GTHAN, GTHAN)) {
      tok_2 = *((Token *) queue_peek(parser->tok_q));

      PARSER_CHECK(!nextIsAndRemove(parser, WORD), queue_peek(parser->tok_q),
          "Shift must be used with arrays.")

      write_ins_id(GET, tok_1.text, out);
      write_ins_id(GET, tok_2.text, out);
      write_ins_default(ARSH, out);
    } else {
      queue_add_front(parser->tok_q,
          token_create(tok_1.type, tok_1.line, tok_1.col, tok_1.text));
      parse_exp_casting(parser, out);
    }
  } else {
    parse_exp_casting(parser, out);
  }

}

void parse_exp_casting(Parser *parser, FILE *out) {
//printf("parse_exp_casting()\n"); fflush(stdout);
  Token tok;
  parse_exp_unary(parser, out);

  tok = *((Token *) queue_peek(parser->tok_q));

  if (WORD != tok.type || !MATCHES(AS_KEYWORD, tok.text)) {
    return;
  }

  free(queue_remove(parser->tok_q));

  tok = *((Token *) queue_peek(parser->tok_q));

  PARSER_CHECK(!nextIsAndRemove(parser, WORD), queue_peek(parser->tok_q),
      "Expect type to cast.")

  if (MATCHES(TYPE_INT_KEYWORD, tok.text)) {
    write_ins_default(TOI, out);
  } else if (MATCHES(TYPE_FLOAT_KEYWORD, tok.text)) {
    write_ins_default(TOF, out);
  } else {
    parser_exit(parser, &tok, "Expect type to cast. (2)");
    ;
  }

}

void parse_exp_unary(Parser *parser, FILE *out) {
//printf("parse_exp_unary()\n");
  Token *tok_next1;

  if (nextIsAndRemove(parser, DEC)) {
    parse_exp_unary(parser, out);
    //write_ins_id(GET, tok_next2->text, out);
    write_ins_default(DUP, out);
    write_ins_default(DUP, out);
    write_ins_value(PUSH, 1, out);
    write_ins_default(SUB, out);
    write_ins_default(RSET, out);

  } else if (nextIsAndRemove(parser, MINUS)) {
    tok_next1 = queue_peek(parser->tok_q);

    if (INT == tok_next1->type) {
      int64_t val = (int64_t) strtoll(tok_next1->text, NULL, 10);
      write_ins_value(PUSH, -val, out);
    } else if (FLOAT == tok_next1->type) {
      float96_t val_d = strtold(tok_next1->text, NULL);
      write_ins_value_float(PUSH, -val_d, out);
    } else if (GTHAN != tok_next1->type) {
      queue_remove(parser->tok_q);
      free(tok_next1);
      parse_exp_obj_item(parser, out);
      write_ins_value(PUSH, -1, out);
      write_ins_default(MULT, out);
      return;
    } else {
      parse_exp_obj_item(parser, out);
      return;
    }

    queue_remove(parser->tok_q);
    free(tok_next1);

  } else if (nextIsAndRemove(parser, TILDE)) {
    parse_exp_unary(parser, out);
    write_ins_default(NOT, out);
  } else if (nextIsAndRemove(parser, INC)) {
    parse_exp_unary(parser, out);
    //write_ins_id(GET, tok_next2->text, out);
    write_ins_default(DUP, out);
    write_ins_default(DUP, out);
    write_ins_value(PUSH, 1, out);
    write_ins_default(ADD, out);
    write_ins_default(RSET, out);

  } else {
    parse_exp_obj_item(parser, out);

    while (TRUE) {
      if (nextIsAndRemove(parser, INC)) {
        write_ins_default(DUP, out);
        write_ins_default(DREF, out);
        write_ins_default(FLIP, out);
        write_ins_default(DUP, out);
        write_ins_value(PUSH, 1, out);
        write_ins_default(ADD, out);
        write_ins_default(RSET, out);

      } else if (nextIsAndRemove(parser, DEC)) {
        write_ins_default(DUP, out);
        write_ins_default(DREF, out);
        write_ins_default(FLIP, out);
        write_ins_default(DUP, out);
        write_ins_value(PUSH, 1, out);
        write_ins_default(MINUS, out);
        write_ins_default(RSET, out);
      } else if (nextIsAndRemove(parser, LBRAC)) {
        parse_exp(parser, out);

        PARSER_CHECK(!nextIsAndRemove(parser, RBRAC), queue_peek(parser->tok_q),
            "Expected ].");

        write_ins_default(IGET, out);

      } else {
        break;
      }
    }
  }

//parse_exp_subscript(parser, out);
}

void parse_exp_subscript(Parser *parser, FILE *out) {

// Array subscripting
  while (nextIsAndRemove(parser, LBRAC)) {
    parse_exp(parser, out);

    PARSER_CHECK(!nextIsAndRemove(parser, RBRAC), queue_peek(parser->tok_q),
        "Expected ].");

    write_ins_default(IGET, out);
  }
}

void parse_exp_obj_item_helper(Parser *parser, FILE *prev_buffer,
    FILE *total_accum, char prev_word[], FILE *out) {

  if (!nextIsAndRemove(parser, PERIOD)) {
    append(total_accum, prev_buffer);
    append(out, total_accum);
    return;
  }

  Token item = *((Token *) queue_peek(parser->tok_q));

  PARSER_CHECK(!nextIsAndRemove(parser, WORD), queue_peek(parser->tok_q),
      "Expected object field to be word.")

  if (nextIsAndRemove(parser, LPAREN)) {
    Token *tok_next = queue_peek(parser->tok_q);
    size_t tuple_size = 0;
    if (RPAREN != tok_next->type) {
      tuple_size = parse_exp_tuple(parser, total_accum);
    }

    PARSER_CHECK(!nextIsAndRemove(parser, RPAREN), queue_peek(parser->tok_q),
        "Expected ). 2");

    append(total_accum, prev_buffer);

    write_ins_value(PUSH, tuple_size, total_accum);

    if (MATCHES(item.text, NEW_KEYWORD)) {
      write_ins_default(ONEW, total_accum);
    } else if (MATCHES(SUPER_KEYWORD, prev_word)) {
      write_ins_id(SCALL, item.text, total_accum);
    } else {
      write_ins_id(OCALL, item.text, total_accum);
    }

  } else {
    append(total_accum, prev_buffer);
    write_ins_id(OGET, item.text, total_accum);
  }

  parse_exp_subscript(parser, total_accum);

  FILE *my_level = tmpfile();
  parse_exp_obj_item_helper(parser, total_accum, my_level, prev_word, out);
  fclose(my_level);
}

void parse_exp_obj_item(Parser *parser, FILE *out) {
//printf("parse_exp_obj_item()\n");
//fflush(stdout);

  FILE *tmp = tmpfile();

  Token first_word = *((Token *) queue_peek(parser->tok_q));

  parse_exp_parens(parser, tmp);

  FILE *my_level = tmpfile();
  parse_exp_obj_item_helper(parser, tmp, my_level, first_word.text, out);
  fclose(my_level);
  fclose(tmp);
}

void parse_exp_parens(Parser *parser, FILE *out) {
//printf("parse_exp_parens()\n");
  if (nextIsAndRemove(parser, LPAREN)) {
    parse_exp(parser, out);
//printf("Was %s\n", ((Token *) queue_peek(parser->tok_q))->text);
    PARSER_CHECK(!nextIsAndRemove(parser, RPAREN), queue_peek(parser->tok_q),
        "Expected ). 1");
  } else if (nextIsAndRemove(parser, PIPE)) {
    parse_exp(parser, out);
//printf("WAS '%d' for |\n", ((Token *) queue_peek(parser->tok_q))->type);
    PARSER_CHECK(!nextIsAndRemove(parser, PIPE), queue_peek(parser->tok_q),
        "Expected | to close |x|.");
    write_ins_default(ALEN, out);
  } else {
    parse_exp_array_dec(parser, out);
  }

}

void parse_exp_array_dec(Parser *parser, FILE *out) {
//printf("parse_exp_tuple()\n");

  Token *tok;

  if (!nextIsAndRemove(parser, LBRAC)) {
    parse_exp_num_or_id(parser, out);
    return;
  }

  write_ins_default(ANEW, out);

  while (RBRAC != (tok = queue_peek(parser->tok_q))->type) {
    remove_if_present(parser, ENDLINE);
    parse_exp_anon_fun(parser, out);
    write_ins_default(AADD, out);
    if (!nextIsAndRemove(parser, COMMA)) {
      break;
    }
  }

  remove_if_present(parser, ENDLINE);

  PARSER_CHECK(!nextIsAndRemove(parser, RBRAC), queue_peek(parser->tok_q),
      "Expected ].");
}

void parse_exp_num_or_id(Parser *parser, FILE *out) {
//printf("parse_exp_num_or_id()\n");
  Token *tok = queue_peek(parser->tok_q), *tok_next;
  int64_t val;
  float96_t val_d;
  if (WORD == tok->type) {

    if (is_keyword(tok->text)) {
      queue_remove(parser->tok_q);
      if (MATCHES(RETURN_KEYWORD, tok->text)) {
        free(tok);
        if (0 == parser->tok_q->size) {
          write_ins_default(RET, out);
          return;
        }

        tok_next = queue_peek(parser->tok_q);
        if (RBRCE != tok_next->type && RPAREN != tok_next->type
            && ENDLINE != tok_next->type) {
          parse_exp(parser, out);
          remove_if_present(parser, ENDLINE);
        }
        remove_if_present(parser, ENDLINE);
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
        queue_add_front(parser->tok_q, tok);
      }

      return;
    }
    if (hashtable_lookup(parser->classes, tok->text)) {
      Composite *class = ((Object *) hashtable_lookup(parser->classes,
          tok->text))->comp;
      queue_remove(parser->tok_q);
      char *class_name = object_to_string(*composite_get(class, "name"));

      int is_new = FALSE;
      size_t tuple_size = 0;
      if (nextIsAndRemove(parser, LPAREN)) {
        is_new = TRUE;
        Token *tok_next = queue_peek(parser->tok_q);
        if (RPAREN != tok_next->type) {
          tuple_size = parse_exp_tuple(parser, out);
        }

        PARSER_CHECK(!nextIsAndRemove(parser, RPAREN),
            queue_peek(parser->tok_q), "Expected ). 2");
      }

      write_ins_id(CLSG, class_name, out);
      free(class_name);

      if (is_new) {
        write_ins_value(PUSH, tuple_size, out);
        write_ins_default(ONEW, out);
      }

    } else {
      queue_remove(parser->tok_q);
      if (nextIsAndRemove(parser, LPAREN)) {
        tok_next = queue_peek(parser->tok_q);
        int num_args = 0;
        if (RPAREN != tok_next->type) {
          num_args = parse_exp_tuple(parser, out);
        }

        //printf("WASSS '%s'\n", ((Token *)queue_peek(parser->tok_q))->text);
        PARSER_CHECK(!nextIsAndRemove(parser, RPAREN),
            queue_peek(parser->tok_q), "Expected ). 3");

        if (MATCHES(PRINT_FUNCTION, tok->text)) {
          if (num_args > 1) {
            write_ins_value(PUSH, num_args, out);
            write_ins_default(PRINTN, out);
          } else {
            write_ins_default(PRINT, out);
          }
        } else if (MATCHES(EXIT_FUNCTION, tok->text)) {
          write_ins_default(EXIT, out);
        } else if (MATCHES(HASH_FUNCTION, tok->text)) {
          write_ins_default(HASH, out);
        } else if (NULL == hashtable_lookup(parser->fun_names, tok->text)) {
          write_ins_id(GET, tok->text, out);
          write_ins_default(PCALL, out);
        } else {
          write_ins_id(CALL, tok->text, out);
        }
      } else if (NULL != hashtable_lookup(parser->fun_names, tok->text)) {
        write_ins_id(PGET, tok->text, out);
      } else {
        write_ins_id(GET, tok->text, out);
      }
    }

  } else if (INT == tok->type) {
    val = (int64_t) strtoll(tok->text, NULL, 10);
    write_ins_value(PUSH, val, out);
    queue_remove(parser->tok_q);
  } else if (FLOAT == tok->type) {
    val_d = strtod(tok->text, NULL);
    write_ins_value_float(PUSH, val_d, out);
    queue_remove(parser->tok_q);
  } else if (STR == tok->type) {
    write_ins_value_str(PUSH, tok->text, out);
    queue_remove(parser->tok_q);
  } else {
    printf("WAS %d\nLine %d(%d)\n", tok->type, tok->line, tok->col);
    parser_exit(parser, tok, "Unexpected token. Expected word or number. 1");
  }

  free(tok);
}

void write_ins_default(Op op, FILE *out) {
  fprintf(out, "%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op));
  fflush(out);
}

void write_ins_value(Op op, int64_t val, FILE *out) {
  fprintf(out, "%*c%s%*c%"PRId64"\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op),
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(op)), ' ', val);
  fflush(out);
}

void write_ins_none(FILE *out) {
  fprintf(out, "%*c%s%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(PUSH),
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(PUSH)), ' ', "None");
  fflush(out);
}

void write_ins_value_float(Op op, float96_t val, FILE *out) {
  fprintf(out, "%*c%s%*c%f\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op),
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(op)), ' ',
      (double) val);
  fflush(out);
}

void write_ins_value_str(Op op, char string[], FILE *out) {
  fprintf(out, "%*c%s%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op),
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(op)), ' ', string);
  fflush(out);
}

void write_ins_address(Op op, int adr, FILE *out) {
  fprintf(out, "%*c%s%*c%d\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op),
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(op)), ' ', adr);
  fflush(out);
}

void write_ins_id(Op op, char id[], FILE *out) {
  fprintf(out, "%*c%s%*c%s\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op),
  SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(op)), ' ', id);
  fflush(out);
}

void write_ins_id_num(Op op, char id[], int num, Parser *parser, FILE *out) {
  if (NULL == parser->in_name) {
    fprintf(out, "%*c%s%*c%s_%d\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op),
    SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(op)), ' ', id,
        num);
  } else {
    fprintf(out, "%*c%s%*c%s_%s_%d\n", FIRST_COL_INDEX, ' ', INSTRUCTIONS(op),
    SECOND_COL_INDEX - FIRST_COL_INDEX - strlen(INSTRUCTIONS(op)), ' ', id,
        parser->in_name, num);
  }
  fflush(out);
}

void write_label(char label[], FILE *out) {
  fprintf(out, "@%s\n", label);
  fflush(out);
}

void write_label_num(char label[], int num, Parser *parser, FILE *out) {
  if (NULL == parser->in_name) {
    fprintf(out, "@%s_%d\n", label, num);
  } else {
    fprintf(out, "@%s_%s_%d\n", label, parser->in_name, num);
  }
  fflush(out);
}
