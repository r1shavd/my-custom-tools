#include "portscan.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>    // Defines fd_set, FD_ZERO, FD_SET, and select
#include <sys/time.h>    // Defines struct timeval

static int priority_ports[] = {
    20, 21, 22, 23, 25, 53, 80, 81,
    110, 123, 143, 443, 445, 587,
    993, 995, 1433, 2222, 3000, 3001,
    3306, 3389, 3390, 5000, 5432, 8000,
    8080, 8443, 8888, 9000, 27017, 33389
};
uint8_t scanned_ports[32768];

int isIPV4(const char* ip)
{
    /*
     * @brief   Checks whether host address is proper ipv4 or not
     *
     * @param[const char*] ip       target ip address
     *
     * @return                      0 for invalid addr
     *                              1 for valid addr
    */

    int octet1, octet2, octet3, octet4;
    char extra;

    // sscanf should return EXACTLY 4. 
    // If it returns 5, it means there are extra trailing characters.
    if (sscanf(ip, "%d.%d.%d.%d%c", &octet1, &octet2, &octet3, &octet4, &extra) != 4) {
        return 0; // Invalid format
    }

    if ((octet1 < 0 || octet1 > 255) || 
        (octet2 < 0 || octet2 > 255) || 
        (octet3 < 0 || octet3 > 255) || 
        (octet4 < 0 || octet4 > 255)) {
        return 0; // Invalid range
    }

    return 1; // Valid IP
}

void* worker_thread(void* arguments)
{
    /*
     * @brief   Mutlithread worker - resolves resource conflicts
     *
     * @params[void] Scannerdetails
     *
    */

    Scandetails* scan_data = (Scandetails*) arguments;
    while (1) {
        int port = -1;

        // Retrieval of the next port num, keeping the thread run safe
        pthread_mutex_lock(&scan_data->counter_mutex);
        if (scan_data->current_port <= scan_data->last_port) {
            port = scan_data->current_port;
            scan_data->current_port++;
        }
        pthread_mutex_unlock(&scan_data->counter_mutex);

        // Breaking loop when the port range is done scanning
        if (port == -1)    break;

        // If current port was already visited
        if (CHECK_PORT(port))    continue;

        // Executing the scan opertaion on the current port
        scanPorts(scan_data->ip, port, scan_data->show_status);
    }
    return NULL;
}

int scan_port_range(void* arguments)
{
    /*
     * @brief   Executes port range based scanning (implemented with
     *          multithreading and other wrappers.
     *
     * @description     This function uses multithreading to utilize the CPU cores and
     *                  threads for parallel scanning and pushing forward quick scan
     *                  results in case of longer ranges.
     *
     * @param[void*] Scannerdetails
     *
     * @return  0 for sucessfull execution
     *         -1 for error case
    */

    Scandetails* scan_data = (Scandetails*) arguments;
    pthread_mutex_init(&scan_data->counter_mutex, NULL);

    printf ("[~] Scanning %s for the range %d to %d\n", scan_data->ip, scan_data->current_port, scan_data->last_port);
    
    // First doing the priority_ports
    memset(scanned_ports, 0, sizeof(scanned_ports));
    for (size_t i = 0; i < (sizeof(priority_ports) /  sizeof(priority_ports[0])); i++) {
        if (priority_ports[i] < scan_data->current_port || priority_ports[i] > scan_data->last_port)    continue;    // Skipping the PORT if not in the range specified by user

        scanPorts(scan_data->ip, priority_ports[i], scan_data->show_status);
        SET_PORT(priority_ports[i]);
    }

    // Creating threads and starting the multithreading to further fasten the scan process on the range
    long num_cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    int num_threads = (num_cpu_cores > 0) ? (int) num_cpu_cores * 4 : 32;
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("Failed to allocate mem space for threads");
        pthread_mutex_destroy(&scan_data->counter_mutex);
        return 1;
    }

    for (int i = 0; i < num_threads; i++)    pthread_create(&threads[i], NULL, worker_thread, scan_data);
    for (int i = 0; i < num_threads; i++)    pthread_join(threads[i], NULL);

    free(threads);
    pthread_mutex_destroy(&scan_data->counter_mutex);

    printf("[*] Scan completed\n");
    return 0;
}

void scanPorts(const char* ip, int port, unsigned char show_status)
{
    /**
    * @brief		Scans a specific network port on a target host to determine its availability.
    *
    * @description	This function attempts to establish a connection to a specified host
    *				IP address through a designated port. It evaluates the connection response to
    *				classify the port status into one of three categories:
    *				- OPEN: The port is actively listening and accessible.
    *				- CLOSED: The port is not accessible. These are typically hidden by default
    *				  but may be displayed based on configuration.
    *				- FILTERED: The port is unresponsive, indicating potential blocking by a firewall.
    *
    * @param[in]	ip			The target host IP address as a null-terminated string.
    * @param[in]	port		The target port number to scan (valid range: 1 to 65535).
    * @param[in]	show_status	Output verbosity flag. Set to 1 (DETAILS_SHOW) to log all statuses
    *							(including CLOSED and FILTERED) for detailed or single-port scans.
     *							Set to 0 (DETAILS_HIDE) to suppress non-open ports during large range scans.
    */

    int socket_connection;
    struct sockaddr_in target;
    fd_set write_fdset, err_fdset;
    struct timeval tv;

    socket_connection = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_connection < 0)
        return;

    // Setting sockets to NON-BLOCKING mode (helps to speed up shit)
    int flags = fcntl(socket_connection, F_GETFL, 0);
    fcntl(socket_connection, F_SETFL, flags | O_NONBLOCK);

    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);

    int res = connect(socket_connection, (struct sockaddr*)&target, sizeof(target));
    
    if (res == 0) {
        // Instant connection (rare in non-blocking unless localhost)
        printf("Port %d: OPEN\n", port);
        close(socket_connection);
        return;
    }

    if (res < 0 && errno == EINPROGRESS) {
        FD_ZERO(&write_fdset);
        FD_ZERO(&err_fdset);
        FD_SET(socket_connection, &write_fdset);
        FD_SET(socket_connection, &err_fdset);

        tv.tv_sec = 1;    // 1-second timeout
        tv.tv_usec = 0;

        int select_res = select(socket_connection + 1, NULL, &write_fdset, NULL, &tv);

        if (select_res > 0) {
            if (FD_ISSET(socket_connection, &write_fdset)) {
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(socket_connection, SOL_SOCKET, SO_ERROR, &so_error, &len);

                if (so_error == 0) {
                    // Immediate connection found
                    printf("Port %-5d: OPEN\n", port);
                }
                else if (so_error == ECONNREFUSED) {
                    if (show_status == 1)
                        printf("Port %-5d: CLOSED\n", port);
                }
                else {
                    if (show_status == 1)
                        printf("Port %-5d: FILTERED\n", port);
                }
            }
        } else if (select_res == 0) {
            if (show_status == 1)
                printf("Port %-5d: FILTERED\n", port);
        } else {
            printf("Port %-5d: ERROR\n", port);
        }
    } else {
        // Immediate failure
        printf("Port %-5d: FILTERED or UNREACHABLE\n", port);
    }

    close(socket_connection);
}
