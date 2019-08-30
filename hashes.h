#ifndef HASHES_H
#define HASHES_H

// Calculates sha256 of a file
int sha256_file(char *path, unsigned char *hash);

// Calculates xxhash of a buffer
int xxhash_file(char *path, unsigned long long *hash);

#endif