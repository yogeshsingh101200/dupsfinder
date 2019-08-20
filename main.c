#include <stdio.h>
#include <time.h>
#include <string.h>

#include "finder.h"

int main(int argc, char* argv[])
{
    // Checks for correct usage
    if (argc != 2)
    {
        fprintf(stderr, "Usage: main <directory>\n");
        return -1;
    }
    
    // Stores directory path
    char directory[MAX_PATH];
    strcpy(directory, argv[1]);

    // Initializes hash table
    initialize();

    // Searches directories for file and then loads them to memory    
    // Time taken by search()
    extern double time_search;
    clock_t start = clock();
    search(directory);
    clock_t end = clock();
    time_search = calculate(start, end);

    // Checks and returns duplicate files
    check();

    // Benchmarks
    benchmarks();
    
    // Stats
    printf("\n\n Total no of duplicates: %u", getDuplicates());
    totalSize();

    return 0;
}