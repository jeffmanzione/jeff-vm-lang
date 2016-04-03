/*
 * main.c
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include "interpreter.h"
#include "program.h"

#define IN_NAME     "equals_test.jl"
#define TMP_NAME    "out.jm"
#define OUT_NAME    "out.jb"

int main(int argc, char *argv[]) {

  printf("sizeof(float96_t)=%d\n", sizeof(float96_t));
  load_program(IN_NAME, TMP_NAME, OUT_NAME);

  return 0;
}
