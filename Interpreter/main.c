/*
 * main.c
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include <stdio.h>
#include <stdlib.h>

#include "command_line.h"
#include "interpreter.h"
#include "memory/memory.h"
#include "memory/memory_shared.h"

//#define IN_NAME     "out.jb"
//#define TMP_NAME    "out.jm"
//#define OUT_NAME    "out.jb"

#define FAIL_ON(stat, msg)  case stat: printf(msg); fflush(stdout); return EXIT_SUCCESS;
#define HANDLE_SUCCESS() default: break;

int main(int argc, char *argv[]) {

  CL_ProcessInfo clpi;
  CL_Status status = cl_args_process(&clpi, argc, argv);
  //cl_args_print_summary(&clpi);
  switch (status) {
    FAIL_ON(CL_TOO_FEW_ARGS, "Too few arguments specified!\n")
    FAIL_ON(CL_TOO_MANY_ARGS, "Too many arguments specified!\n")
    FAIL_ON(CL_UNKNOWN_FLAG, "Unknown flag!\n")
    FAIL_ON(CL_WEIRD_FILE_EXTENSION,
        "File has wrong extension! Must be '.JL', '.JM', or '.JB'.\n")
    HANDLE_SUCCESS()
  }

  memory_init();
  memory_set_policy(IMMEDIATE);

  void error_handler(MemoryError *error) {
    exit(-1);
  }

  memory_set_error_handler(error_handler);

  load_program(&clpi);

  cl_args_finalize(&clpi);

  memory_free();

  return EXIT_SUCCESS;
}
