#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "encrypt.h"
#include "decrypt.h"

int main(int argc, char *argv[])
{
    char filepath[256];
    char password[256];

    if (argc > 1) {
        strncpy(filepath, argv[1], sizeof(filepath) - 1);
        filepath[sizeof(filepath) - 1] = '\0';
    } else {
        printf("Enter the file path: ");
        if (!fgets(filepath, sizeof(filepath), stdin)) return 1;
        filepath[strcspn(filepath, "\n")] = 0;
    }

	if (argc > 2) {
		if (strcmp(argv[2], "--encrypt") == 0 || strcmp(argv[2], "-e") == 0) {
			// If the user choosed ENCRYPTION

			printf("Enter password for encryption: ");
   			if (!fgets(password, sizeof(password), stdin)) return 1;
    		password[strcspn(password, "\n")] = 0;
			if (strlen(password) < 2) {
				printf("[%s!%s] Please enter a valid password\n", RED, DEFAULT);
				return EXIT_FAILURE;
			}
    		encrypt_file(filepath, password);

    		// Cleaning up mem safely
    		memset(password, 0, sizeof(password));

		} else if (strcmp(argv[2], "--decrypt") == 0 || strcmp(argv[2], "-d") == 0) {
			// If the user choosed DECRYPTION

			printf("Enter password for decryption: ");
			if (!fgets(password, sizeof(password), stdin)) return 1;
    		password[strcspn(password, "\n")] = 0;
			if (strlen(password) < 2) {
				printf("[%s!%s] Please enter a valid password\n", RED, DEFAULT);
				return EXIT_FAILURE;
			}
    		decrypt_file(filepath, password);

   			 // Cleaning up mem safely
    		memset(password, 0, sizeof(password));		
		} else {
			// Error argument

			printf("[%s!%s] No such argument '%s'. Use --encrypt --decrypt or --help\n", RED, DEFAULT, argv[2]);
			return EXIT_FAILURE;
		}
	}    
    return 0;
}
