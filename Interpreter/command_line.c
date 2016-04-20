/*
 * command_line.c
 *
 *  Created on: Apr 3, 2016
 *      Author: Jeff
 */

#include "command_line.h"

#include <stdlib.h>
#include <string.h>

#include "shared.h"

char *J_FILE_TYPE[] = { "J_NONE", "J_LANG", "J_BIN" };

char *CL_ACTION[] = { "NOTHING", "WRITE_SRC", "WRITE_BIN", "WRITE_SRC_AND_BIN",
    "CREATE_LIB", "WRITE_SRC_AND_CREATE_LIB", "WRITE_BIN_AND_CREATE_LIB",
    "WRITE_SRC_AND_BIN_AND_CREATE_LIB", "EXECUTE", "WRITE_SRC_AND_EXECUTE",
    "WRITE_BIN_AND_EXECUTE", "WRITE_SRC_AND_BIN_AND_EXECUTE",
    "CREATE_LIB_AND_EXECUTE", "WRITE_SRC_AND_CREATE_LIB_AND_EXECUTE",
    "WRITE_BIN_AND_CREATE_LIB_AND_EXECUTE",
    "WRITE_SRC_AND_BIN_AND_CREATE_LIB_AND_EXECUTE", };

CL_Status cl_flag_process(CL_ProcessInfo *clpi, const char arg[]) {
  switch (arg[0]) {
    case 's':
      clpi->action |= WRITE_SRC;
      break;
    case 'c':
      clpi->action |= WRITE_BIN;
      break;
    case 'l':
      clpi->action |= CREATE_LIB;
      break;
    case 'e':
      clpi->action |= EXECUTE;
      break;
    default:
      return CL_UNKNOWN_FLAG;
  }
  return CL_SUCCESS;
}

CL_Status cl_args_process(CL_ProcessInfo *clpi, int argc, char **argv) {
  clpi->action = NOTHING;
  clpi->in.file_type = J_NONE;
  int i;
  for (i = 1; i < argc; i++) {
    char *arg = argv[i];
    if ('-' == arg[0]) {
      if (CL_UNKNOWN_FLAG == cl_flag_process(clpi, arg + 1)) {
        return CL_UNKNOWN_FLAG;
      }
    } else {
      if (J_NONE != clpi->in.file_type) {
        return CL_TOO_MANY_ARGS;
      }

      clpi->in.fn = strdup(arg);

      if (ends_with(arg, ".jl")) {
        clpi->in.file_type = J_LANG;
      } else if (ends_with(arg, ".jm")) {
        clpi->in.file_type = J_SOURCE;
      } else if (ends_with(arg, ".jb")) {
        clpi->in.file_type = J_BIN;
      } else {
        return CL_WEIRD_FILE_EXTENSION;
      }

    }
  }

  if (J_NONE == clpi->in.file_type) {
    return CL_TOO_FEW_ARGS;
  }

  return CL_SUCCESS;
}

void cl_args_finalize(CL_ProcessInfo *clpi) {
  free(clpi->in.fn);
}

void cl_args_print_summary(CL_ProcessInfo *clpi) {
  printf("Command Line Input Summary:\n");
  printf("  Input File:  '%s'\n", clpi->in.fn);
  printf("  Input Type:  '%s'\n", J_FILE_TYPE[clpi->in.file_type]);
  printf("  Action:      '%s'\n", CL_ACTION[clpi->action]);
  printf("End Summary.\n\n");
  fflush(stdout);
}
