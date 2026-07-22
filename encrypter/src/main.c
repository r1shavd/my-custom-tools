#include "colors.h"
#include "encrypt.h"
#include "decrypt.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

static enum TaskType
{
    // To check if user wants encryption / decryption

    NO_TASK = 0,
    ENCRYPT_TASK,
    DECRYPT_TASK
};

void display_help()
{
    printf("Usage: ./encrypter --file <path> [--encrypt | --decrypt] [--help] --password <password>\n\n");
    printf("Options:\n");
    printf("  --file <path>         Specify the file to process (Required)\n");
    printf("  --password <passw>    Specify the passw for encryption (Required)\n");
    printf("  --encrypt             Perform encryption task\n");
    printf("  --decrypt             Perform decryption task\n");
    printf("  --help                Display this help menu\n");
}

int main(int argc, char** argv)
{
    int opt;
    char* filename = NULL;
    char password[64];
    memset(password, '\0', sizeof(password));
    enum TaskType task = NO_TASK;

    struct option long_options[] = {
        {"file",    required_argument, 0, 'f'},
        {"password", required_argument, 0, 'p'},
        {"encrypt", no_argument,       0, 'e'},
        {"decrypt", no_argument,       0, 'd'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'p':
                snprintf(password, sizeof(password), "%s", optarg);
                break;
            case 'e':
                task = ENCRYPT_TASK;
                break;
            case 'd':
                task = DECRYPT_TASK;
                break;
            case 'h':
                display_help();
                return EXIT_SUCCESS;
            default:
                display_help();
                return EXIT_FAILURE;
        }
    }

    // Validation
    if (filename == NULL) {
        fprintf(stderr, "[!] The --file argument is required\n");
        return EXIT_FAILURE;
    }

    if (task == NO_TASK) {
        fprintf(stderr, "[!] You must specify either --encrypt or --decrypt\n");
        return EXIT_FAILURE;
    }

    if (password[0] == '\0') {
        fprintf(stderr, "[!] Please specify a password using flag --password\n");
        return EXIT_FAILURE;
    }

    if ((sizeof(password) / sizeof(password[0])) < 2) {
        fprintf(stderr, "[!] Enter a valid password\n");
        return EXIT_FAILURE;
    }

    // Encrypting or decrypting based on task
    if (task == ENCRYPT_TASK) {
    	encrypt_file(filename, password);
    } else {
        decrypt_file(filename, password);
    }

    return 0;
}
