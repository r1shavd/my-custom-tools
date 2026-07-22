#include "portscan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static enum ScannerMode
{
    // Scan modes

    DEFAULT_MODE = 0,
    SINGLE_SCAN,
    MULTI_SCAN,
    PRIORITY_SCAN,
    RANGE_SCAN,
    ALL_SCAN,
};


void display_usage()
{
    printf("Usage:\n");
    printf("  Way 1: ./portscanner --host <host> --ports <port1> <port2> ...\n");
    printf("  Way 2: ./portscanner --host <host> --port <port>\n");
    printf("  Way 3: ./portscanner --host <host> --port-range <start> <end>\n");
    printf("  Way 4: ./portscanner --host <host> --port-range all\n");

    printf("  Way 5: ./portscanner --host <host> --ports-priority\n");
}

int main(int argc, char *argv[])
{
    Scandetails scan_data = {0};
    scan_data.ip = NULL;
    scan_data.show_status = DETAILS_HIDE;

    int* ports = NULL;
    char* endptr;
    
    int port_count = 0;
    enum ScannerMode mode = DEFAULT_MODE;

    static struct option long_options[] = {
        {"help",       no_argument,       0, 'h'},
        {"host",       required_argument, 0, 'i'},
        {"port",       required_argument, 0, 'p'},
        {"ports",      no_argument,       0, 'm'},
        {"ports-priority", no_argument,   0, 'P'},
        {"port-range", required_argument, 0, 'r'},
        {"detailed",   no_argument,       0, 'd'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                scan_data.ip = optarg;

                if (isIPV4(scan_data.ip) == 0) {
                    fprintf(stderr, "[!] Invalid ip address mentioned, use ipv4\n");
                    return EXIT_FAILURE;
                }

                break;
                
            case 'p':
                mode = SINGLE_SCAN;
                scan_data.current_port = strtol(optarg, &endptr, 10);
                
                if (endptr == optarg || *endptr != '\0') {
                    fprintf(stderr, "[!] Invalid port, use --help for help\n");
                    return EXIT_FAILURE;
                }

                port_count = 1;
                break;
                
            case 'm':
                mode = MULTI_SCAN;
                
                // We consume arguments following '--ports' until another flag or end of argv
                while (optind < argc && argv[optind][0] != '-') {

                    ports = realloc(ports, (port_count + 1) * sizeof(int));
                    ports[port_count] = strtol(argv[optind], &endptr, 10);
                    
                    if (endptr == argv[optind] || *endptr != '\0') {
                        fprintf(stderr, "[!] Invalid port, use --help for help\n");
                        free(ports);
                        ports = NULL;
                        return EXIT_FAILURE;
                    }

                    port_count++;
                    optind++;
                }

                break;
               
            case 'P':
                mode = PRIORITY_SCAN;
                
                int priority_list[] = {
                    20, 21, 22, 23, 25, 53, 80, 81,
                    110, 123, 143, 443, 445, 587,
                    993, 995, 1433, 2222, 3000, 3001,
                    3306, 3389, 3390, 5000, 5432, 8000,
                    8080, 8443, 8888, 9000, 27017, 33389
                };
                
                port_count = sizeof(priority_list) / sizeof(priority_list[0]);
                
                ports = malloc(sizeof(priority_list));
                if (ports == NULL) {
                    fprintf(stderr, "[!] Memory allocation failed\n");
                    return EXIT_FAILURE;
                }
                memcpy(ports, priority_list, sizeof(priority_list));
                break;

            case 'r':
                if (strcmp(optarg, "all") == 0) {
                    mode = ALL_SCAN;
                    
                    scan_data.current_port = 1;
                    scan_data.last_port = 65535;
                } else {
                    mode = RANGE_SCAN;

                    scan_data.current_port = strtol(optarg, &endptr, 10);
                    if (endptr == optarg || *endptr != '\0') {
                        fprintf(stderr, "[!] Invalid port, use --help for help\n");
                        return EXIT_FAILURE;
                    }

                    // The next argument must be the end of the range
                    if (optind < argc && argv[optind][0] != '-') {
                        scan_data.last_port = strtol(argv[optind], &endptr, 10);
                        if (endptr == argv[optind] || *endptr != '\0') {
                            fprintf(stderr, "[!] Invalid port, use --help for help\n");
                            return EXIT_FAILURE;
                        }

                        optind++;
                    } else {
                        fprintf(stderr, "Error: --port-range requires a ending port or 'all'.\n");
                        display_usage();
                        return EXIT_FAILURE;
                    }
                }
                break;
            case 'd':
                scan_data.show_status = DETAILS_SHOW;
                break;
            case 'h':
            default:
                display_usage();
                return EXIT_FAILURE;
        }
    }

    if (!scan_data.ip || mode == DEFAULT_MODE) {
        fprintf(stderr, "Error: Missing required arguments.\n");
        display_usage();
        free(ports);
        ports = NULL;
        return EXIT_FAILURE;
    }

    printf("[~] Scanning target - %s\n", scan_data.ip);
    switch (mode) {
        case SINGLE_SCAN:
            scan_data.show_status = DETAILS_SHOW;
            scanPorts(scan_data.ip, scan_data.current_port, scan_data.show_status);
            break;
        
        case MULTI_SCAN:
        case PRIORITY_SCAN:
            scan_data.show_status = DETAILS_SHOW;
            
            for (int i = 0; i < port_count; i++) {
                scanPorts(scan_data.ip, ports[i], scan_data.show_status);
            }
            
            printf("\n");
            break;
        
        case RANGE_SCAN:
        case ALL_SCAN:
            scan_port_range(&scan_data);
            break;
        
        case DEFAULT_MODE:
        default:
            display_usage();
            free(ports);
            break;
    }

    // Clean up
    if (ports != NULL) {
        free(ports);
        ports = NULL;
    }
    return EXIT_SUCCESS;
}
