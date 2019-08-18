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
void search(char*);                    

// Function to load files in memory
bool load(char*, off_t);

// Function to find duplicates
void check(void);

// Returns total no of duplicates
unsigned int getDuplicates();

// Returns total size taken by duplicate files
void totalSize(void);

// Benchmarks
void benchmarks(void);

#endif