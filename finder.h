// Contains declaration of functions defined in finder.c and used in main.c

#ifndef FINDER_H
#define FINDER_H

#include <stdbool.h>

// Maximum path length
#define MAX_PATH 260

// Hash length
#define HASH_LENGTH 64

// Initializes hashtable buckets
void initialize(void);

// Function to search directory for files
bool search(const char*);                    

// Function to load files in memory
bool load(const char*, long);

// Function to find duplicates
bool check(void);

// Function to unload files from memory
bool unload(void);

// Gives stats like total duplicates found and size taken by them
void stats(void);

#endif