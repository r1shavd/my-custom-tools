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

    int port = 0;
    if (strcmp(task, "--port") == 0) {
        // If the user requested to scan 1 particular port

        if (argv[3] == NULL) {
            // If the third argument i.e., the port to be scanned is not provided, then raise an error

            printf("[!] Please mention a port.\nUse: portscan --port <port>\n");
            return EXIT_FAILURE;
        }
        port = strtol(argv[3], NULL, 10);
        printf("[~] Scanning for %s at port %d\n", target, port);
        scanPorts(target, port, SHOW);
        return 0;
    } else if (strcmp(task, "--ports") == 0) {
        // If the user requested to scan multiple ports

		// Checking if user mentioned priority ports
		// i.e., ./portscanner 127.0.0.1 --ports --priority
		if (strcmp(argv[3], "--priority") == 0) {
			int priority_ports[] = { 20, 21, 22, 23, 25, 53, 80, 81, 110, 123, 143, 443, 445, 587, 993, 995, 1433, 2222, 3000, 3001, 3306, 3389, 3390, 5000, 5432, 8000, 8080, 8443, 8888, 9000, 27017, 33389 };
			printf("[~] Scanning %s for priority ports\n", target);
			for (size_t i = 0; i < (sizeof(priority_ports) / sizeof(priority_ports[0])); i++) {
				scanPorts(target, priority_ports[i], SHOW);
			}
			return 0;
		 }

		// Continuing with the user given arguments
        printf("[~] Scanning for %s at given ports\n", target);
        for (int i = 3; i < argc; i++) {
            port = strtol(argv[i], NULL, 0);
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
        
        int port_ = 0;
        if (strcmp(argv[3], "all") == 0) {
            // If the user specified all the existing ports
            // we set 1 to 65535

            port = 1;
            port_ = 65535;
        } else {
            // If the port range is a speicified numbers. For example: --port-range 20 1000
        
            port = strtol(argv[3], NULL, 10);
            port_ = strtol(argv[argc-1], NULL, 10);
            if (!port) {
                // If the port ranges are invalid (for lower counter of the range)

                printf("[!] Invalid port given %s\n", argv[3]);
                return EXIT_FAILURE;
            }
			if (!port_) {
				// If the last argument fails to be the port range end
				// Then, we try for nearest argument to port range head

				port_ = strtol(argv[4], NULL, 10);
				if (!port_) {
					printf("[!] Invalid port given %s\n", argv[4]);
					return EXIT_FAILURE;
				}
			}

            // Swapping the ports in case range ends are larger to small
            // i.e., [ a, b ] -> [ 100, 20 ]
            if (port > port_) {
                port = port ^ port_;
                port_ = port ^ port_;
                port = port ^ port_;
            }
        }
       
        // Starting the scan as in multithreading mode
        Scandetails scan_data;
        scan_data.ip = target;
        scan_data.current_port = port;
        scan_data.last_port = port_;

		// Checking if --detailed argument mentioned
		// ./portscanner 127.0.0.1 --port-range 10 200 --detailed
		// or,
		// ./portscanner 127.0.0.1 --port-range --detailed
		// - - - * - - -
        if ((argv[4] != NULL && strcmp(argv[4], "--detailed") == 0) || (argv[5] != NULL && strcmp(argv[5], "--detailed") == 0))    scan_data.show_status = SHOW;
		scan_data.show_status = HIDE;	// If --detailed not mentioned
		// - - - * - - -
		
        scan_port_range(&scan_data);
        return 0;
    }

    printf("Scan complete.\n");
    return 0;
}
