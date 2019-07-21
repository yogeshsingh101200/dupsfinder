#include <stdio.h>
#include <string.h>

#include "finder.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        //printf("Usage: main <directory>");
        //return -1;
        argv[1] = "../Desktop";
    }
    
    // Stores directory path
    char directory[MAX_PATH];
    strcpy(directory, argv[1]);

    // Initializes hash table
    initialize();

    // Searches directories for file and then loads them to memory
    search(directory);

    // Checks for duplicate files
    check();
    printf("\n\n Total no of duplicates: %u\n", duplicates());
    
    // Unloads directory from memory
    unload();

    return 0;
}