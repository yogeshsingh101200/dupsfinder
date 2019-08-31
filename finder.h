// Contains declaration of functions defined in finder.c and used in main.c

#ifndef FINDER_H
#define FINDER_H

#include <stdbool.h>

// Maximum path length
#define MAX_PATH 260

// No of buckets in hashtable
#define N 65535

// Structure of a node in hashtable
typedef struct node
{
    long file_size;
    char path[MAX_PATH];
    unsigned long long *xxhash;
    unsigned char *file_hash;
    struct node* next;
} node;

// Hashtable to store directory's entries
node* hashtable[N];

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
unsigned int getDuplicates();

// Gives stats like total duplicates found and size taken by them
void stats(void);

#endif