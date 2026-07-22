#ifndef PORTSCAN_H

#define PORTSCAN_H

#include <stdint.h>
#include <pthread.h>

#define DETAILS_SHOW 1
#define DETAILS_HIDE 0

// Definitions for avoiding re-scanning any scanned priority ports (for port-ranges)
#define SET_PORT(PORT)    (scanned_ports[(PORT) / 8] |= (1 << ((PORT) % 8)))
#define CHECK_PORT(PORT)    (scanned_ports[(PORT) / 8] & (1 << ((PORT) % 8)))

typedef struct {
    /*
     * @brief   stores details for the scan operation
     *          useful in case of multithreaded scans.
    */

    const char* ip;
    int current_port;
    int last_port;
    int show_status;
    pthread_mutex_t counter_mutex;
} Scandetails;

int isIPV4(const char* ip);

void* worker_thread(void* arguments);
int scan_port_range(void* arguments);

void scanPorts(const char* ip, int port, unsigned char show_status);

#endif    /*  PORTSCAN_H */
