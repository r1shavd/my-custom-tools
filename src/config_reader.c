/*
Config Reader

System: All
Platform: All

This src reads config files (generally in INI formats and displays them
in a tabular format. This is a common script made to be implemented in 
the various programs which i make.

Current format for configuration:
```
[CurrentSection]
hostname=<value>
usern=<value>
passw=<value>
port=<value>
[NewSection]
hostname=<value>
...
...
port=<value>
```

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Custom data structure to store ssh host details
typedef struct sshhost {
	char hostname[16];
	char usern[64];
	char passw[64];
	int port;

	struct sshhost* NEXT;
} sshhost;

void trimWhitespaceConfigRead(char *str) {
	char *end = str + strlen(str) - 1;
	while (end >= str && (*end == ' ' || *end == '\n' || *end == '\r')) {
			*end = '\0';
			end--;
	}
}

void displayHosts(const sshhost* head) {
	/*
	This function displays the list of hosts which is passed as
	a dyanmically assigned linked list as argument to this function
	*/

    if (head == NULL) {
        printf("[\033[0;91m!\033[0m] No hosts found in the configuration list.\n");
        return;
    }

    // Initialize temporary tracking pointer to head node
    const sshhost* current = head;
    int index = 1;

    // Loop until current points to NULL
    printf("\t\033[1;96mLOADED SSH HOSTS\033[0m\n\n");
    while (current != NULL) {
        printf("\033[0;93m[%02d]\033[0m Destination: \033[0;92m%s:\033[0;95m%d\033[0m\n", index, current->hostname, current->port);
        printf("     Credential:  %s : %s\n", current->usern, current->passw);

        // Move the pointer to the next block in the chain
        current = current->NEXT;
        index++;
    }
    printf("\t\033[0;96m------- * -------\033[0m\n");
}

void freeHostsList(sshhost* head) {
    sshhost* current = head;
    while (current != NULL) {
        sshhost* next_node = current->NEXT;
        free(current);
        current = next_node;
    }
}

sshhost* filterHosts(const char* filelocation) {
	/*
	This function filters out the host details and put them inside a
	dynamically allocated linked list and returns the results
	This function requires a config file as the argument.
	*/

	FILE *file = fopen(filelocation, "r");
	if (!file) {
		perror("[\033[0;91m!\033[0m] Failed to open file");
		return NULL; 
	}

	// Linked list pointers
	sshhost* head = NULL;
	sshhost* current_node = NULL;

	char newEntry[64] = {0};
	char line[64];

	while ( fgets(line, sizeof(line), file) ) {
		trimWhitespaceConfigRead(line);

		if (line[0] == '\0' || line[0] == ';' || line[0] == '#')	continue;

		if (line[0] == '[' && line[strlen(line)-1] == ']') {
			sscanf(line, "[%63[^]]]", newEntry);
		    continue;
		}
	
		char key[64], value[64];
		if ( sscanf(line, "%63[^=]= %63[^\n]", key, value) == 2 ) {
			trimWhitespaceConfigRead(key);
			trimWhitespaceConfigRead(value); // Clean the extracted value too

			if ( strcmp(newEntry, "host") == 0 ) {
				// If we hit a hostname entry, we start memory allocation
				// + filtering and storing the credentials
				
				if (strcmp(key, "hostname") == 0 || current_node == NULL) {
					sshhost* new_node = (sshhost*) calloc(1, sizeof(sshhost));
					if (!new_node) {
						perror("Memory allocation failed");
						fclose(file);
						return head;
					}
					
					if (head == NULL)	head = new_node;
					else	current_node->NEXT = new_node;
					current_node = new_node;
				}

				// Assigning values to our persistent active node
				if ( strcmp(key, "hostname") == 0 )    strncpy(current_node->hostname, value, sizeof(current_node->hostname) - 1);
				if ( strcmp(key, "usern") == 0 )       strncpy(current_node->usern, value, sizeof(current_node->usern) - 1);
				if ( strcmp(key, "passw") == 0 )       strncpy(current_node->passw, value, sizeof(current_node->passw) - 1);
				if ( strcmp(key, "port") == 0 )        current_node->port = atoi(value);
			}
		}
	}
	fclose(file);
	return head;
}