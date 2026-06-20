/*
ENCRYPTER

System: Windows + Linux
Platform: tty based

This tool encrypts files and folders and both the process require a
password. The algorithm is simple for now, this tool is just made
for me to hide my files, notes, and source code from watchers on
my work machine.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// Custom header files
#include "colors.h"

// --- Helper functions for string manipulations ---

void get_hidden_input(const char *prompt, char *output, size_t max_len) {
    printf("%s", prompt);
    fflush(stdout);
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    fgets(output, max_len, stdin);
    SetConsoleMode(hStdin, mode);
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fgets(output, max_len, stdin);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    output[strcspn(output, "\r\n")] = 0; // Strip trailing newlines
    printf("\n");
}

void simple_sha256_mock(const char *input, char *output) {
    // Pure C lacks a built-in cryptographic library.
    // If you compile with OpenSSL, replace this block with the real SHA256() function.
    // This mock generates a stable 64-character hex hash representation for logic flow matching.
    unsigned long hash = 5381;
    int c;
    while ((c = *input++)) hash = ((hash << 5) + hash) + c;
    sprintf(output, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b%03lx", hash % 1000);
}

void rot13(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') str[i] = (str[i] - 'a' + 13) % 26 + 'a';
        else if (str[i] >= 'A' && str[i] <= 'Z') str[i] = (str[i] - 'A' + 13) % 26 + 'A';
    }
}

char *to_hex(const char *str) {
    size_t len = strlen(str);
    char *hex_str = malloc(len * 2 + 1);
    for (size_t i = 0; i < len; i++) sprintf(hex_str + (i * 2), "%02x", (unsigned char)str[i]);
    hex_str[len * 2] = '\0';
    return hex_str;
}

char *from_hex(const char *hex) {
    size_t len = strlen(hex);
    char *str = malloc(len / 2 + 1);
    for (size_t i = 0; i < len / 2; i++) {
        unsigned int val;
        sscanf(hex + (i * 2), "%02x", &val);
        str[i] = (char)val;
    }
    str[len / 2] = '\0';
    return str;
}

// Custom base64 table lookup 
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(const char *str) {
    size_t input_len = strlen(str);
    size_t output_len = 4 * ((input_len + 2) / 3);
    char *encoded = malloc(output_len + 1);
    
    size_t i, j;
    for (i = 0, j = 0; i < input_len;) {
        uint32_t octet_a = i < input_len ? (unsigned char)str[i++] : 0;
        uint32_t octet_b = i < input_len ? (unsigned char)str[i++] : 0;
        uint32_t octet_c = i < input_len ? (unsigned char)str[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded[j++] = b64_table[(triple >> 18) & 0x3F];
        encoded[j++] = b64_table[(triple >> 12) & 0x3F];
        encoded[j++] = (i > input_len + 1) ? '=' : b64_table[(triple >> 6) & 0x3F];
        encoded[j++] = (i > input_len) ? '=' : b64_table[triple & 0x3F];
    }
    encoded[output_len] = '\0';
    return encoded;
}

char *base64_decode(const char *str) {
    size_t input_len = strlen(str);
    size_t output_len = input_len / 4 * 3;
    if (str[input_len - 1] == '=') output_len--;
    if (str[input_len - 2] == '=') output_len--;
    
    char *decoded = malloc(output_len + 1);
    int table[256];
    for (int i = 0; i < 64; i++) table[(int)b64_table[i]] = i;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_len;) {
        uint32_t sextet_a = str[i] == '=' ? 0 : table[(int)str[i]]; i++;
        uint32_t sextet_b = str[i] == '=' ? 0 : table[(int)str[i]]; i++;
        uint32_t sextet_c = str[i] == '=' ? 0 : table[(int)str[i]]; i++;
        uint32_t sextet_d = str[i] == '=' ? 0 : table[(int)str[i]]; i++;
        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        
        if (j < output_len) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < output_len) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < output_len) decoded[j++] = triple & 0xFF;
    }
    decoded[output_len] = '\0';
    return decoded;
}

// Declaring the main function - mutual/clean function routing
bool encrypt(const char *file, const char *passw_param);
bool decrypt(const char *file, const char *passw_param);

void traverse_directory(const char *dir_path, int encryption_mode, const char *passw_param) {
    /*
    This function traverses throughout the directory tree.
    Then, it does encryption / decryption as per the mode
    mentioned in the arguments.
    */

    DIR *dir = opendir(dir_path);
    if (!dir) {
        printf("[\033[0;91mERROR\033[0m] Cannot open folder: %s%s%s\n", WHITE, dir_path, DEFAULT);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip current directory "." and parent directory ".." to avoid infinite loops
        
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build full path dynamically
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat stat_buf;
        if (stat(full_path, &stat_buf) == 0) {
            if (S_ISDIR(stat_buf.st_mode)) {
                // If it is a directory, descend recursively without breaking structural layout
                
                traverse_directory(full_path, encryption_mode, passw_param);
            } else if (S_ISREG(stat_buf.st_mode)) {
                // If it is a regular file, apply corresponding operation
                
                if (encryption_mode) {
                    encrypt(full_path, passw_param);
                } else {
                    decrypt(full_path, passw_param);
                }
            }
        }
    }
    closedir(dir);
}

// --- Encryption and Decryption Logical Function Blocks ---
bool encrypt(const char *file, const char *passw_param) {
     /*
    This function processes text by applying ROT13, converting to a Hex string, 
    and then encoding into Base64.
    Reads data from a file and saves to the same file.
    */

    FILE *f = fopen(file, "r");
    if (!f) {
        if (errno == ENOENT) {
            printf("[\033[0;91mERROR\033[0m] File not found: \033[0;97m%s\033[0m\n", file);
        } else if (errno == EACCES) {
            printf("[\033[0mPERMISSION DENIED\033[0m] Check your user read privileges\n");
        } else if (errno == EPERM) {
            printf("[\033[0mNOT PERMITTED\033[0m] Higher system restrictions apply\n");
        } else {
            printf("[\033[0mERROR\033[0m] %d - Operating system fault\n", errno);
        }
        return 0;
    }

    // Reading the file and encrypting
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *text = malloc(size + 1);
    fread(text, 1, size, f);
    text[size] = '\0';
    fclose(f);

    // Computing simple hex string representation using parameter passed from main
    char passw[128] = {0};
    simple_sha256_mock(passw_param, passw);

    // Checking if file text array already begins with the 64-char hash layout
    char first_line[128] = {0};
    strncpy(first_line, text, 64);
    if (strcmp(passw, first_line) == 0) {
        printf("[\033[0;91mERROR\033[0m] File already encrypted: %s\n", file);
        printf("[%s!%s] %sSkipping...%s\n", YELLOW, DEFAULT, WHITE, DEFAULT);
        free(text);
        return 0;
    }

    rot13(text);
    char *hex_text = to_hex(text);
    char *b64_text = base64_encode(hex_text);
    
    char *text_final = malloc(strlen(passw) + strlen(b64_text) + 2);
    sprintf(text_final, "%s\n%s", passw, b64_text);

    // Saving the encrypted data back to the file
    f = fopen(file, "w");
    if (!f) {
        free(text); free(hex_text); free(b64_text); free(text_final);
        return 0;
    }
    fprintf(f, "%s", text_final);
    fclose(f);

    printf("[\033[0;92mSUCCESS\033[0m] Encrypted file: %s\n", file);
    free(text); free(hex_text); free(b64_text); free(text_final);
    return 1;
}

bool decrypt(const char *file, const char *passw_param) {
    /*
    This function processes text by decoding Base64, converting from a Hex string, 
    and then reversing the ROT13 shift.
    Reads data from a file and saves to the same file.
    */
    
    FILE *f = fopen(file, "r");
    if (!f) {
        if (errno == ENOENT) printf("[\033[0;91mERROR\033[0m] File not found: \033[0;97m%s\033[0m\n", file);
        return 0;
    }

    // Reading the file and decrypting
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *text = malloc(size + 1);
    fread(text, 1, size, f);
    text[size] = '\0';
    fclose(f);

    // Splitting the file to get the stored hash and the encrypted data
    char *newline_ptr = strchr(text, '\n');
    if (!newline_ptr) {
        printf("[\033[0;91mERROR\033[0m] File is not encrypted or is corrupted: %s\n", file);
        free(text);
        return 0;
    }

    size_t hash_len = newline_ptr - text;
    char *stored_hash = malloc(hash_len + 1);
    strncpy(stored_hash, text, hash_len);
    stored_hash[hash_len] = '\0';
    char *encrypted_data = newline_ptr + 1;

    // Compute simple hex string representation using parameter passed from main
    char passw[128] = {0};
    simple_sha256_mock(passw_param, passw);

    if (strcmp(passw, stored_hash) != 0) {
        printf("[\033[0;91mERROR\033[0m] Incorrect password for file: %s%s%s\n", YELLOW, file, DEFAULT);
        free(text); free(stored_hash);
        exit(1); // Error exit mid execution on incorrect password match
    }

    // Reversing the encryption pipeline
    char *b64_decoded = base64_decode(encrypted_data);
    char *hex_decoded = from_hex(b64_decoded);
    rot13(hex_decoded);
    char *text_final = hex_decoded;

    // Saving the decrypted data back to the file
    f = fopen(file, "w");
    if (!f) {
        free(text); free(stored_hash); free(b64_decoded); free(hex_decoded);
        return 0;
    }
    fprintf(f, "%s", text_final);
    fclose(f);

    printf("[\033[0;92mSUCCESS\033[0m] Decrypted file: %s\n", file);
    free(text); free(stored_hash); free(b64_decoded); free(hex_decoded);
    return 1;
}

int main(int argc, char *argv[]) {
    bool encryption_mode = false;
    bool decryption_mode = false;
    char *target_file = NULL;
    char *target_dir = NULL;

    // Loop through arguments to parse options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
            printf("%sUsage:%s wsbcrypt [%soptions%s]%s\n\n", BLUE, WHITE, DEFAULT, WHITE, DEFAULT);
            printf("%sOptions:%s\n", BLUE, WHITE);
            printf("\t%s--encrypt%s%-20sSet mode to encrypt files\n", CYAN, DEFAULT, "");
            printf("\t%s--decrypt%s%-20sSet mode to decrypt files\n", CYAN, DEFAULT, "");
            printf("\t%s--file=%s\"path\"%-20sTarget a single specific file\n", CYAN, WHITE, DEFAULT);
            printf("\t%s--folder=%s\"path\"%-18sTarget and recursively process a directory tree\n", CYAN, WHITE, DEFAULT);
            printf("\t%s-h, --help%s%-19sShow this help menu configuration\n", CYAN, DEFAULT, "");
            return 0;
        } else if (strcmp(argv[i], "--encrypt") == 0) {
            encryption_mode = true;
        } else if (strcmp(argv[i], "--decrypt") == 0) {
            decryption_mode = true;
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            target_file = argv[i] + 7; // Extract path after the '=' sign
        } else if (strncmp(argv[i], "--folder=", 9) == 0) {
            target_dir = argv[i] + 9; // Extract path after the '=' sign
        }
    }

    // Validate that a specific operation mode was selected
    if (!encryption_mode && !decryption_mode) {
        printf("[\033[0;91mERROR\033[0m] Must specify either --encrypt or --decrypt\n");
        return 1;
    }
    if (encryption_mode && decryption_mode) {
        printf("[\033[0;91mERROR\033[0m] Cannot specify both --encrypt and --decrypt together\n");
        return 1;
    }

    // Validate that a specific target was provided
    if (!target_file && !target_dir) {
        printf("[\033[0;91mERROR\033[0m] Must provide a target using --file=\"...\" or --folder=\"...\"\n");
        return 1;
    }

    // Ask for password exactly once globally before starting loop sequences
    char passw_buf[256] = {0};
    if (encryption_mode) {
        get_hidden_input("Enter a password for encryption: ", passw_buf, sizeof(passw_buf));
    } else {
        get_hidden_input("Enter the password for decryption: ", passw_buf, sizeof(passw_buf));
    }
    if (strlen(passw_buf) == 0) {
        // Error exit in case password is left empty completely
        
        printf("[\033[0;91mERROR\033[0m] Password cannot be empty\n");
        exit(1);
    }

    // Executing targetted actions safely based on matches
    if (target_file) {
        if (encryption_mode) {
            encrypt(target_file, passw_buf);
        } else {
            decrypt(target_file, passw_buf);
        }
    }

    if (target_dir) {
        traverse_directory(target_dir, encryption_mode, passw_buf);
    }
    return 0;
}