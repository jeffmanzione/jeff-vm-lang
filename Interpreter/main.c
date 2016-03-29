/*
 * main.c
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include "interpreter.h"
#include "program.h"

#define IN_NAME     "temp.jl"
#define TMP_NAME    "out.jm"
#define OUT_NAME    "out.jb"

int main(int argc, char *argv[]) {

  load_program(IN_NAME, TMP_NAME, OUT_NAME);

  return 0;
}
