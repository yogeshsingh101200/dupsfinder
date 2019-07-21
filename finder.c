#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>

#include "finder.h"
#include "sha256.h"


// No of buckets in hashtable
#define N 100000

// Structure of a node in hashtable
typedef struct node
{
    char path[MAX_PATH];
    char file_hash[HASH_LENGTH + 1];
    struct node* next;
} node;

// Hashtable to store directory's entries
node* hashtable[N];

// Intializes hashtable
void initialize(void)
{
    for (int i = 0; i < N; ++i)
    {
        hashtable[i] = NULL;
    }
}

// Ref: https://www.reddit.com/r/cs50/comments/1x6vc8/pset6_trie_vs_hashtable/
unsigned int hash(const char *word)
{
    unsigned int hash = 0;
    for (int i = 0, n = strlen(word); i < n; ++i)
    {
        hash = (hash << 2) ^ (tolower(word[i]));
    }
    return hash % N;
}

// Tracks total no of duplicates
unsigned int totDuplicates = 0;

int search(char* path)
{
    // To store path of subdirectory
    char newpath[MAX_PATH];
    
    // Opens directory
    DIR* dir = opendir(path);

    // Validates
    if (!dir)
        return 1;

    // To read directory
    struct dirent* d = readdir(dir);
    while (d)
    {
        strcpy(newpath, path);
        strcat(newpath, "/");
        if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0)
        {
            strcat(newpath, d->d_name);
            if (search(newpath))
            {
                if (!load(newpath))
                {
                    fprintf(stderr, "Unable to load file at %s\n", newpath);
                }
            }
        }
        
        // Next entry
        d = readdir(dir);
    }

    // Closes directory
    closedir(dir);

    return 0;
}

bool load(char* path)
{
    // Stores file hash
    char file_hash[HASH_LENGTH + 1];

    // Calculates sha256 hash of file
    int res = sha256_file(path, file_hash);
    if (res)
        return false;

    // Getting hash of file_hash
    unsigned int index = hash(file_hash);

    // Allocating memory to store file info
    node* file = malloc(sizeof(node));

    // Storing file info    
    strcpy(file->path, path);
    strcpy(file->file_hash, file_hash);
    file->next = NULL;

    // For first file insertion
    if (!hashtable[index])
    {
        hashtable[index] = file;
    }

    // For other insertions
    else
    {
        file->next = hashtable[index];
        hashtable[index] = file;
    }
    return true;
}

void check(void)
{
    node *travOut = NULL, *travIn = NULL, *temp = NULL;
    for (int i = 0; i < N; ++i)
    {
        if (hashtable[i])
        {
            travOut = hashtable[i];
            while(travOut)
            {
                int sno = 0, turn = 1;
                temp = travOut;
                travIn = travOut->next;
                while(travIn)
                {
                    if (strcmp(travIn->file_hash, travOut->file_hash) == 0)
                    {
                        ++sno;
                        if (turn)
                            printf("\n\n Duplicate(s) of %s is at:\n", travOut->path);
                        printf(" %d) %s\n", sno, travIn->path);
                        ++totDuplicates;
                        temp->next = travIn->next;
                        free(travIn);
                        travIn = temp->next;
                        turn = 0;
                    }
                    else
                    {
                        temp = travIn;
                        travIn = travIn->next;
                    }
                }
                travOut = travOut->next;
            }
        }
    }
}

unsigned int duplicates(void)
{
    // Returns the total no of duplicates found
    return totDuplicates;
}

void unload(void)
{
    // Pointer to trav linked lists in hashtable
    node* trav = NULL;

    // Pointer to hold the node to be freed
    node* temp = NULL;
    for (int i = 0; i < N; ++i)
    {
        if (hashtable[i])
        {
            trav = hashtable[i];
            while(trav)
            {
                temp = trav;
                trav = trav->next;
                free(temp);
            }
            hashtable[i] = NULL;
        }
    }
}