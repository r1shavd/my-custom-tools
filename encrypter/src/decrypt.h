#ifndef DECRYPT_H
#define DECRYPT_H

#include <stddef.h>

// File decryption functions declarations
int read_encryptedfile(
        const char* filepath,
        char** encoded_hash,
        char** encoded_ciphertext,
        size_t* cipherLength);

void decrypt_payload(
        unsigned char* data,
        size_t dataLength,
        const char* key);
int decrypt_file(const char* filepath, const char* password);

#endif
