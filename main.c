// POSIX.1-2008 + XSI, i.e. SuSv4, features
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "finder.h"
#include "stack.h"

void help(void);

int main(int argc, char* argv[])
{
    // Checks for correct usage
    if (argc < 2)
    {
        help();
        return -1;
    }

    // Flag to know whether to delete files or not
    bool isDelete = false;

    // Parses arguments and form corresponding options
    int opt;
    bool called = false; // To avoid multiple calls to help()
    while ((opt = getopt(argc, argv, "dh")) != -1)
    {
        switch (opt)
        {
            case 'd': isDelete = true;
                break;        
            case 'h': help();
                return 0;
            default: help();
                called = true;
                break;
        }
    }

    // Checks whether list of directories supplied or not
    if (!argv[optind])
    {
        fprintf(stderr, "\n No directories specified!\n");
        if (!called) help();
        return -1;
    }
    
    // Initializes hash table
    initialize();

    for (int i = optind; i < argc; ++i)
    {
        // Stores directory path
        char *directory = strdup(argv[i]);

        // Searches directories for file and then loads them to memory    
        if (search(directory) == false)
        {
            // Clears before exiting
            unload();

            exit(-1);
        }      
        
        // Frees memory
        free(directory);
    }

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
    if (isDelete == true && getDuplicates() != 0)
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

void help(void)
{
    printf("\n Usage: ./dupsfinder <directory list> <options>\n");
    printf("\n Options:\n\n");
    printf("\t -h : to print this help guide\n");
    printf("\t -d : delete the duplicate files, retaining the first one in each group\n\n");
}
