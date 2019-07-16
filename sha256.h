//https://www.openssl.org/docs/manmaster/man3/SHA1.html
//https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c

#include <openssl/sha.h>
#include <stdlib.h>

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
    while((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }

    // Places message digest to hash
    SHA256_Final(hash, &sha256);

    // Store message digest in lowercase hexadecimal to outputbuffer
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = '\0';

    // Closes file
    fclose(file);

    // Free memory
    free(buffer);

    // Indicates succes
    return 0;
}