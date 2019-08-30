#include <ftw.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#include "finder.h"
#include "hashes.h"
#include "stack.h"

// Intializes hashtable
void initialize(void)
{
    for (int i = 0; i < N; ++i)
    {
        hashtable[i] = NULL;
    }
}

// Tracks total no of duplicates
unsigned int duplicates = 0;

// Tracks total size taken by duplicates
long dupsSize = 0;

// Total no of files
unsigned int no_of_files = 0;

static inline void progress(unsigned int processed_files)
{
    printf("\r[%u/%u] files checked ||  \b[%u/%u] duplicates found.", processed_files, no_of_files, duplicates, processed_files);
    fflush(stdout);
}

static bool load(const char *path, long size)
{
    // Allocating memory to store file info
    node* file = malloc(sizeof(node));
    if (!file)
    {
        fprintf(stderr, "Not enough memory to load file!\n");
        return false;
    }

    // Storing file info
    file->file_size = size;
    strcpy(file->path, path);
    file->xxhash = NULL;
    file->file_hash = NULL;

    // Index in hashtable
    unsigned int index = file->file_size % N;

    // File insertion
    file->next = hashtable[index];
    hashtable[index] = file;

    // Success
    return true;
}

static int fileTree(const char *fpath, const struct stat *sb, int typeflag)
{
    if (typeflag == FTW_F)
    {
        ++no_of_files;
        if (!load(fpath, sb->st_size))
        {
            fprintf(stderr, "Unable to load file at %s\n", fpath);
            return -1;
        }
    }
    else if (typeflag == FTW_DNR)
    {
        fprintf(stderr, "Unable to read %s\n", fpath);
    }
    return 0;
}

bool search(const char* dirpath)
{
    if (ftw(dirpath, fileTree, FOPEN_MAX))
    {
        fprintf(stderr, "Unable to traverse file tree\n");
        return false;
    }
    return true;
}

static int compxxhash(node *travOut, node *travIn)
{
    int resO = 0, resI = 0;
    
    // Calculates hash of parent file only if does not exist
    if (!travOut->xxhash)
    {
        travOut->xxhash = malloc(sizeof(unsigned long long));
        if (!travOut->xxhash)
        {
            fprintf(stderr, "Not enough memory!\n");
            return ENOMEM;    
        }
        resO = xxhash_file(travOut->path, travOut->xxhash);
    }

    // Calculates hash of child file only if does not exist
    if (!travIn->xxhash)
    {
        travIn->xxhash = malloc(sizeof(unsigned long long));
        if (!travIn->xxhash)
        {
            fprintf(stderr, "Not enough memory!\n");
            return ENOMEM;    
        }
        resI = xxhash_file(travIn->path, travIn->xxhash);  
    }

    // Comparing the two files, if there hashes are computed, on the basis of sha256 hash 
    if (!resO && !resI)
    {
        if (*travOut->xxhash == *travIn->xxhash)
        {
            return 0;
        }
    }
    return -1;
}

static int compsha256(node * travOut, node *travIn)
{
    int resO = 0, resI = 0; 

    // Calculates hash of parent file only if does not exist
    if (!travOut->file_hash)
    {
        travOut->file_hash = malloc(SHA256_DIGEST_LENGTH);
        if (!travOut->file_hash)
        {
            fprintf(stderr, "Not enough memory!\n");
            return ENOMEM;
        }
        resO = sha256_file(travOut->path, travOut->file_hash);
        if (resO == ENOMEM)
            return ENOMEM;
    }

    // Calculates hash of child file only if does not exist
    if (!travIn->file_hash)
    {
        travIn->file_hash = malloc(SHA256_DIGEST_LENGTH);
        if (!travIn->file_hash)
        {
            fprintf(stderr, "Not enough memory!\n");
            return ENOMEM;
        }
        resI = sha256_file(travIn->path, travIn->file_hash);      
        if (resI == ENOMEM)
            return ENOMEM; 
    }

    // Comparing the two files, if there hashes are computed, on the basis of sha256 hash 
    if (!resO && !resI)
    {
        if (memcmp(travOut->file_hash, travIn->file_hash, SHA256_DIGEST_LENGTH) == 0)
            return 0;
    }
    return -1;
}

bool check(void)
{
    // Pointers to traverse through the linked list
    node *travOut = NULL, *travIn = NULL;

    int result = -1;

    unsigned int processed_files = 0;

    for (int i = 0; i < N; ++i)
    {
        if (hashtable[i])
        {
            // Checking
            travOut = hashtable[i];

            // Till the end of linked list
            while(travOut)
            {
                progress(++processed_files);

                bool isDup = false;

                // To traverse remaining nodes in linked list
                travIn = travOut->next;

                // Until end of list
                while(travIn)
                {
                    // Groups files on the basis of file size
                    if (travOut->file_size == travIn->file_size && travOut->file_size != -1 && travIn->file_size != -1)
                    {
                        if ((result = compxxhash(travOut, travIn)) == 0)
                        {
                            if ((result = compsha256(travOut, travIn)) == 0)
                            {
                                isDup = true;
                                push(travIn, false);
                                ++duplicates;
                                dupsSize += travIn->file_size;

                                // Removing duplicate file from comparison
                                travIn->file_size = -1;
                            }
                            else if (result == ENOMEM)
                            {
                                return false;
                            }
                        }
                        else if (result == ENOMEM)
                        {
                            return false;
                        }
                    }

                    // Moves to next node on list
                    travIn = travIn->next;
                }
                
                if (isDup)
                    push(travOut, true);

                // Moves to next node
                travOut = travOut->next;
            }
        }
    }
    return true;
}

void deleteAll(void)
{
    stack *trav = NULL, *temp = NULL;
    trav = top;
    while(trav)
    {
        temp = trav;
        trav = trav->next;
        if (!temp->isParent)
            if (remove(temp->file->path) == -1)
            {
                fprintf(stderr, "\nUnable to remove file %s\n", temp->file->path);
                fprintf(stderr, "Error: %s\n", strerror(errno));
            }
        pop();
    }
    top = NULL;
    printf("\n\nDeleted all duplicate files!\n\n");
}

bool unload(void)
{
    // Pointer to trav linked lists in hashtable
    node* trav = NULL;

    // Pointer to hold the node to be freed
    node* temp = NULL;  

    for (int i = 0; i < N; ++i)
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

    return true;
}

void stats(void)
{
    // Total no of duplicates
    printf("\n\n Total no of duplicates: %u", duplicates);
    
    // Total size calculations
    const unsigned int KB = 1024;
    const unsigned int MB = 1024 * 1024;
    const unsigned int GB = 1024 * 1024 * 1024;

    if (dupsSize >= GB)
    {
        printf("\n Total space taken by duplicates: %.02lf GB\n", (double)dupsSize / GB);
    }
    else if (dupsSize >= MB)
    {
        printf("\n Total space taken by duplicates: %.02lf MB\n", (double)dupsSize / MB);
    }
    else if (dupsSize >= KB)
    {
        printf("\n Total space taken by duplicates: %.02lf KB\n", (double)dupsSize / KB);
    }
    else
    {
        printf("\n Total space taken by duplicates: %ld Bytes\n", dupsSize);
    }   
}