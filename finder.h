// Contains declaration of functions defined in finder.c and used in main.c

#ifndef FINDER_H
#define FINDER_H

#include <stdbool.h>

// Initializes hashtable buckets
void initialize(void);

// Function to search directory for files
bool search(const char*);                    

// Function to find duplicates
bool check(void);

// Deletes all duplicate files
void deleteAll(void);

// Function to unload files from memory
bool unload(void);

// Return total no of duplicates
unsigned int getDuplicates(void);

// Gives stats like total duplicates found and size taken by them
void stats(void);

#endif