#ifndef STACKADT_H
#define STACKADT_H

#include <stdbool.h>   /* C99 only */

typedef struct stack_type *Stack;

Stack Stack_create(void);
void Stack_destroy(Stack s);
void Stack_make_empty(Stack s);
bool Stack_is_empty(Stack s);
bool Stack_is_full(Stack s);
void Stack_push(Stack s, void * i);
void * Stack_pop(Stack s);
void *peek(Stack s);
void stack_print(Stack s); //printing function to help debug
#endif
