#!/bin/bash

# Ensure the script runs in a modern version of Bash
if (( BASH_VERSINFO[0] < 4 )); then
    echo "Error: This script requires Bash 4.0 or higher for associative arrays." >&2
    exit 1
fi

# Initialize associative arrays for storing structured data
declare -A OPTIONS
declare -a POSITIONAL_ARGS

# Default values
OPTIONS[verbose]="false"
OPTIONS[env]="development"

# Display help message
print_usage() {
    echo "Usage: $0 [options] [arguments]"
    echo ""
    echo "Options:"
    echo "  -h, --help           Show this help message"
    echo "  -v, --verbose        Enable verbose mode"
    echo "  -e, --env <value>    Specify environment (default: development)"
    echo "  --env=<value>        Alternative format for specifying environment"
}

# The Parser Loop
while (( "$#" )); do
    case "$1" in
        -h|--help)
            print_usage
            exit 0
            ;;
        -v|--verbose)
            OPTIONS[verbose]="true"
            shift
            ;;
        -e|--env)
            if [[ -n "$2" && "$2" != -* ]]; then
                OPTIONS[env]="$2"
                shift 2
            else
                echo "Error: Argument for $1 is missing." >&2
                exit 1
            fi
            ;;
        # Magic pattern: Splits --env=prod into key/value automatically
        --env=*)
            OPTIONS[env]="${1#*=}"
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

# ==============================================================================
# ACCESSING THE STORED DATA
# ==============================================================================

echo -e "\n\033[0;92m--- Parsed Options ---\033[0m"
echo "Verbose Status: ${OPTIONS[verbose]}"
echo "Environment:    ${OPTIONS[env]}"

echo -e "\n\033[0;92m--- Positional Arguments ---\033[0m"
if [ ${#POSITIONAL_ARGS[@]} -eq 0 ]; then
    echo "No positional arguments provided."
else
    # Loop through the traditional indexed array
    for i in "${!POSITIONAL_ARGS[@]}"; do
        echo "Arg [$i]: ${POSITIONAL_ARGS[$i]}"
    done
fi

