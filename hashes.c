//https://www.openssl.org/docs/manmaster/man3/SHA1.html
//https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "xxhash.h"

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
    unsigned char *buffer = malloc(bufSize);
    int bytesRead = fread(buffer, 1, bufSize, file);
    unsigned long long const seed = 0;
    *hash = XXH64(buffer, bytesRead, seed);

    // Closes file
    fclose(file);

    // Free memory
    free(buffer);

    // Indicates success
    return 0;
}