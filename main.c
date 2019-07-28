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
    search(directory);

    // Checks and returns duplicate files
    check();

    // Stats
    printf("\n\n Total no of duplicates: %u", getDuplicates());
    printf("\n Total size taken by duplicate files: %lu Bytes\n\n", getSizeTaken());

    return 0;
}