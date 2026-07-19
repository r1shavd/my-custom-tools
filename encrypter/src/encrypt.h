#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stddef.h>

// File encryption function declaration
unsigned char* read_plainfile(const char* filepath, size_t* data_len);

void encrypt_payload(unsigned char* data, size_t data_len, const char* key); 

int encrypt_file(const char* filepath, const char* password);

#endif
