#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <stdlib.h>

#include "finder.h"


// No of buckets in hashtable
#define N 100000

// Structure of a node in hashtable
typedef struct node
{
    long file_size;
    char path[MAX_PATH];
    char* file_hash;
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

// Returns index for hashtable for given size
unsigned int hash(long size)
{
    return size % N;
}

// Tracks total no of duplicates
unsigned int totDuplicates = 0;

// Tracks total size taken by duplicates
unsigned long sizeTaken = 0;

// Returns total file size in bytes
long size_of_file(char path[])
{
    // Opens file in binary mode to read
    FILE *file = fopen(path, "rb");

    // Handles unsuccessfull opening of file
    if (!file)
        return -1;

    // Moves file pointer to end
    fseek(file, 0, SEEK_END);
    
    // Returns file size
    return ftell(file);
}

// Calculates sha256 hash of a file
//https://www.openssl.org/docs/manmaster/man3/SHA1.html
//https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c

int sha256_file(char *path, char outputBuffer[65])
{
    // Opens file from given path
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s\n", path);
        return 1;
    }

    // To store digest
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    // Intializes sha256 context structure
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    // To read chunks of data repeatedly and feed them to sha256 update to hash
    const int bufSize = 32768;
    unsigned char *buffer = malloc(bufSize);
    int bytesRead = 0;
    if (!buffer)
    {
        fprintf(stderr, "Out of memory!\n");
        return 2;
    }
    while ((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }

    // Places message digest to hash
    SHA256_Final(hash, &sha256);

    // Store message digest in lowercase hexadecimal to outputbuffer
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[HASH_LENGTH - 1] = '\0';

    // Closes file
    fclose(file);

    // Free memory
    free(buffer);

    // Indicates succes
    return 0;
}

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

bool load(char *path)
{
    // Find and store file size
    long size = size_of_file(path);

    if (size == -1)
        return false;

    // Getting hash out of file size
    unsigned int index = hash(size);

    // Allocating memory to store file info
    node* file = malloc(sizeof(node));
    if (!file)
    {
        fprintf(stderr, "Not enough memory!\n");
        return false;
    }

    // Storing file info
    file->file_size = size;
    strcpy(file->path, path);
    file->file_hash = NULL;
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
    // Pointer to traverse through the linked list
    node *travOut = NULL;
    
    // Pointer to traverse through linked list while comparing to the node pointed by travOut
    node *travIn = NULL;
    
    // Pointer to hold the previous node in the linked list
    node *temp = NULL;

    // Checking
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
                    if (travIn->file_size == travOut->file_size)
                    {
                        int resO = 0, resI = 0;
                        if (!travOut->file_hash)
                        {
                            travOut->file_hash = malloc(HASH_LENGTH + 1);
                            if (!travOut->file_hash)
                            {
                                fprintf(stderr, "Not enough memory!\n");
                                return;
                            }
                            resO = sha256_file(travOut->path, travOut->file_hash);
                            if (resO)
                            {
                                fprintf(stderr, "Unable to compute file hash of %s!\n", travOut->path);
                                break;
                            }
                        }
                        if (!travIn->file_hash)
                        {
                            travIn->file_hash = malloc(HASH_LENGTH + 1);
                            if (!travIn->file_hash)
                            {
                                fprintf(stderr, "Not enough memory!\n");
                                return;
                            }
                            resI = sha256_file(travIn->path, travIn->file_hash);       
                            if (resI)
                                fprintf(stderr, "Unable to compute file hash of %s!\n", travIn->path);
                        }
                        if (!resO && !resI)
                        {
                            if (strcmp(travIn->file_hash, travOut->file_hash) == 0)
                            {
                                ++sno;
                                if (turn)
                                    printf("\n\n Duplicate(s) of %s is at:\n", travOut->path);
                                printf(" %d) %s\n", sno, travIn->path);
                                ++totDuplicates;
                                sizeTaken += travIn->file_size;
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
                        else
                        {
                            temp = travIn;
                            travIn = travIn->next;
                        }
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

// Returns total no of duplicates found
unsigned int duplicates(void)
{
    return totDuplicates;
}

// Returns total size taken by duplicates
unsigned long size_taken(void)
{
    return sizeTaken;
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
                free(temp->file_hash);
                free(temp);
            }
            hashtable[i] = NULL;
        }
    }
}