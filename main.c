// POSIX.1-2008 + XSI, i.e. SuSv4, features
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "finder.h"
#include "stack.h"

int main(int argc, char* argv[])
{
    // Checks for correct usage
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./dupsfinder <directory> <delete>(optional)\n");
        return -1;
    }
    
    // Stores directory path
    char *directory = strdup(argv[1]);

    // Initializes hash table
    initialize();

    // Searches directories for file and then loads them to memory    
    if (search(directory) == false)
    {
        // Clears before exiting
        unload();

        exit(-1);
    }      
    
    // Frees memory
    free(directory);

    // Checks and returns duplicate files
    if (check() == false)
    {
        // Clears before exiting
        unload();

        exit(-1);
    }
    
    // Prints all duplicates
    print();

    // Stats
    stats();

    // File Deletion
    if (argc == 3 && strcmp(argv[2], "delete") == 0 && getDuplicates() != 0)
    {
        char choice;
        printf("\n\n!! This action is irreversible !!\n\n");
        printf("Do you really want to delete all duplicate files(Y/n)?\n");
        if (scanf("%c", &choice))
        {
            if (choice == 'Y')
                deleteAll();
            else
                printf("\n Cancelled deletion!\n");
        }
        else
        {
            fprintf(stderr, "Unable to read your input!\n");   
        }
    }

    // Empties stack
    empty();

    // Unloads files from memory
    unload();

    return 0;
}