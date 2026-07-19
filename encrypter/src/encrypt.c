#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "colors.h"
#include "encrypt.h"
#include "encode_hash.h"

unsigned char* read_plainfile(
        const char* filepath,
        size_t* data_len)
{
    /*
     * @brief   Reads and returns back the data inside plain file
     *
     * @param[const char*] filepath    Path of the file to be read
     * @param[size_t*] data_len        length of the returned file contents
    */

    FILE* file = fopen(filepath, "rb"); 
    if (!file) return NULL;

    size_t capacity = 1024;
    unsigned char *data = malloc(capacity);
    if (!data) {
        fclose(file);
        return NULL;
    }

    *data_len = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (*data_len + 1 >= capacity) {
            capacity *= 2;
            unsigned char *temp = realloc(data, capacity);
            
            if (!temp) {
                free(data);
                fclose(file);
                return NULL;
            }

            data = temp;
        }
        data[(*data_len)++] = (unsigned char)ch;
    }

    fclose(file);
    return data;
}

void encrypt_payload(
        unsigned char* data,
        size_t data_len,
        const char* key)
{
    /*
     * @brief   Converts plain text to cipher text using custom algorithm 
     *          generated via user given password
     *
     * @param[usigned char*] data   the plain text data
     * @param[size_t] data_len      size of data input given
     * @paam[const char*] key       password for encryption
     *
     * @return  There is no return data, this is null return function.
     *          The updated data string is updated via pointer way.
    */
    
    size_t key_len = strlen(key);
    for (size_t i = 0; i < data_len; i++) {
        data[i] = (unsigned char)(((int)data[i] + (int)((unsigned char) key[i % key_len])) % 127);
    }
}

int encrypt_file(const char* filepath, const char* password)
{
    /*
     * @brief   A wrapper around all required encryption functions
     *
     * @description
     *
     * 1. Reads file
     * 2. Generate a cipher key from user given passw
     * 3. Hashes the passw to keep safe
     * 4. Converts the plain data to encrypted data
     *      plain data --> cipher data --> b64 encoding
     * 5. Attaches the hashes passw on top for verification
     *      during decryption process
     * 6. Saves the data to the same file
     *
     * @param[const char*] filepath
     * @param[const char*] password
     *
     * @return     0 in success case
     *            -1 in of error
    */

    size_t data_len = 0;
    unsigned char* data = read_plainfile(filepath, &data_len);
    if (!data) {
        printf("[%s!%s] Cannot open or read file '%s' ]\n", RED, DEFAULT, filepath);
        return -1;
    }

    unsigned long p_hash = hash_DJB2(password);
    char hash_str[64];
    snprintf(hash_str, sizeof(hash_str), "%lu", p_hash);

    size_t encoded_hash_len;
    char* encoded_hash = base64_encode((unsigned char *)hash_str, strlen(hash_str), &encoded_hash_len);
    if (!encoded_hash) {
        printf("[%s!%s]  Memory Allocation Failure\n", RED, DEFAULT);
        free(data);
        return -1;
    }

    // Encrypting the payload data buffer
	// And, then converting to BASE64 encoding
    encrypt_payload(data, data_len, password);
    size_t encoded_cipher_len;
    char* encoded_cipher_text = base64_encode(data, data_len, &encoded_cipher_len);

    if (!encoded_cipher_text) {
        printf("[ Error: Memory Allocation Failure ]\n");
        free(encoded_hash);
		free(data);
        return -1;
    }

	// Saving the output to the file itself
    FILE *out = fopen(filepath, "wb");
    if (!out) {
        printf("[%s!%s] File rewrite lock blocked, run using administrative controls\n", RED, DEFAULT);
        free(encoded_cipher_text);
		free(encoded_hash);
		free(data);
        return -1;
    }
    fprintf(out, "%s\n", encoded_hash);
    fwrite(encoded_cipher_text, 1, encoded_cipher_len, out);
    fclose(out);

    printf("[%s*%s] File '%s' encrypted successfully\n", GREEN, DEFAULT, filepath);

	// Cleaning up the mem space
    memset(data, 0, data_len);
    memset(encoded_hash, 0, encoded_hash_len);
    memset(encoded_cipher_text, 0, encoded_cipher_len);

    free(data);
    free(encoded_hash);
    free(encoded_cipher_text);

    return 0;
}
