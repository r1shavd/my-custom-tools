/*

config_reader.h - header file

*/

#ifndef CONFIG_READER_H
#define CONFIG_READER_H

// Core data structure available to other source files
typedef struct sshhost {
    char hostname[16];
    char usern[64];
    char passw[64];
    int port;

    struct sshhost* NEXT;
} sshhost;

// Public Interface Functions (Function Prototypes)
sshhost* filterHosts(const char *filename);
void displayHosts(const sshhost* head);
void freeHostsList(sshhost* head);

#endif // CONFIG_READER_H