import re

def getHosts(sshclient, discoveryMethod = "new"):
    """
    Extracts the host list from all .ssh/known_hosts files across the system.
    Filters a list to keep only valid, external host addresses.
    Removes blanks, localhost addresses, and domain names.

    Old method:
    Fecthes the appropriate hosts via the .ssh/known_hosts file across various location of the device. This is ssh history for the current device.

    New method:
    Scraps the system ARP cache
    """ 

    if discoveryMethod.lower() in ["old", "o"]:
        stdin, stdout, stderr = sshclient.exec_command(
            "for i in $( locate .ssh/known_hosts ); do if [[ -f $i ]]; then cat $i | awk '{ print $1 }'; fi; done"
        )
    else:
        stdin, stdout, stderr = sshclient.exec_command(
            "for host in $( arp -e | awk 'NR > 1 { print $1 }' ); do if [[ -n $( nmap -p 22 $host | grep -i 'open' ) ]]; then echo $host; fi; done"
        )
    
    # Read, decode, and split lines cleanly
    output = stdout.read().decode("utf-8")
    
    # Fix: Use a list comprehension to strip whitespace and drop empty lines safely
    hosts = [line.strip() for line in output.split('\n') if line.strip()]

    ip_pattern = re.compile(r'^(\d{1,3}\.){3}\d{1,3}$')
    x = []

    for host in hosts:
        # Match IPv4 structure
        if ip_pattern.match(host):
            octets = host.split('.')

            # Filter out localhost/loopback range (127.x.x.x)
            if octets[0] == '127':
                continue
            # Filter out local default route
            if host in ['0.0.0.0', '192.168.1.1']:
                continue
            # Verify octets are within valid 0-255 range
            if all(0 <= int(octet) <= 255 for octet in octets):
                # Check against the set of used hosts
                x.append(host)

    return x