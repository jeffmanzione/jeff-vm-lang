/*
 * command_line.h
 *
 *  Created on: Apr 3, 2016
 *      Author: Jeff
 */

#ifndef COMMAND_LINE_H_
#define COMMAND_LINE_H_

typedef enum {
  CL_SUCCESS, CL_TOO_FEW_ARGS, CL_TOO_MANY_ARGS
} CL_Status;

CL_Status cl_args_process(char *fn_in, char *fn_mid, char *fn_out);
void cl_args_finalize(char *fn_in, char *fn_mid, char *fn_out);

#endif /* COMMAND_LINE_H_ */
