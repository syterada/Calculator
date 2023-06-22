#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#ifdef IMP
#include "reference_stackADT.h"
#else
#include "stackADT.h"
#endif

#include "memtrace.h"

// maximum size of a single command 
#define MAX_INPUT_SIZE 4096
// maximum size of a operation
#define MAX_OP_SIZE 64


void print_command_help();
int process(char *input, Stack dataStack, Stack opStack);
bool is_int(char *);
int runOperation(char *op, Stack dataStack);
int runCloseParen(Stack dataStack, Stack opStack);
bool higherPriority(char *op1, char *op2);

void error_msg_extraData(char *cmd) {
  printf("ERROR: %s: found data left over!\n", cmd);
}

void error_msg_badCommand(void) {
  printf("ERROR: bad command!\n");
}

void error_msg_opStackNotEmpty(char *cmd) {
  printf("ERROR: %s: not able to process all operations\n", cmd);
}

void error_msg_missingResult(char *cmd) {
  printf("ERROR: %s: no result!\n", cmd);
}

void error_msg_opMissingArgs(char *op) {
  printf("ERROR: op %s: empty stack need two arguments: found none.\n", op);
}

void error_msg_divByZero(void) {
  printf("Error: Division by zero!\n");
}

void error_msg_badOp(char *op) {
  printf("Error: Unrecognized operator: %s!\n", op);
}

int main(int argc, char *argv[]) 
{
  Stack dataStack;
  Stack opStack;
  char *command = NULL;
  int max_input = MAX_INPUT_SIZE;
  int result;
  int runOp;
  int closeParen;


  // PART B: See writup for details.
  // Your job is to implment the calculator taking care of all the dynamic
  // memory management using the stack module you implmented in Part A
  dataStack = Stack_create();
  opStack = Stack_create();

  while (true){
    command = (char *)malloc(sizeof(char) * max_input); // memory for new command

    char *str = fgets(command, max_input, stdin);
    printf("%s", command);

    if (!str || *str == '\n'){
      break;
    }

    int processed = process(command, dataStack, opStack); //Call process on the command.

    if(processed == 0){ //if process did not return an error

      while(!Stack_is_empty(opStack)){
        char *opStackTop;
        opStackTop = Stack_pop(opStack);
        runOp = runOperation(opStackTop, dataStack);
        free(opStackTop);
        
        if(runOp == -1){ //if runOp fails, break
          break;
        }
      }

      if(runOp != -1){ //if runOp did not fail
        if(!Stack_is_empty(opStack)){ //If the opStack is not empty, error
          error_msg_opStackNotEmpty(command);

        }else if(Stack_is_empty(dataStack)){ //if dataStack is empty
          error_msg_missingResult(command);

        }else{
          int *resultptr;
          resultptr = Stack_pop(dataStack);
          result = *resultptr;
          free(resultptr);

          if(!Stack_is_empty(dataStack)){//if data stack still not empty
            error_msg_extraData(command);

          }else{ //if data stack is empty, print result
            printf("= %d\n", result);
          }
        }
      }
    }
  free(command);
  Stack_make_empty(dataStack); //empty stacks for next command
  Stack_make_empty(opStack);
  }
  free(command);

  Stack_destroy(dataStack);
  Stack_destroy(opStack);//destroy stacks at end of program
  return 0;
}

/***********************************************************************
 This is the main skeleton for processing a command string:
   You main task is to get it working by adding the necressary memory
   allocations and deallocations.  The rest of the logic is taken care
   of for you.  See writeup for and explanation of what this function 
   does.
***********************************************************************/
int
process(char *command, Stack dataStack, Stack opStack){
  char delim[] = " ";
  int *data = ((void *)0);
  char *operation = ((void *)0);
  int rc = 0;
  int closeParen;
  void *peeked;

  char* token = strtok(command, delim);
  while (token != ((void *)0)){ //while token is not null

    data = malloc(sizeof(int));
    operation = malloc(sizeof(char));

    if(sscanf(token, "%d", data) == 1){//if the token is a data term, push onto data stack

      Stack_push(dataStack, data);
      data = NULL;
      free(operation);
      operation = NULL;
      
    }else{
      free(data);
      data = NULL;
      sscanf(token, "%c", operation);

      if(*operation == ')'){//if op is close paren, runCloseParen

        closeParen = runCloseParen(dataStack, opStack);
        free(operation);

        if(closeParen == -1){//break if fails
          break;
        }

      }else{
        if(*operation == '('){//if op is open paren, push to opStack
          Stack_push(opStack, operation);
          operation = NULL;

        }else{//op must be an operator
          while(true){

            if(Stack_is_empty(opStack)){//if opStack is empty, break
              break;
            }

            peeked = peek(opStack);

            if(higherPriority(peeked, operation)){//if op on top of opStack
                                                          //has higher priority than token,
                                                          //runOp on old operator
              peeked = Stack_pop(opStack);
              rc = runOperation(peeked, dataStack);
              free(peeked);

              if(rc == -1){//break out of outer while loop if fails
                free(operation);
                goto FAILED;
              }

            }else{//else keep stack the way it was and break
              break;
            }
          }
          Stack_push(opStack, operation); //push new op onto stack
          operation = NULL;
          //stack_print(opStack);
        }
      }
    }
    token = strtok(((void *)0), delim);

  }
  FAILED: if(rc == -1 || closeParen == -1){// If any function call above failed, return an error,
    error_msg_badCommand();
    return -1;
  }

  return 0;
}


int
runCloseParen(Stack dataStack, Stack opStack) {
  int rc = 0;
  char *op = ((void *)0);
  int stack_empty = 0;

  while(true){
    if(Stack_is_empty(opStack)){ //if opStack is empty, break
      stack_empty = 1;
      break;
    }
    op = Stack_pop(opStack);

    if(*op == '('){  //if the top of the opStack is an open parenthesis, break
      free(op);
      break;

    } else{

      rc = runOperation(op, dataStack); //Call runOperation() with what was on top of the opStack
      free(op);

      if(rc == -1){ //break if unsuccessful
        break;
      }
    }
  }

  if(stack_empty || rc == -1){ //return error if we broke out of loop or runOp failed
    return -1;
  }

  return 0;
}

int
getPriority(char* op)
{
  if(*op == '*' || *op == '/') return 2;
  if(*op == '+' || *op == '-') return 1;
  return 0;
}

_Bool
higherPriority(char *oldOp, char *newOp)
{
  return getPriority(oldOp) >= getPriority(newOp);
}

// This function executes the specified operation 
//  It's arguments are the first two values on the data stack
//  You must carefully analyze it and add the necessary code
//  to allocate and deallocte the necessary memory items 
int
runOperation(char *op, Stack dataStack)
{
  int data1;
  int data2;
  int result;
  int *data1ptr;
  int *data2ptr;
  int *resultptr;
  int divide_by_0 = 0;
  int bad_op = 0;

  // Pop the top two terms from the data stack.
  // If this is not possible then print operator missing argument error message and Return -1

  if(Stack_is_empty(dataStack)){
    error_msg_opMissingArgs(op);
    return -1;
  }

  data1ptr = Stack_pop(dataStack);
  data1 = *data1ptr;

    if(Stack_is_empty(dataStack)){
    error_msg_opMissingArgs(op);
    free(data1ptr);
    return -1;
  }

  data2ptr = Stack_pop(dataStack);
  data2 = *data2ptr;

  if(*op == '+'){ //addition
    result = data2 + data1;

  } else if(*op == '*'){ //multiplication
    result = data2 * data1;

  }else if(*op == '/'){ //division

    if(data1 == 0){ //error if trying to divide by 0
      divide_by_0 = 1;
      goto FAILED_RUNOP;
    }

    result = data2 / data1;

  }else if(*op == '-'){ //subtraction
    result = data2 - data1;

  } else{ //there was not an operation inputted, return error
    bad_op = 1;
    goto FAILED_RUNOP;
  }

  resultptr = (int *)malloc(sizeof(int));
  *resultptr = result;
  Stack_push(dataStack, resultptr); //push result to stack and return 0
  free(data1ptr);
  free(data2ptr);

  FAILED_RUNOP: if(divide_by_0){
    free(data1ptr);
    free(data2ptr);
    error_msg_divByZero();
    return -1;
  }else if(bad_op){
    free(data1ptr);
    free(data2ptr);
    error_msg_badOp(op);
    return -1;
  }
  

  return 0;
}
