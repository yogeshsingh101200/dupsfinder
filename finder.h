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
int search(char*);                    

// Function to load files in memory
bool load(char*);

// Function to find duplicates
void check(void);

// Returns total no of duplicates find
unsigned int duplicates();

// Function to unload directory from memory
void unload(void);

#endif