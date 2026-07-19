#ifndef ENCODE_HASH_H
#define ENCODE_HASH_H

#include <stddef.h>

// BASE64 functions declarations
//static int b64_index(char c);
char* base64_encode(const unsigned char* data, size_t in_len, size_t* out_len);
unsigned char* base64_decode(const unsigned char* data, size_t in_len, size_t* out_len);

// Hashing functions declarations (crypto + non-crypto)
unsigned long hash_DJB2(const char* str);
void hash_SHA256(const char* str, unsigned char* result);

#endif
