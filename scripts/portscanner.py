# - - - * - - -
# Port Scanner
# - - - * - - - 
# 
# System: All (Requires python-nmap to be installed)
# Platform: All (Requires python-nmap to be installed)
#
# Made for testing purposes
#
# - - - * - - -

from sys import argv as arguments, platform
from re import match as RegexMatch
try:
    from nmap import PortScanner
except ModuleNotFoundError:
    # If the python-nmap major dependency is not found, we ask the user to install them

    print("[!] python-nmap is not installed\n    Install using: python3 -m pip install python3-nmap\n")

# Defining the required functions
# - - -
def isHostsValid(host: str) -> bool:
    """
    This function validates whether a given host address is valid or not.
    It returns then true / false.
    
    Syntax to use this:

    if isHostsValid("192.168.1.1"):
        # STATEMENTS
    else:
        # STATEMENTS

    """
    
    return bool(RegexMatch( r"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?:\/(?:3[0-2]|[12]?[0-9]))?$", host ))

def scanPorts(hosts, ports = "all", arguments = ""):
    """
    This function scans the given target host and then returns a
    detailed and sectional form of data.
    This function ensures that only alive / available hosts are
    visible.

    Output format:
    
    192.168.1.1 |  22 (closed)  |  80 (closed)
    192.168.1.21 |  22 (closed)  |  80 (closed) | 22 (closed)

    """

    if ports == "all":
        ports_ = "1-65535"
        arguments = "-T4"
    elif ports == "ssh":
        ports_ = "22,2022,8022"
    elif ports == "http":
        ports_ = "80,443,3000,5000,7001,7002,8000,8001,8080,8088,8443,8888,9000"
    elif ports == "rdp":
        ports_ = "3389,3390,3391,3392"
    elif ports == "database":
        ports_ = "1433,1521,3306,5432,6379,27017"
    elif ports == "ftp":
        ports_ = "21,139,445,2049"
    
    # Starting the scan
    print(f"[~] Scanning for {hosts} for the ports ({ports}):\n")
    portscanner = PortScanner()
    portscanner.scan(hosts = hosts, ports = ports_, arguments = arguments)

    # Iterating with the results
    if ports == "all":
        # Port scanning for all the hosts

        for host in portscanner.all_hosts():
            if portscanner[host]["status"]["state"] == "up":
                print(f"[\033[0;93m*\033[0m] {host}")
                for port, data in portscanner[host]["tcp"].items():
                    print("\t%6d (%9s)" %(port, data["state"]))
    else:
        # Displaying port scan results for all other ports which are small
        # in size as for their range

        for host in portscanner.all_hosts():
            if portscanner[host]["status"]["state"] == "up":
                head = False
                for port, data in portscanner[host]["tcp"].items():
                    if data["state"] == "open" or data["state"] == "filtered":
                        if not head:
                            print("%18s " %host, end = ' ')
                            head = True
                            print("|%6d (%9s)" %(port, data["state"]), end = ' ')
                if head:
                    print()
# - - -

if __name__ == "__main__":
    if len(arguments) < 2:
        print("[!] No Arguments were provided")
    else:
        hosts, ports = "", "all"
        for argument in arguments:
            # Iterating through each argument to get the value

            if argument == "--help" or argument == "help" or argument == "-h":
                # Displaying the help text in case user requested for it

                print("help")
                exit(0)
            elif isHostsValid(argument):
                hosts += argument
            elif argument in [ "database", "ssh", "ftp", "rdp", "http" ]:
                ports = argument
            continue

        if hosts == "":
            # If the hosts value is empty, then we raise an error

            print("[\033[0;91m!\033[0m] Please mention a host address / subnet to scan")
        scanPorts(hosts, ports)