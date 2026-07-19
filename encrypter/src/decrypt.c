#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "colors.h"
#include "encode_hash.h"

// Global character map lookup for decoding base64
//static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int read_encryptedfile(
        const char* filepath,
        char** encoded_hash,
        char** encoded_ciphertext,
        size_t* cipher_len)
{
    /*
     * @brief   Reads encrypted file and return data separated from passw hash
     *
     * @param[const char*] filepath         Location of encrypted file
     * @param[char**] encoded_hash          stores hash passw in line 1
     * @param[char**] encoded_ciphertext    stores encrypted txt in line 2
     * @param[size_t*] cipher_len           stores length of encrypted txt
     *
     * @return          -1 in error case
     *                  0 in success case
    */
	
    FILE* file = fopen(filepath, "rb");
    if (!file) {
		// If the file fails to open for any reason

		printf("[%s!%s] File failed to open. %s.\n", RED, DEFAULT, strerror(errno));
		return -1;
	}

	// Reading the verify first line and decoding from BASE64 to normal (passphrase)
    size_t hashcapture = 64;
    *encoded_hash = malloc(hashcapture);
    if (!*encoded_hash) {
		printf("[%s!%s] Failed to allocate memory\n", RED, DEFAULT);
		fclose(file);
		return -1;
	}

    size_t hashLength = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF && ch != '\n' && ch != '\r') {
        if (hashLength + 1 >= hashcapture) {
            hashcapture *= 2;
            char *temp = realloc(*encoded_hash, hashcapture);
            if (!temp) {
				printf("[%s!%s] Failed to allocate memory\n", RED, DEFAULT);
				free(*encoded_hash);
				fclose(file);
				return -1;
			}
            *encoded_hash = temp;
        }
        (*encoded_hash)[hashLength++] = (char)ch;
    }
    (*encoded_hash)[hashLength] = '\0';	// Adding escape seq at end of str

    // Skipping carriage returns if file was edited on Windows (\r\n)
    if (ch == '\r') {
        ch = fgetc(file); 
        if (ch != '\n' && ch != EOF) ungetc(ch, file);
    }

    // Reading the 2nd line to extract BASE64 ciphertext block
    size_t ciphercapture = 1024;
    *encoded_ciphertext = malloc(ciphercapture);
    if (!*encoded_ciphertext) { free(*encoded_hash); fclose(file); return -2; }

    *cipher_len = 0;
    while ((ch = fgetc(file)) != EOF) {
        if (*cipher_len + 1 >= ciphercapture) {
            ciphercapture *= 2;
            char *temp = realloc(*encoded_ciphertext, ciphercapture);
            if (!temp) {
				printf("[%s!%s] Failed to allocate memory\n", RED, DEFAULT);
				free(*encoded_hash);
				free(*encoded_ciphertext);
				fclose(file);
				return -1;
			}
            *encoded_ciphertext = temp;
        }
        (*encoded_ciphertext)[*cipher_len] = (char)ch;
        (*cipher_len)++;
    }
    (*encoded_ciphertext)[*cipher_len] = '\0';

    fclose(file);
    return 0;
}

void decrypt_payload(
        unsigned char* data,
        size_t data_len,
        const char* key)
{
    /*
     * @brief   Decrypts the cipher text payload using the key
     *
     * @param[unsigned char*] data  The cipher text
     * @param[size_t]  data_len     The size of cipher text
     * @param[const char*] key      The user given passw
     *
     * @return      This function returns NULL, and stores the decrypted text
     *              back in memloc of `data`. 
    */

	size_t key_len = strlen(key);
	for (size_t i = 0; i < data_len; i++) {
        int val = (int) data[i] - (int)((unsigned char) key[i % key_len]);
        // Handling standard modulo math wrapper for -ve outputs in C

        if (val < 0) {
            val = (val % 127) + 127;
        }
        data[i] = (unsigned char)(val % 127);
    }
}

int decrypt_file(const char* filepath, const char* password)
{
	/*
     * @brief   Wrapper function around all decryption functions
     *
     * @description
     *
     * 1. Hashes the user given passw
     * 2. Extracts the details from encrpted file
     * 3. Checks user given passw hash with original stored in file
     * 4. Decrypts the file according to the passw
     * 5. Saves the plain text contents back to source
     *
     * @param[const char*] filepath     File to decrypt
     * @param[const char*] password     Password of encryption of file
     *
     * @return      -1 in ERROR case
     *               0 in SUCCESS case
     *
	*/

    char* encoded_hash = NULL;
    char* encoded_ciphertext = NULL;
    size_t encoded_ciphertext_len = 0;

    // Filling the data arrays from file lines
    int fileStatus = read_encryptedfile(filepath, &encoded_hash, &encoded_ciphertext, &encoded_ciphertext_len);
    if (fileStatus != 0) {
        printf("[%s!%s] Failed to read the file\n", RED, DEFAULT);
        return -1;
    }

    // Decoding file - password verification hash (from first line)
    size_t decoded_hash_len;
    unsigned char* decoded_hash = base64_decode(encoded_hash, strlen(encoded_hash), &decoded_hash_len);
    if (!decoded_hash) {
        free(encoded_hash);
		free(encoded_ciphertext);
        return -1;
    }

    // Converting string hash token back into numerical long value
    unsigned long stored_hash = strtoul((char *)decoded_hash, NULL, 10);
    unsigned long input_hash = hash_DJB2(password);

    // Validating the user entered password 
    if (stored_hash != input_hash) {
        printf("[%s!%s] Access Denied. Invalid Password\n", RED, DEFAULT);
        memset(decoded_hash, 0, decoded_hash_len);
        free(decoded_hash);
		free(encoded_hash);
		free(encoded_ciphertext);
        return -1;
    }

    // Decoding the cipher text (stored at 2nd line)
    size_t cipher_text_len;
    unsigned char *ciphertext = base64_decode(encoded_ciphertext, encoded_ciphertext_len, &cipher_text_len);
    if (!ciphertext) {
        printf("[%s!%s] Ciphertext content payload is corrupted\n", RED, DEFAULT);
        free(decoded_hash);
		free(encoded_hash);
		free(encoded_ciphertext);
        return -1;
    }

    // Reversing the mathematical adjustments in the payload
    decrypt_payload(ciphertext, cipher_text_len, password);

    // Overwriting target file layout cleanly with original structure
    FILE *out = fopen(filepath, "wb");
    if (!out) {
        printf("[%s!%s] Failed to write lock blocked, run using sudo controls. %s\n", 
                RED, DEFAULT,
                strerror(errno));
        free(ciphertext);
		free(decoded_hash);
		free(encoded_hash);
		free(encoded_ciphertext);
        return -1;
    }
    fwrite(ciphertext, 1, cipher_text_len, out);
    fclose(out);

    printf("[%s*%s] File '%s' decrypted successfully\n", GREEN, DEFAULT, filepath);

    // Cleaning up memory
    memset(ciphertext, 0, cipher_text_len);
    memset(decoded_hash, 0, decoded_hash_len);

    free(ciphertext);
    free(decoded_hash);
    free(encoded_hash);
    free(encoded_ciphertext);

    return 0;
}
