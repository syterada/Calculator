# Calculator
Calculator implementation using dynamic memory allocation

Implementation in calculator.c. Operator takes input in in-fix notation and requires a space between terms and operators (e.g. "5 + 2" is acceptable, "5+2" is not), spaces are also required between numbers and parenthesis (e.g "( 5 + 2 )"). 
Calculator uses dynamic memory allocation by using malloc to push items onto a computation stack and later freeing them once they have been popped off the stack. 
