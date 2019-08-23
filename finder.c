#include <stdio.h>
#include <string.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <time.h>

#include "finder.h"
#include "xxhash.h"

// No of buckets in hashtable
#define N 65535

// Structure of a node in hashtable
typedef struct node
{
    off_t file_size;
    char path[MAX_PATH];
    unsigned long long xxhash;
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

// Tracks total no of duplicates
unsigned int duplicates = 0;

// Tracks total size taken by duplicates
off_t dupsSize = 0;

// Record no of calls made to sha256_file()
unsigned int sha256_calls = 0;

// Records total no of bytes read by sha256_file()
long dataRead = 0;

// Records total no of sets in which duplicates are distributed
unsigned int sets = 0;

// Records no of size matched
unsigned int size_matched = 0;

// Record no of calls made to xxhash_file()
unsigned int xxhash_calls = 0;

// Records no of xxhash matched
unsigned int xxhash_matched = 0;

// Record total time taken by sha256_file()
double time_sha256 = 0.0;

// Record total time taken by xxhash_file()
double time_xxhash = 0.0;

// Record toatl time taken by search()
double time_search = 0.0;

// Calculates cpu time 
double calculate(clock_t start, clock_t end)
{
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Calculates sha256 hash of a file
//https://www.openssl.org/docs/manmaster/man3/SHA1.html
//https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c
int sha256_file(char *path, char outputBuffer[HASH_LENGTH + 1])
{
    ++sha256_calls;

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
    const int bufSize = 256 * 1024;
    unsigned char *buffer = malloc(bufSize);
    int bytesRead = 0;
    if (!buffer)
    {
        fprintf(stderr, "Out of memory!\n");
        return 2;
    }
    while ((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        dataRead += bytesRead;
        SHA256_Update(&sha256, buffer, bytesRead);
    }

    // Places message digest to hash
    SHA256_Final(hash, &sha256);

    // Store message digest in lowercase hexadecimal to outputbuffer
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[HASH_LENGTH] = '\0';

    // Closes file
    fclose(file);

    // Free memory
    free(buffer);

    // Indicates success
    return 0;
}

int xxhash_file(char *path, unsigned long long *hash)
{
    ++xxhash_calls;
    
    // Opens file from given path
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s\n", path);
        return 1;
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

void search(const char* dirpath)
{
    int res = ftw(dirpath, fileTree, FOPEN_MAX);
    if (res)
        fprintf(stderr, "Unable to traverse file tree\n");
}

bool load(const char *path, off_t size)
{
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
    file->xxhash = 0;
    file->file_hash = NULL;

    // Index in hashtable
    unsigned int index = file->file_size % N;

    // File insertion
    file->next = hashtable[index];
    hashtable[index] = file;

    // Success
    return true;
}

bool compXXHASH(node *travOut, node *travIn)
{
    clock_t start, end;

    int resO = 0, resI = 0;
    
    // Calculates hash of parent file only if does not exist
    if (!travOut->xxhash)
    {
        start = clock();
        resO = xxhash_file(travOut->path, &travOut->xxhash);
        end = clock();
        time_xxhash += calculate(start, end);
    }

    // Calculates hash of child file only if does not exist
    if (!travIn->xxhash)
    {
        start = clock();
        resI = xxhash_file(travIn->path, &travIn->xxhash);
        end = clock();
        time_xxhash += calculate(start, end);       
    }

    // Comparing the two files, if there hashes are computed, on the basis of sha256 hash 
    if (!resO && !resI)
    {
        if (travOut->xxhash == travIn->xxhash)
        {
            ++xxhash_matched;
            return true;
        }
    }
    return false;
}

void check(void)
{
    clock_t start, end;

    bool isDup;

    // Pointer to traverse through the linked list
    node *travOut = NULL;
    
    // Pointer to traverse through linked list while comparing to the node pointed by travOut
    node *travIn = NULL;
    
    // Pointer to hold the previous node in the linked list
    node *temp = NULL;

    for (int i = 0; i < N; ++i)
    {
        if (hashtable[i])
        {
            isDup = false;

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
                        ++size_matched;
                        if (compXXHASH(travOut, travIn))
                        {
                            int resO = 0, resI = 0;
                            
                            // Calculates hash of parent file only if does not exist
                            if (!travOut->file_hash)
                            {
                                travOut->file_hash = malloc(HASH_LENGTH + 1);
                                if (!travOut->file_hash)
                                {
                                    fprintf(stderr, "Not enough memory!\n");
                                    return;
                                }
                                start = clock();
                                resO = sha256_file(travOut->path, travOut->file_hash);
                                end = clock();
                                time_sha256 += calculate(start, end);
                            }

                            // Calculates hash of child file only if does not exist
                            if (!travIn->file_hash)
                            {
                                travIn->file_hash = malloc(HASH_LENGTH + 1);
                                if (!travIn->file_hash)
                                {
                                    fprintf(stderr, "Not enough memory!\n");
                                    return;
                                }
                                start = clock();
                                resI = sha256_file(travIn->path, travIn->file_hash);    
                                end = clock();
                                time_sha256 += calculate(start, end);   
                            }

                            // Comparing the two files, if there hashes are computed, on the basis of sha256 hash 
                            if (!resO && !resI)
                            {
                                if (strcmp(travIn->file_hash, travOut->file_hash) == 0)
                                {
                                    isDup = true;

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
                            }
                        }
                    }

                    // Moves to next node on list
                    temp = travIn;
                    travIn = temp->next;
                }
                
                // Moves to next node
                travOut = travOut->next;
            }

            // Unloading

            // Pointer to trav linked lists in hashtable
            node* trav = NULL;

            // Pointer to hold the node to be freed
            node* temp = NULL;  

            trav = hashtable[i];
            while(trav)
            {
                temp = trav;
                trav = trav->next;
                free(temp->file_hash);
                free(temp);
            }
            hashtable[i] = NULL;

            if (isDup)
                ++sets;
        }
    }
}

// Returns total no of duplicates
unsigned int getDuplicates(void)
{
    return duplicates;
}

// Returns total size taken by duplicates
void totalSize(void)
{
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

// Benchmarks
void benchmarks(void)
{
    printf("\n No of sets duplicates are distributed: %u\n", sets);
    printf("\n No of size mathced: %u\n", size_matched);
    printf("\n Time taken by search(): %lf\n", time_search);
    printf("\n Time taken by sha256_file(): %lf\n", time_sha256);
    printf("\n No of calls to sha256_file(): %u\n", sha256_calls);
    printf("\n Total bytes read by sha256_file(): %ld\n", dataRead);
    printf("\n Time taken by xxhash_file(): %lf\n", time_xxhash);
    printf("\n No of calls to xxhash_file(): %u\n", xxhash_calls);
    printf("\n No of xxhash matched: %u\n", xxhash_matched);
    printf("\n Total time taken by sha256 and xxhash: %lf\n", time_sha256 + time_xxhash);
}