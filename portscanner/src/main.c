#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portscan.h"

int main(int argc, const char** argv) {
    // The main driver function

    /*
    NOTE:
      This version currently has configurations:
       * single port scan and multi port scan runs on a single thread, thus maximizing power
         for the single scan. Also, show_status = SHOW as we need complete details whether 
         the port is OPEN / CLOSED / FILTERED

       * port range scans are executed in multiple threads to squeeze out the max speed required.
         show_status = HIDE default as to avoid clutter of results and filter out only OPEN connections.
    */

    // Parsing the arguments
    // - - - * - - -
    if (argc < 4) {
        // If the arguments aren't enough ./portscan <ip> <command> <ip1> <ip2> ....
        // We display usage text / help
        
       printf("USAGE:\n\nportscan <ip> <command> <ip1> <ip2> .... <ipN>\n\nLike,\nportscan 192.168.1.2 --port-range 80 2000\n");
       return 0;
    }

    // Getting and validating the user input IPV4 host addr
    const char* target = argv[1];
    if (isIPV4(target) == 1) {
        // If the user given host address is not valid

        printf("[!] Invalid IP address provided\n");
        return EXIT_FAILURE;
    }

    // Validating the command
    const char* task = argv[2];
    if (!task) {
        // If the user didn't mention the command / task

        printf("[!] Task not mentioned availabe: --port-range, --ports, --port\n");
        return EXIT_FAILURE;
    }
    // - - - * - - -

    int port;
    if (strcmp(task, "--port") == 0) {
        // If the user requested to scan 1 particular port

        if (argv[3] == NULL) {
            // If the third argument i.e., the port to be scanned is not provided, then raise an error

            printf("[!] Please mention a port.\nUse: portscan --port <port>\n");
            return EXIT_FAILURE;
        }
        port = atoi(argv[3]);
        printf("[~] Scanning for %s at port %d\n", target, port);
        scanPorts(target, port, SHOW);
        return 0;
    } else if (strcmp(task, "--ports") == 0) {
        // If the user requested to scan multiple ports

        printf("[~] Scanning for %s at given ports\n", target);
        for (int i = 3; i < argc; i++) {
            port = atoi(argv[i]);
            if (!port) {
                // If invalid port format is given

                printf("[!] Invalid port: %s\n", argv[i]);
                continue;
            }
            
            scanPorts(target, port, SHOW);
        }
        return 0;
    } else if (strcmp(task, "--port-range") == 0) {
        // If the user requested for a port range
        
        port = strtol(argv[3], NULL, 10);
        int port_ = strtol(argv[argc-1], NULL, 10);
        if (!port || !port_) {
            // If the port ranges are invalid

            printf("[!] Invalid port range given %s - %s\n", argv[3], argv[argc-1]);
            return EXIT_FAILURE;
        }

        // Starting the scan as in multithreading mode
        Scandetails scan_data;
        scan_data.ip = target;
        scan_data.current_port = port;
        scan_data.last_port = port_;
        scan_data.show_status = HIDE;
        scan_port_range(&scan_data);
        return 0;
    }

    printf("Scan complete.\n");
    return 0;
}
