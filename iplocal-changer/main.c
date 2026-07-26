/*
 * iplan-changer
 *
 * Author: Rishav Das (r1shavd)
 *
 * This is a single file program which allows to change IP address of
 * the local machine as in ipv4 static address.
 *
 * usage:    ./iplan-changer 192.168.11.141
 *
 * @note    This doesn't uses my helpers lib (as it has GNU/Linux
 *          dependency, and this is cross-platform).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void set_static_ip(
        const char* interface,
        const char* ip, const char* subnet,
        const char* gateway, const char* dns)
{
    printf("Configuring static IP %s on interface '%s'...\n", ip, interface);

    char cmd[512];
    int ret;

#if defined(_WIN32) || defined(__CYGWIN__)
    // --- WINDOWS ---
    
    // Construct and execute IP/Gateway command
    if (snprintf(cmd, sizeof(cmd), "netsh interface ip set address name=\"%s\" static %s %s %s 1", 
                 interface, ip, subnet, gateway) >= (int)sizeof(cmd)) {
        fprintf(stderr, "Error: IP command buffer overflow prevented.\n");
        return;
    }

    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: Execution failed. Ensure you are running as Administrator.\n");
        return;
    }
    printf("Successfully updated IP configuration.\n");

    // Construct and execute DNS command
    if (dns != NULL) {
        printf("Configuring DNS server %s...\n", dns);
        if (snprintf(cmd, sizeof(cmd), "netsh interface ip set dns name=\"%s\" static %s", 
                     interface, dns) >= (int)sizeof(cmd)) {
            fprintf(stderr, "Error: DNS command buffer overflow prevented.\n");
            return;
        }

        ret = system(cmd);
        if (ret == 0) {
            printf("Successfully updated DNS configuration.\n");
        } else {
            fprintf(stderr, "Error: Failed to update DNS configuration.\n");
        }
    }

#else
    // --- LINUX ---

    // Constructing set up
    if (snprintf(cmd, sizeof(cmd), "ip addr add %s/%s dev %s", ip, subnet, interface) >= (int)sizeof(cmd)) {
        
        // Alternative traditional command if 'ip' utility isn't preferred:
        // "ifconfig %s %s netmask %s up"
        if (snprintf(cmd, sizeof(cmd), "ifconfig %s %s netmask %s up", interface, ip, subnet) >= (int)sizeof(cmd)) {
            fprintf(stderr, "Error: IP command buffer overflow prevented.\n");
            return;
        }
    }

    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: Execution failed. Ensure you are running with sudo/root privileges.\n");
        return;
    }

    // Construct and execute Gateway command
    if (gateway != NULL) {
        if (snprintf(cmd, sizeof(cmd), "ip route add default via %s dev %s", gateway, interface) >= (int)sizeof(cmd)) {
            
            // Alternative traditional command: "route add default gw %s %s"
            if (snprintf(cmd, sizeof(cmd), "route add default gw %s %s", gateway, interface) >= (int)sizeof(cmd)) {
                fprintf(stderr, "Error: Gateway command buffer overflow prevented.\n");
                return;
            }
        }
        system(cmd); 
    }
    printf("Successfully updated IP and Gateway configuration.\n");

    // Writing to /etc/resolv.conf
    if (dns != NULL) {
        printf("Configuring DNS server %s...\n", dns);
        if (snprintf(cmd, sizeof(cmd), "echo \"nameserver %s\" > /etc/resolv.conf", dns) >= (int)sizeof(cmd)) {
            fprintf(stderr, "Error: DNS command buffer overflow prevented.\n");
            return;
        }

        ret = system(cmd);
        if (ret == 0) {
            printf("Successfully updated DNS configuration.\n");
        } else {
            fprintf(stderr, "Error: Failed to update DNS configuration.\n");
        }
    }
#endif
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "[!] Please mention a new static IP\n");
        return 1;
    }

    if (strncmp(argv[1], "192.168.", 8) == 0) {

#if defined(_WIN32) || defined(__CYGWIN__)
        const char* default_interface = "Ethernet";
#else
        const char* default_interface = "eth0"; // Change as per ifconfig (or wlan0, enp3s0, etc.)
#endif

        set_static_ip(default_interface, argv[1], "255.255.255.0", "192.168.1.1", "8.8.8.8");
    } else {
        fprintf(stderr, "[!] Invalid IP. Must start with 192.168.\n");
        return 1;
    }

    return 0;
}
