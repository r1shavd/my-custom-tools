#!/bin/bash

# ------- * -------
#     COMPILE.SH
# ------- * -------
# 
# This script compiles the C src code given in the first argument.
# Constantly checks if the file is updated, then compiles it again (every 30 seconds)
#
# Author: Rishav (github.com/r1shavd)
# ------- * -------


# ------- * -------
#     HELP MENU
# ------- * -------
#
# Displays this information if argument used:
# >>> ./compile.sh --help
#
# Compile a src file
# >>> ./compile.sh <src-filename>
#
# Compile + Execute the src file
# >> ./compile.sh <src-filename> --execute
#
# Note: All src files are default stored at src/ sub-directory under the project.
# 		Edit this script in case of modifications made to the project structure.
#
# ------- * -------
 

# Src file validation / Argument parsing
if [[ -z $1 ]]
then
	# If the source file name at argument not provided
	printf "[\033[0;91m!\033[0m] ERROR: Source file not provided\nUse '-h' or '--help' for usage\n"
	exit
elif [[ $1 == "--help" || $1 == "-h" ]]
then
	# If the user called for help argument
	cat << 'EOF'
 ------- * -------
     HELP MENU
 ------- * -------

 Displays this information if argument used:
 >>> ./compile.sh --help

 Compile a src file
 >>> ./compile.sh <src-filename>

 Compile + Execute the src file
 >>> ./compile.sh <src-filename> --execute

 Adding custom standar lib, linker support (argument for gcc)
 >>> ./compile.sh <src-filename> --compiler-args "arguments"

 Note: All src files are default marked at src/ sub-directory under the project.
       Edit this script in case of modifications made to the project structure.
       This script compiles by default using GCC, and default custom header files
       are marked at include/ sub-directory under the same project. 
 ------- * -------
EOF
	exit
else
	if [[ ! -f "src/$1" ]]
	then
		# If the mentioned file doesn't exists
		printf "[\033[0;91m!\033[0m] ERROR: Source file not accessible\n"
		exit
	else
		# Creating the sub-directories if they dont exiss
		if [[ ! -d "bin/" ]]; then mkdir bin/; fi
		if [[ ! -d "tests/" ]]; then mkdir tests/; fi
		if [[ ! -d "include/" ]]; then mkdir include/; fi
	fi
fi

binloc=$( echo $1 | sed -E 's/\.(c|cpp)$//' )
eval "gcc src/$1 -I include/ -o bin/$binloc -std=c99"

# Creating a hash for the file (to further check modification)
eval "md5sum src/$1 > tests/${1}.hash"

if [[ ! -f bin/$binloc ]]
then
	# If the code compilation error occurs
	printf "[\033[0;91m!\033[0m] Code not compiled -- \033[0;91mexiting\033[0m...\n"
	exit
else
	printf "[\033[0;92m*\033[0m] Compiled at: \033[0;92m$binloc\033[0m\n"

	# Running the compiled binary, only if the user passed argument
	if [[ ! -z $2 && $2 == "--execute" ]]; then eval "clear"; printf "[\033[0;93m~\033[0m] OUTPUT:\n\n"; eval "./bin/$binloc"; echo; fi

fi

printf "[\033[0;93m~\033[0m] Checking for changes in the src file\n"
while true
do
	sleep 30	# Wait time for recheck 30 seconds
	if [[ ! -f bin/$binloc ]]
	then
		# If the compiled binary is not available
		printf "[\033[0;91m!\033[0m] Binary (\033[0;93m$binloc)\033[0m either moved or missing\n"
		read -r -p "Press Y to continue / enter to exit" choice
		if [[ ! choice =~ ^[Yy]([Ee][Ss])?$ ]]
		then
			printf "Exiting compile script..."
			eval "rm tests/${1}.hash"
			exit	
		fi
		
	else
		# If the file is present
		eval "md5sum --status -c tests/${1}.hash"
		if [[ $? == 1 ]]
		then
			# The source file is modified we compile it again
			printf "[\033[0;93m~\033[0m] Source file updated -- compiling again...\n"
			eval "gcc src/$1 -I include/ -o bin/$binloc -std=c99"
			printf "[\033[0;92m*\033[0m] Compiled at: \033[0;92m$binloc\033[0m\n"
			eval "md5sum src/$1 > tests/${1}.hash"
		
			# Running the compiled binary, only if the user passed argument
			if [[ ! -z $2 && $2 == "--execute" ]]; then eval "clear"; printf "[\033[0;93m~\033[0m] OUTPUT:\n\n"; eval "./bin/$binloc"; echo; fi
		fi
	fi
done
