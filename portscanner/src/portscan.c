#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>    // Defines fd_set, FD_ZERO, FD_SET, and select
#include <sys/time.h>    // Defines struct timeval
#include <pthread.h>    // Defines the multithreading pthread creation

#include "portscan.h"

int isIPV4(const char* ip) {
    /*
     This function validates whether an given host ip is an valid IPV4
     ADDRESS or not.
     Returns 0 if SHOW
             1 if NOT VALID
    */

    int octet1, octet2, octet3, octet4;
    char extra;

    // Checking if string matches the pattern
    if (sscanf(ip, "%d.%d.%d.%d%c", &octet1, &octet2, &octet3, &octet4, &extra) != 4) {
        return 0; // Invalid format or extra characters found
    }

    // Validating each octet in 0-255 range
    if ((octet1 < 0 || octet1 > 255) || (octet2 < 0 || octet2 > 255) || (octet3 < 0 || octet3 > 255) || (octet4 < 0 || octet4 > 255)) {
        // If any of the octets fails validation

        return 1;
    }

    return 0; // Valid IPV4 address returns no error
}

void* worker_thread(void* arguments) {
    /*
     This function helps running the multithreaded port scan.
     Avoiding any resources conflicts.
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

        // Executing the scan opertaion on the current port
        scanPorts(scan_data->ip, port, scan_data->show_status);
    }
    return NULL;
}

int scan_port_range(void* arguments) {
    /*
     This function executes the port range based scanning for the mentioned target host. This function serves the purpose of creating multiple threads and focussing on max utilization of CPU cores count to scan the given port range as quickly as possible.
     Current math for max thread utilization is
        if (count for cpu cores > 0)
            -> utilize all the cpu cores i.e., threads = cpu core count x 4
        else
            -> 32 max threads count

     Requires arguments:
        * arguments -> pointer to the Scandetails data;
    */

    Scandetails* scan_data = (Scandetails*) arguments;
    pthread_mutex_init(&scan_data->counter_mutex, NULL);

    // Creating threads and starting the multithreading to further fasten the scan process on the range
    printf ("[~] Scanning %s for the range %d to %d\n", scan_data->ip, scan_data->current_port, scan_data->last_port);
    long num_cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    int num_threads = (num_cpu_cores > 0) ? (int) num_cpu_cores * 4 : 32;
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("Failed to allocate mem space for threads");
        pthread_mutex_destroy(&scan_data->counter_mutex);
        return 1;
    }

    // Launching the worker thread and passing the runtime Scandetails pointer
    for (int i = 0; i < num_threads; i++)    pthread_create(&threads[i], NULL, worker_thread, scan_data);

    // Waiting for all workers threads to get executed 
    for (int i = 0; i < num_threads; i++)    pthread_join(threads[i], NULL);

    free(threads);
    pthread_mutex_destroy(&scan_data->counter_mutex);

    printf("[*] Scan completed\n");
    return 0;
}


void scanPorts(const char* ip, int port, unsigned char show_status) {
    /*
     This function connects to the given HOST IP ADDRESS via the given PORT.
     Checks if the PORT can be connected, then gives out the result
        * OPEN     - port is accessible
        * CLOSED   - port is not accessible, we usually do not display closed ports as of
                     intial codebase. But, this may be changed in later versions.
        * FILTERED - port is either getting blocked (most probably via a firewall)
     
     Requires argument:
      * ip -> string (const char*)
      * port -> int (range should be 1 to 65535
      * show_status -> int ; value should be either 0 false or 1 true.
      
      if show_status == SHOW, then we display all status regarding CLOSED or FILTERED.
                           Useful in case of single port scan or hard scan
      if show_status == HIDE, then we just display the OPEN status if available
                            Useful for multiple ports, range scan.
    */

    // Creating a socket connection config
    int socket_connection;
    struct sockaddr_in target;
    fd_set write_fdset, err_fdset;
    struct timeval tv;

    socket_connection = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_connection < 0) {
        return;
    }

    int flags = fcntl(socket_connection, F_GETFL, 0);
    fcntl(socket_connection, F_SETFL, flags | O_NONBLOCK);

    // Assigning the given host address and port to the socket connection
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);

    // Starting the connection
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

        // Monitor both writeability and error exceptions
        int select_res = select(socket_connection + 1, NULL, &write_fdset, NULL, &tv);

        if (select_res > 0) {
            if (FD_ISSET(socket_connection, &write_fdset)) {
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(socket_connection, SOL_SOCKET, SO_ERROR, &so_error, &len);

                if (so_error == 0) {
                    // If there is an immediate connection, we mark open

                    printf("Port %-5d: OPEN\n", port);
                } else if (so_error == ECONNREFUSED) {
                    // If hoost sends an RST packet (Port active but closed)

                    if (show_status == 1)    printf("Port %-5d: CLOSED\n", port);
                } else {
                    // If the host or router returned an ICMP error/other failure 

                    if (show_status == 1)    printf("Port %-5d: FILTERED\n", port);
                }
            }
        } else if (select_res == 0) {
            // If timeout reached with zero response (firewall silently dropped packet)

            if (show_status == 1)    printf("Port %-5d: FILTERED\n", port);
        } else {
            // Select error occurred (either connection, or i suck at coding)

            printf("Port %-5d: ERROR\n", port);
        }
    } else {
        // If we get immediate failure (i.e., Network Unreachable)
        
        printf("Port %-5d: FILTERED or UNREACHABLE\n", port);
    }

    close(socket_connection);
}
