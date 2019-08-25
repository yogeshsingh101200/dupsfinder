#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#include "finder.h"
#include "xxhash.h"

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

// Calculates sha256 hash of a file
//https://www.openssl.org/docs/manmaster/man3/SHA1.html
//https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c
int sha256_file(char *path, unsigned char *hash)
{
    // Opens file from given path
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s\n", path);
        return ENOENT;
    }
    
    // Intializes sha256 context structure
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    // To read chunks of data repeatedly and feed them to sha256 update to hash
    const int bufSize = 256 * 1024;
    unsigned char *buffer = malloc(bufSize);
    int bytesRead = 0;
    if (!buffer)
    {
        fprintf(stderr, "Out of memory!\n");
        return ENOMEM;
    }
    while ((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }

    // Places message digest to hash
    SHA256_Final(hash, &sha256);

    // Closes file
    fclose(file);

    // Free memory
    free(buffer);

    // Indicates success
    return 0;
}

// Calculates xxhash of a file
int xxhash_file(char *path, unsigned long long *hash)
{    
    // Opens file from given path
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s\n", path);
        return -1;
    }

    const int bufSize = 2048;
    unsigned char buffer[bufSize];
    int bytesRead = fread(buffer, 1, bufSize, file);
    unsigned long long const seed = 0;
    *hash = XXH64(buffer, bytesRead, seed);

    // Closes file
    fclose(file);

    // Indicates success
    return 0;
}

int fileTree(const char *fpath, const struct stat *sb, int typeflag)
{
    if (typeflag == FTW_F)
    {
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

bool load(const char *path, long size)
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

int compxxhash(node *travOut, node *travIn)
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

int compsha256(node * travOut, node *travIn)
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
        if (memcmp(travIn->file_hash, travOut->file_hash, SHA256_DIGEST_LENGTH) == 0)
            return 0;
    }
    return -1;
}

bool check(void)
{
    // Pointer to traverse through the linked list
    node *travOut = NULL;
    
    // Pointer to traverse through linked list while comparing to the node pointed by travOut
    node *travIn = NULL;
    
    // Pointer to hold the previous node in the linked list
    node *temp = NULL;

    int result = -1;

    for (int i = 0; i < N; ++i)
    {
        if (hashtable[i])
        {
            // Checking
            travOut = hashtable[i];

            // Till the end of linked list
            while(travOut)
            {
                // turn is to print 'duplicate of' only 1 time
                int turn = 1;

                // To hold the previous node
                temp = travOut;

                // To traverse remaining nodes in linked list
                travIn = travOut->next;

                // Until end of list
                while(travIn)
                {
                    // Groups files on the basis of file size
                    if (travOut->file_size == travIn->file_size)
                    {
                        if ((result = compxxhash(travOut, travIn)) == 0)
                        {
                            if ((result = compsha256(travOut, travIn)) == 0)
                            {
                                if (turn)
                                    printf("\n\nDuplicate(s) of %s is at:\n", travOut->path);
                                printf("%s\n", travIn->path);
                                ++duplicates;
                                dupsSize += travIn->file_size;

                                // Removing duplicate file
                                temp->next = travIn->next;
                                free(travIn);
                                travIn = temp->next;
                                turn = 0;
                                continue;
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
                    temp = travIn;
                    travIn = temp->next;
                }
                
                // Moves to next node
                travOut = travOut->next;
            }
        }
    }
    return true;
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