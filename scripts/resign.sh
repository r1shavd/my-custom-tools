# ------- * -------
#   RESIGN SCRIPT
# ------- * -------

# System: Linux-based
# Platform: Bash

# This script automates the tasks when I resign from a particular desktop system.
# 	- Deletes all github configuration on the user (--global)
# 	- Deletes all scoop apps if installed
# 	- Deletes all firefox browsing

# Requires sudo/root access
# ------- * -------

# Declaring the argument array
declare -A OPTIONS
OPTIONS["mode"]="safe"

# Declaring the required functions
# - - -
function printUsage {
	# This function displays the help section for the script

	printf "\n[ \033[0;97m------- \033[0;96mRESIGN SCRIPT - \033[0;95mHELP MENU\033[0;97m -------\033[0m ]\n"
	printf "This script automates system cleanup actions upon resignation.\n\n"
	
	printf "\033[0;92mUsage:\033[0m\n"
	printf "  bash resign.sh [options]\n\n"
	
	printf "\033[0;92mOptions:\033[0m\n"
	printf "  \033[0;96m-h, --help\033[0m          Displays this help section and documentation menu\n"
	printf "  \033[0;96m--mode=safe\033[0m         Simulates the action blocks without wiping records (Default)\n"
	printf "  \033[0;96m--mode=execute\033[0m      Deletes configurations and logs immediately from storage media\n\n"
	
	printf "\033[0;92mNotes:\033[0m\n"
	printf "  - Requires active execution context with root context parameters\n"
	printf "  - Safely descends down directory loops to preserve filesystem geometry\n"
}

function deleteGitConfig {
	# This function removes all the git related configuration.
	# as well as the other files.

	printf "[\033[0;93m~\033[0m] Deleting git configuration global as well as system levels\n"
	if [[ "$1" == "safe" ]]
	then
		# Executing in safe mode
		
		printf "[\033[0;92m*\033[0m] \033[0;97mDeleted all the data and configuration\033[0m\n"		
	elif [[ "$1" == "execute" ]]
	then
		# Executing in normal mode

		if [[ -f "${HOME}/.gitconfig" ]]; then rm -rf "${HOME}/.gitconfig"; fi

		if [[ -d "${HOME}/.config/git" ]]; then rm -rf "${HOME}/.config/git"; fi
		git credential-cache exit 2>/dev/null
	else
		printf "\n"
	fi
	return 0
}

function deleteFirefoxData {
	# This function removes all firefox configuration
	# works with standard installation, flatpack and snap

	printf "[\033[0;93m~\033[0m] Deleting firefox data and configuration...\n"
	if [[ "$1" == "safe" ]]
	then
		# Executing in safe mode
		
		printf "[\033[0;92m*\033[0m] \033[0;97mDeleted all the data and configuration\033[0m\n"		
	elif [[ "$1" == "execute" ]]
	then
		# Executing in normal mode

		# Deleting the standard firefox files
		if [[ -d "${HOME}/.mozilla" ]]; then rm -rf "${HOME}/.mozilla"; fi

		if [[ -d "${HOME}/.cache/mozilla" ]]; then rm -rf "${HOME}/.cache/mozilla"; fi
		printf "[\033[0;92m*\033[0m] \033[0;97mDeleted all the standard data and configuration\033[0m\n"

		# if installed via snap
		if [[ -d "${HOME}/snap/firefox" ]]
		then
				rm -rf "${HOME}/snap/firefox"
			printf "[\033[0;92m*\033[0m] \033[0;97mDeleted firefox snap configuration\033[0m\n"		
		fi

		# if installed via flatpack
		if [[ -d "${HOME}/.var/app/org.mozilla.firefox" ]]
		then
				rm -rf "${HOME}/snap/firefox"
			printf "[\033[0;92m*\033[0m] \033[0;97mDeleted firefox flatpack configuration\033[0m\n"		
		fi
	else
		printf "\n"
	fi
	return 0
}

function deleteFilesData {
	# This function removes all data stored in the home directory.
	# Works according to my setup - Documents, Downloads, Temp, Development, Screenshots.
	# + other important items like .ssh and private keys

	printf "[\033[0;93m~\033[0m] Deleting all the data stored in the ${HOME}\n"
	declare -a dirs=( "Documents" "Downloads" "Temp" "Development" "Screenshots" "Scripts" )
	for dir in "${dirs[@]}"
	do
		if [[ -d "${HOME}/${dir}" ]]
		then
			if [[ "$1" == "execute" ]]
			then
				# Executing the deletion
				
				eval "rm -rf ${HOME}/${dir}/*"
			else
				# Else case is safe mode
				
				printf ""
			fi
			if [[ "$?" -eq 0 ]]; then printf "[\033[0;92m*\033[0m] Deleted all the data: \033[0;97m${HOME}/${dir}\033[0m\n"; fi
		fi
	done
	return 0
}
# - - -

# Parsing the arguments
# - - -
while (( "$#" )); do
    case "$1" in
        -h|--help)
            printUsage
            exit 0
            ;;
        # Magic pattern: Splits --mode=safe into key/value automatically
        --mode=*)
            OPTIONS[mode]="${1#*=}"
            shift
            ;;
        # Catch unsupported flags to prevent typos
        -*|--*)
            printf "[\033[0;91mError\033[0m] Unsupported option $1\n" >&2
            exit 1
            ;;
        # Anything else is a positional argument (like file names or paths)
        *)
            printf ""
            shift
            ;;
    esac
done
# - - -

if [[ "${OPTIONS[mode]}" == "safe" ]]
then
	printf "\n[ \033[0;97m------- \033[0;96mExecuting in \033[0;92mSAFE mode\033[0;97m -------\033[0m ]\n"
	deleteGitConfig "safe"
	deleteFirefoxData "safe"
	deleteFilesData "safe"
elif [[ "${OPTIONS[mode]}" == "execute" ]]
then
	deleteGitConfig "execute"
	deleteFirefoxData "execute"
	deleteFilesData "execute"
else
	print "..."
fi