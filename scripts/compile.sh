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

# Declaring the important flags
# - - -
declare -A OPTIONS
declare -a POSITIONAL_ARGS

# Default options
OPTIONS[std]="-std=c99"
OPTIONS[include]="-I include/"
OPTIONS[output]="bin/a"
OPTIONS[src]="src/main.c"
OPTIONS[hash]=""
OPTIONS[execute]="false"
# - - -

# Defining the required functions
# - - -
function compileCode {
	# This function compiles the code as per the said arguments
	# and files

	gcc ${OPTIONS[src]} ${OPTIONS[include]} ${OPTIONS[std]} -o ${OPTIONS[output]} -Wall
	if [[ "$?" -eq 1 ]]
	then
		# If any error occurs with GCC compilation

		printf "[!\033[0;91m!\033[0m] Error occured during compilation. \033[0;91mQUITING...\033[0m\n"
		cleanupCompilation
		exit 1
	fi

	if [[ ! -f "${OPTIONS[output]}" ]]
	then
		# If there is no output binary found at the desired location

		printf "[\033[0;91m!\033[0m] Hash file either not found or missing: \033[0;91m%30s\nQUITING...\033[0m\n" "${OPTIONS[output]}"
		cleanupCompilation
		exit 1
	fi

	# If the binary available at the desired location OPTIONS[output]
	printf "[\033[0;92m*\033[0m] Compiled at: \033[0;92m%-30s\033[0m\n" "${OPTIONS[output]}"

	# Running the compiled binary, only if the user passed argument
	# -> Proceeding with monitoring
	executeBinary
	monitorSourceFile
}

function cleanupCompilation {
	# This function cleans up the various log files and variables declared

	eval "rm -rf ${OPTIONS[hash]}"
	return "$?"
}

function executeBinary {
	# This function runs the compiled binary only if the flag is marked
	# to be executed OPTIONS[execute]
	#
	# >>> ./compile.sh <src-files> --execute

	if [[ "${OPTIONS[execute]}" == "true" ]]
	then
		eval "clear"
		printf "[\033[0;93m~\033[0m] OUTPUT:\n\n"
		eval "./${OPTIONS[output]}"
		echo
	fi
}

function monitorSourceFile {
	# This function modifies the main source file for any changes and
	# then re-compiles the src file.

	while true
	do
		# Constantly monitoring (with every 15 seconds)
		sleep 15
		printf "[\033[0;93m~\033[0m] Checking for changes...\n"
		for hash in ${OPTIONS[hash]}
		do
			# Looping through each hash file, to check updates

			# Checking if hash file exists
			if [[ ! -f "${hash}" ]]
			then
				# If the hash file is not available

				printf "[\033[0;91m!\033[0m] Hash file either not found or missing: \033[0;91m%30s\nQUITING...\033[0m\n" "${hash}"
				cleanupCompilation
				exit 1
			fi

			# Validating the md5sum hash
			if ! md5sum --status -c "${hash}"
			then
				# If change is detected, then we update the binary and update the hash

				printf "[\033[0;93m~\033[0m] \033[0;97mChanges detected: \033[0;93m%30s\033[0m\n\033[0;96mRE-COMPILING...\033[0m\n" "src/${hash:6:-5}"

				eval "md5sum src/${hash:6:-5} > ${hash}"
				compileCode
			fi
		done
	done
}
# - - -

# Parsing the arguments
# - - -
# Looping through the arguments
while (( "$#" )); do
    case "$1" in
        -h|--help)
            # If the user called for help argument
            printf "   \033[0;94m------- * -------\033[0m\n"
            printf " \033[1;96mCompile script - Help\033[0m\n"
            printf "   \033[0;94m------- * -------\033[0m\n\n"
            printf " Displays this information if argument used:\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --help\n\n"
            printf " Compile a src file\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src=\033[0;92m<src-filename>\033[0m\n\n"
            printf " Compile + Execute the src file\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m --execute\n\n"
            printf " Specify a custom output binary name/path\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m --output \033[0;92m<out-filename>\033[0m\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m -o \033[0;92m<out-filename>\033[0m\n\n"
            printf " Pass custom header search paths (can be used multiple times)\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m --include \033[0;92m<dir-path>\033[0m\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m -I=\033[0;92m<dir-path>\033[0m\n\n"
            printf " Specify a language standard for compilation (e.g., c11, c99)\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m --std \033[0;92m<std-version>\033[0m\n"
            printf " \033[0;93m>>>\033[0m ./compile.sh --src \033[0;92m<src-filename>\033[0m -S=\033[0;92m<std-version>\033[0m\n\n"
            printf " \033[0;91mNote:\033[0m All src files are default marked at \033[0;96msrc/\033[0m sub-directory under the project.\n"
            printf "       Edit this script in case of modifications made to the project structure.\n"
            printf "       This script compiles by default using \033[1mGCC\033[0m, and default custom header files\n"
            printf "       are marked at \033[0;96minclude/\033[0m sub-directory under the same project. \n"
            exit 0
            ;;
        --execute)
        	OPTIONS[execute]="true"
        	shift
        	;;
        --src)
            if [[ -n "$2" && "$2" != -* ]]; then
                OPTIONS[src]="$2"
                shift 2
            else
                echo "Error: Argument for $1 is missing." >&2
                exit 1
            fi
            ;;
        # Magic pattern: Splits --src=src/main.c into key/value automatically
        --src=*)
            OPTIONS[src]="${1#*=}"
            shift
            ;;
        -o|--output)
            if [[ -n "$2" && "$2" != -* ]]; then
                OPTIONS[output]="$2"
                shift 2
            else
                echo "Error: Argument for $1 is missing." >&2
                exit 1
            fi
            ;;
        # Magic pattern: Splits --output=/tmp/binary into key/value automatically
        --output=*)
            OPTIONS[output]="${1#*=}"
            shift
            ;;
        -I|--include)
            if [[ -n "$2" && "$2" != -* ]]; then
                OPTIONS[include]="${OPTIONS[include]} $2"
                shift 2
            else
                echo "Error: Argument for $1 is missing." >&2
                exit 1
            fi
            ;;
        # Magic pattern: Splits --include=include/linux/ into key/value automatically
        --include=*)
            OPTIONS[include]="${OPTIONS[include]} ${1#*=}"
            shift
            ;;
        -S|--std)
            if [[ -n "$2" && "$2" != -* ]]; then
                OPTIONS[std]="-std=$2"
                shift 2
            else
                echo "Error: Argument for $1 is missing." >&2
                exit 1
            fi
            ;;
        # Magic pattern: Splits --std=c11 into key/value automatically
        --std=*)
            OPTIONS[std]="-std=${1#*=}"
            shift
            ;;
        # Catch unsupported flags to prevent typos
        -*|--*)
            echo "Error: Unsupported option $1" >&2
            exit 1
            ;;
        # Anything else is a positional argument (like file names or paths)
        *)
            POSITIONAL_ARGS+=("$1")
            shift
            ;;
    esac
done

# Validating the src files (these are stored in src/ folder by default)
if [[ ${#POSITIONAL_ARGS[@]} -eq 0 ]]
then
    # No positional arguments provided, thus going with default

    if [[ ! -f "${OPTIONS[src]}" ]]
    then
    	printf "[\033[0;91m~\033[0m] No arguments given. Atleast put a default file at \033[0;93msrc/main.c\033[0m\n"
    	printf "\033[0;91mQUITING...\033[0m\n"
		exit 1
    fi
else
    # Loop through the traditional indexed array
    OPTIONS[src]=""
    for i in "${!POSITIONAL_ARGS[@]}"
	do
		printf "[\033[0;93m~\033[0m] Validating \033[0;97m%-30s\033[0m: " "src/${POSITIONAL_ARGS[$i]}"
        if [[ -f "src/${POSITIONAL_ARGS[$i]}" ]]
        then
        	# If the src file is found
        	
        	printf "\033[0;92mValidated\033[0m\n"
        	OPTIONS[src]="${OPTIONS[src]} src/${POSITIONAL_ARGS[$i]}"

        	# Generating hash for each src file
       		eval "md5sum src/${POSITIONAL_ARGS[$i]} > tests/${POSITIONAL_ARGS[$i]}.hash"
        	OPTIONS[hash]="${OPTIONS[hash]} tests/${POSITIONAL_ARGS[$i]}.hash"
        else
        	# If the src file is not found

        	printf "\033[0;91mNot found\033[0m\n"
        	printf "[\033[91m!\033[0m] src files not found. QUITING...\n"
        	cleanupCompilation
        	exit 1
        fi
    done
fi

# Populating the project directory, if the required directories dont exists
if [[ "${OPTIONS[ouput]:0:4}" == "bin/a" && ! -d "bin/" ]]; then mkdir bin/; fi
if [[ ! -d "tests/" ]]; then mkdir tests/; fi
if [[ "${OPTIONS[include]}" == "include/" && ! -d "include/" ]]; then mkdir include/; fi
# - - -

# First compilation
compileCode