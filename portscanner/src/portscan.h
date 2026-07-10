#ifndef PORTSCAN_H
#define PORTSCAN_H

#define SHOW 1
#define HIDE 0

// Custom data structures
// - - - * - - -
typedef struct {
    /*
     This stores the scan details for the multithreaded scan
     in case of port range scan. where we need to constantly
     access the memory location of the target details.
     This process becomes easier with defining a custom
     structure.
    */

    const char* ip;
    int current_port;
    int last_port;
    int show_status;
    pthread_mutex_t counter_mutex;
} Scandetails;
// - - - * - - -

// Helper functions for small tasks
int isIPV4(const char* ip);

// Helper functions for heavy range scans (via multithreading)
void* worker_thread(void* arguments);
int scan_port_range(void* arguments);

// Core functions
void scanPorts(const char* ip, int port, unsigned char show_status);

#endif    // PORTSCAN_H
