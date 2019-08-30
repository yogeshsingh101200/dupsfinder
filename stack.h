#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

#include "finder.h"

typedef struct stack
{
    bool isParent;
    node *file;
    struct stack *next;
} stack;

stack *top;

int push(node*, bool);
void pop(void);
void print(void);
void empty(void);

#endif