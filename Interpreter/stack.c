/*
 * stack.c
 *
 *  Created on: Dec 21, 2015
 *      Author: Jeff
 */

#include "stack.h"
#include "shared.h"

#include <stdio.h>
#include <stdlib.h>

int stack_init(Stack *stack, size_t capacity) {
  stack->tape = NEW_ARRAY(stack->tape, capacity, Object)
  NULL_CHECK(stack->tape, "Failed to calloc stack!")

  stack->capacity = capacity;
  stack->sp = 0;
  stack->bp = 0;

  return capacity;
}

void stack_finalize(Stack *stack) {
  free(stack->tape);
}

void push_stack(Stack *stack, Object obj) {
  CHECK(stack->capacity == stack->sp, "Stack Overflow!")
  stack->tape[stack->sp++] = obj;
}

Object pop_stack(Stack *stack) {
  //printf("> %d\n", stack->sp);
  //fflush(stdout);
  CHECK(0 == stack->sp, "Stack Underflow!")
  return stack->tape[--stack->sp];
}

Object peek_stack(Stack *stack) {
  CHECK(0 == stack->sp, "Stack Underflow!")
  return stack->tape[stack->sp - 1];
}
