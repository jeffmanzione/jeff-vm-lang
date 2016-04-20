/*
 * command_line.h
 *
 *  Created on: Apr 3, 2016
 *      Author: Jeff
 */

#ifndef COMMAND_LINE_H_
#define COMMAND_LINE_H_

typedef enum {
  CL_SUCCESS,
  CL_TOO_FEW_ARGS,
  CL_TOO_MANY_ARGS,
  CL_UNKNOWN_FLAG,
  CL_WEIRD_FILE_EXTENSION
} CL_Status;

typedef struct {
  enum {
    J_NONE, J_LANG, J_SOURCE, J_BIN,
  } file_type;
  char *fn;
} JLang_Info;

extern char *J_FILE_TYPE[];

typedef enum {
  NOTHING, // 0000
  WRITE_SRC, // 1000
  WRITE_BIN,         // 0100
  WRITE_SRC_AND_BIN, // 1100
  CREATE_LIB,                       // 0010
  WRITE_SRC_AND_CREATE_LIB,         // 1010
  WRITE_BIN_AND_CREATE_LIB,         // 0110
  WRITE_SRC_AND_BIN_AND_CREATE_LIB, // 1110
  EXECUTE,                                      // 0001
  WRITE_SRC_AND_EXECUTE,                        // 1001
  WRITE_BIN_AND_EXECUTE,                        // 0101
  WRITE_SRC_AND_BIN_AND_EXECUTE,                // 1101
  CREATE_LIB_AND_EXECUTE,                       // 0011
  WRITE_SRC_AND_CREATE_LIB_AND_EXECUTE,         // 1011
  WRITE_BIN_AND_CREATE_LIB_AND_EXECUTE,         // 0111
  WRITE_SRC_AND_BIN_AND_CREATE_LIB_AND_EXECUTE, // 1111
} CL_Action;

extern char *CL_ACTION[];

typedef struct {
  JLang_Info in;
  CL_Action action;
} CL_ProcessInfo;

CL_Status cl_args_process(CL_ProcessInfo *clpi, int argc, char **argv);
void cl_args_finalize(CL_ProcessInfo *clpi);
void cl_args_print_summary(CL_ProcessInfo *clpi);

#endif /* COMMAND_LINE_H_ */
