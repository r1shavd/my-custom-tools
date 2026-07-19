#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <openssl/evp.h>

#include "encode_hash.h"

// Setting a global character map lookup for BASE64 encoding
static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64_index(char c)
{
	/*
     * @brief   This function is a quick inverse lookup table for BASE64 decoding.
     *
     * @param[char] c  Character to look up for
     *
     * @return          Index for the character to be replaced
	*/

    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
    if (c >= '0' && c <= '9')
        return c - '0' + 52;
    if (c == '+')
        return 62;
    if (c == '/')
        return 63;
    return -1;
}

char* base64_encode(const unsigned char* data, size_t in_len, size_t* out_len)
{
	/*
     * @brief   This function encodes a given data into BASE64 encoding.
     *
     * @param data              string data to be encoded
     * @param[size_t] in_len    input data length
     * @param[size_t*] out_len  output data length pointer
     *
     * @return[char*]           The BASE64 encoded data
     *
    */

    *out_len = 4 * ((in_len + 2) / 3);
    char *encoded_data = malloc(*out_len + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < in_len;) {
        uint32_t octet_a = i < in_len ? data[i++] : 0;
        uint32_t octet_b = i < in_len ? data[i++] : 0;
        uint32_t octet_c = i < in_len ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = b64chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = b64chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = i > in_len + 1 ? '=' : b64chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = i > in_len ? '=' : b64chars[triple & 0x3F];
    }
    encoded_data[*out_len] = '\0';
    return encoded_data;
}

unsigned char* base64_decode(const unsigned char* data, size_t in_len, size_t* out_len)
{
	/*
     * @brief   This function decodes a text from BASE64 encoding to  readable format
     *
     * @param data              string data to be decoded
     * @param[size_t] in_len    input data length
     * @param[size_t*] out_len  output data length pointer
     *
     * @return[char*]           The plain text decoded from BASE64 encoding
     *
    */


    if (in_len % 4 != 0) return NULL;

    size_t padding = 0;
    if (in_len > 0 && data[in_len - 1] == '=') padding++;
    if (in_len > 1 && data[in_len - 2] == '=') padding++;

    *out_len = (in_len / 4) * 3 - padding;
    unsigned char *decoded_data = malloc(*out_len + 1);
    if (decoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < in_len;) {
        uint32_t n = 0;
        for (int k = 0; k < 4; k++) {
            n <<= 6;
            int idx = b64_index(data[i++]);
            if (idx >= 0) n |= idx;
        }

        if (j < *out_len) decoded_data[j++] = (n >> 16) & 0xFF;
        if (j < *out_len) decoded_data[j++] = (n >> 8) & 0xFF;
        if (j < *out_len) decoded_data[j++] = n & 0xFF;
    }
    decoded_data[*out_len] = '\0';
    return decoded_data;
}

unsigned long hash_DJB2(const char* str)
{
    /*
     * @brief   Hashes plain text to DJ82 self implementation format
     *
     * @param str               The readable plain text
     *
     * @return[unsigned long]   DJ82 hashed data
    */
	
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void hash_SHA256(const char* str, unsigned char* result)
{
    /*
     * @brief   Hashes plain text using SHA256 algorithm
     *
     * @param[const char*] str          Text to be hashed
     * @param[unsigned char*] result    Result pointer where the function stores
     *                                  the hashed output
    */

    EVP_MD_CTX *context = EVP_MD_CTX_new();
    unsigned int out_len;

    EVP_DigestInit_ex(context, EVP_sha256(), NULL);
    EVP_DigestUpdate(context, str, strlen(str));
    EVP_DigestFinal_ex(context, result, &out_len);

    EVP_MD_CTX_free(context);
}
