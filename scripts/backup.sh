#!/bin/bash

# ------- * -------
#   BACKUP MAKER
# ------- * -------
#
# Only designed to make backups for my own user folder
#
# ------- * -------

# Ensure the script runs in a modern version of Bash
if (( BASH_VERSINFO[0] < 4 )); then
	printf "[\033[0;91mError\033[0;97m]\033[0m This script requires Bash 4.0 or higher\n" >&2
	exit 1
fi

# Initialize associative arrays for storing arguments
declare -A OPTIONS
declare -a POSITIONAL_ARGS

# Default parameters
OPTIONS[help]="false"
OPTIONS[exclude]="false"
OPTIONS[start]="false"
OPTIONS[verbose]=""
OPTIONS[copy_loc]=""
OPTIONS[remote]="aw-121@192.168.1.121"
OPTIONS[remote_loc]=""

function printUsage {
	echo -e
    echo -e "\033[0;96m===================================================================\033[0m"
    echo -e "\033[0;97m                       BACKUP MAKER ---- Help                      \033[0m"
    echo -e "\033[0;96m===================================================================\033[0m"
    echo -e "Only designed to create and securely transfer local user folder backups."
    echo -e ""
    echo -e "\033[0;93mUSAGE:\033[0m"
    echo -e "  ./backup.sh [OPTIONS] [\033[0;97m--exclude\033[0m file1 dir2 ...]"
    echo -e ""
    echo -e "\033[0;93mOPTIONS:\033[0m"
    echo -e "  \033[0;97m-h, --help\033[0m           Show this help configuration screen and exit."
    echo -e "  \033[0;97m--start, --backup\033[0m   Run the script immediately without a confirmation prompt."
    echo -e "  \033[0;97m-v, --verbose\033[0m       Enable verbose output mode during file compression."
    echo -e "  \033[0;97m--loc, --copy-loc\033[0m   Set local directory path to backup. "
    echo -e "                      \033[0;90mSyntax: --loc=/path OR --loc \"/path\" (Default: PWD)\033[0m"
    echo -e "  \033[0;97m--remote, --ssh\033[0m     Set target remote SSH user and host configuration."
    echo -e "                      \033[0;90mSyntax: --ssh=user@host OR --ssh \"user@host\"\033[0m"
    echo -e "  \033[0;97m--remote-loc\033[0m        Set destination file path on the remote SSH receiver."
    echo -e "                      \033[0;90mSyntax: --remote-loc=/path OR --remote-loc \"/path\"\033[0m"
    echo -e "  \033[0;97m--exclude\033[0m           Exclude specified trailing items from being archived."
    echo -e "                      \033[0;90mMust be placed right before file list patterns.\033[0m"
    echo -e ""
    echo -e "\033[0;93mNOTES:\033[0m"
    echo -e "  * A default remote SSH server configuration is hardcoded directly into "
    echo -e "    the script file for convenience."
    echo -e "  * To update this permanent default address, edit the internal script variable "
    echo -e "    directly within the file."
    echo -e "  * To bypass the default profile and deploy your backup to an alternate "
    echo -e "    SSH machine, declare the active destination inline using the \033[0;97m--ssh\033[0m "
    echo -e "    or \033[0;97m--remote\033[0m flag arguments."
    echo -e ""
    echo -e "\033[0;93mEXAMPLES:\033[0m"
    echo -e "  ./backup.sh --start --loc=\"/home/user/backend\" --remote-loc=\"/backups\""
    echo -e "  ./backup.sh --ssh=\"user@alternate-host\" --remote-loc=\"/custom-destination\""
    echo -e "  ./backup.sh --loc=\$(pwd) --exclude \"node_modules\" \".git\" \"dist\""
    echo -e ""
    echo -e "\033[0;96m===================================================================\033[0m"
}

function checkRemoteHost {
	# This function checks whether the user stated host is available or not

    local pattern="^[a-zA-Z0-9_-]+@[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$"
	if [[ ! $1 =~ $pattern ]]
	then
		echo $1
		printf "[\033[0;91mERROR\033[0;97m]\033[0m Provided remote address invalid\n"
		return 1
	fi

    host="${1#*@}"
	if ping -c 2 -W 1 "$host" > /dev/null 2>&1
	then
		# If the remote host is reachable
		return 0
	else
		# If the remote host is unreachable
		printf "[\033[0;91mERROR\033[0;97m]\033[0m Provided remote address is unreachable\n"
		return 1
	fi
}

# Parsing the arguments 
while (( "$#" )); do
	case "$1" in
	-h|--help)
		# Displaying the help menu

		printUsage
		exit 0
		;;
	# ------- * -------

	--start|--backup)
		# Starting the backup process without any prompt

		OPTIONS[start]="true"
		shift
		;;
	# ------- * -------

	-v|--verbose)
		# Doing the process in versbose mode

		OPTIONS[verbose]="-v"
		shift
		;;
	# ------- * -------

	--copy-loc|--loc)
		# Setting the location to do backup

		if [[ -n "$2" && "$2" != -* ]]
		then
			OPTIONS[copy_loc]="$2"
			shift 2
		else
			printf "[\033[0;91mERROR\033[0;97m]\033[0m Argument for $1 is missing." >&2
			exit 1
		fi
		;;
	# Magic pattern: Splits --loc="/home/user/scripts" into key/value automatically
	--copy-loc=*|--loc=*)
		OPTIONS[copy_loc]="${1#*=}"
		shift
		;;
	# ------- * -------

	--ssh|--remote)
		# Setting the remote backup SSH

		if [[ -n "$2" && "$2" != -* ]]
		then
			OPTIONS[remote]="$2"
			shift 2
		else
			printf "[\033[0;91mERROR\033[0;97m]\033[0m Argument for $1 is missing." >&2
			exit 1
		fi
		;;
	# Magic pattern: Splits --remote=user@hostname into key/value automatically
	--ssh=*|--remote=*)
		OPTIONS[remote]="${1#*=}"
		shift
		;;
	# ------- * -------

	--remote-loc)
		# Setting the location on remote SSH host

		if [[ -n "$2" && "$2" != -* ]]
		then
			OPTIONS[remote_loc]="${1#*=}"
			shift 2
		else
			printf "[\033[0;91mERROR\033[0;97m]\033[0m Argument for $1 is missing." >&2
			exit 1
		fi
		;;
	# Magic pattern: Splits --remote-loc="/location/to/destination" into key/value automatically
	--remote-loc=*)
		OPTIONS[remote_loc]="${1#*=}"
		shift
		;;
	# ------- * -------

	--exclude)
		# Keeping the exclude flag on, this will read all the other positional arguments if the directories or files are found

		OPTIONS[exclude]="true"
		shift
		;;
	# ------- * -------

	-*|--*)
		# Catch unsupported flags to prevent typos
		printf "[\033[0;91mError\033[0;97m]\033[0m Unsupported option: $1\n" >&2
		exit 1
		;;
	*)
		# Anything else is a positional argument (like file names or paths)
		POSITIONAL_ARGS+=("$1")
		shift
		;;
	esac
done

# Setting custom SSH location for backup
if [[ ! -z ${OPTIONS[remote]} ]]
then
	checkRemoteHost ${OPTIONS[remote]}  || eval "exit"

	# Curating the remote_loc
	OPTIONS[remote_loc]="${OPTIONS[remote_loc]#\"}"
	OPTIONS[remote_loc]="${OPTIONS[remote_loc]%\"}"
	printf "\033[0;97m[\033[0;93m~\033[0;97m]\033[0m Setting remote backup location - \033[92m${OPTIONS["remote"]}\033[0;97m:\033[0;93m${OPTIONS[remote_loc]}\033[0m\n"
fi

# If the location on local machine where we need to do backup is not provided, then we assume PWD
if [[ -z ${OPTIONS[copy_loc]} ]]; then OPTIONS[copy_loc]=$( pwd ); fi
printf "\033[0;97m[\033[0;93m~\033[0;97m]\033[0m Backing up: \033[0;97m${OPTIONS[copy_loc]}\033[0m\n"

# If exclude files/dirs are not yet set or marked false
if [[ -z "${OPTIONS[exclude]}" || "${OPTIONS[exclude]}" == "true" ]]; then
    # Clear the "false" value before appending
    OPTIONS[exclude]=""
    
	printf "\033[0;97m[\033[0;93m~\033[0;97m]\033[0m Excluding: \033[0;95m"
    # Loop through the traditional indexed array
    for i in "${!POSITIONAL_ARGS[@]}"
    do
    	printf "${POSITIONAL_ARGS[$i]} "
        OPTIONS[exclude]+="--exclude=${POSITIONAL_ARGS[$i]} "
    done
    printf "\033[0m\n"
else
	# If no files are given to exclude
	OPTIONS[exclude]=""
fi

# Prompting the user to continue
if [[ ${OPTIONS[start]} == "false" ]]
then
	printf "\033[0;96mDo you want to start the backup (Y/n)?\033[0m "
	read -r -n 1 response
	echo ''
    case "$response" in
    	[nN][oO]|[nN])
            printf "[\033[0;91mERROR\033[0m] Backup cancelled by user\n"
            exit 0
            ;;
    esac
fi

function main {
	# The main process

	tarfile="/tmp/$( whoami )@$( hostname -I | awk '{ print $1 }' )_$( date +"%Y%m%d_%H%M%S" ).tar.xz"
	eval "tar ${OPTIONS[verbose]} ${OPTIONS[exclude]} -cJf $tarfile ${OPTIONS[copy_loc]}"
	if [[ "$?" -eq 0 ]]
	then
		# If backup is created at the desired location

		printf "\033[0;97m[\033[0;92m*\033[0;97m]\033[0m Compressed to: \033[0;92m$tarfile\033[0m\n"
	else
		# If backup  files creation fails
		
		printf "[\033[0;91m!\033[0m] Failed to create the backup zip\n"
		exit 1
	fi

	# Copying to the remote location
	printf "\033[0;97m[\033[0;93m~\033[0;97m]\033[0m Copying to remote location\n"
	eval "scp $tarfile ${OPTIONS[remote]}:${OPTIONS[remote_loc]}"
	if [[ "$?" -eq 0 ]];
	then
		# If copying the backup zip successful over SSH

		printf "[ ------- \033[0;92mPROCESS FINISHED\033[0m] ------- ]\n"
	else
		# If copying the backup over SSH fails
		
		printf "[\033[0;91m!\033[0m] Copying backup file over SCP failed..\033[0;91mTerminating...\033[0m\n"
	fi
	rm $tarfile
	exit 0
}

# Starting the process
main
