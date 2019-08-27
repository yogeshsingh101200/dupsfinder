#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stack.h"

int push(node* file, bool flag)
{
    stack *level = malloc(sizeof(stack));
    if (!level)
    {
        fprintf(stderr, "Not enough memory\n");
        return ENOMEM;
    }
    level->isParent = flag;
    level->file = file;
    level->next = top;
    top = level;
    return 0;
}

void pop(void)
{
    if (!top)
        return;
    stack *temp = top;
    top = top->next;
    free(temp);
}

void print(void)
{
    stack *level = top;
    while(level)
    {
        if (level->isParent)
            printf("\n\nDuplicate of %s is at: \n", level->file->path);
        else
            printf("%s\n", level->file->path);
        level = level->next;
    }
}

void deleteAll(void)
{
    stack *trav = NULL, *temp = NULL;
    trav = top;
    while(trav)
    {
        temp = trav;
        trav = trav->next;
        if (!temp->isParent)
            if (!remove(temp->file->path))
                fprintf(stderr, "Unable to remove file %s\n", temp->file->path);
        pop();
    }
    top = NULL;
    printf("\n\nDeleted all duplicate files!\n\n");
}

void empty(void)
{
    while(top)
    {
        pop();
    }
}