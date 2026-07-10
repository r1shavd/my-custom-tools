# - - - * - - -
# .bashrc - custom
# - - - * - - -
#
# -> I made this for myself, feel free to customize as per your needs.
# -> I currently added cpu-usage and memory-usage as a custom functions.
# But, if you have any native utilities made specially for these tasks,
# feel free to use them as these bash functions are slower than native C
#
# - - - * - - -

# Source global definitions
if [ -f /etc/bashrc ]; then
    . /etc/bashrc
fi

# User specific environment
if ! [[ "$PATH" =~ "$HOME/.local/bin:$HOME/bin:" ]]; then
    PATH="$HOME/.local/bin:$HOME/bin:$PATH"
fi
export PATH

# Uncomment the following line if you don't like systemctl's auto-paging feature:
# export SYSTEMD_PAGER=

# User specific aliases and functions
if [ -d ~/.bashrc.d ]; then
    for rc in ~/.bashrc.d/*; do
        if [ -f "$rc" ]; then
            . "$rc"
        fi
    done
fi
unset rc

# User specific aliases and functions
export PS1="\033[0;92m\u\033[0m:\w\$ "

# User defined alias
alias ls="ll -h"
alias mkdir="mkdir -p"
alias df="df -h"
alias free="free -m"
alias rm="rm -i"

# User defined functions
# - - - 
function cpu-usage {
	# This function displays the CPU usage in a pretty shorted manner
	
	top -bn1 | grep "Cpu(s)" | awk '{print "\033[0;97mCPU Usage: \033[0;93m" 100 - $8 "% \033[0m"}'
}

function memory-usage {
    # This function displays the memory usage in a pretty manner, more
	# like a HTOP version. Coded with ANSII color codes.
	#
	# NOTE: I also made a similiar utility in C lang, which is extremely
	# faster than this shit.
	#
	
	local key val mem_total mem_avail swap_total swap_free
    local mem_used swap_used ram_pct swap_pct BAR_WIDTH
    local ram_fill swap_fill ram_bar swap_bar
    local ram_used_gb ram_total_gb swap_used_gb swap_total_gb

	# Reading memory statistics directly from the system kernel
    while IFS=':' read -r key val; do
        val=$(echo "$val" | awk '{print $1}')
        case "$key" in
            MemTotal)      mem_total=$val ;;
            MemAvailable)  mem_avail=$val ;;
            SwapTotal)     swap_total=$val ;;
            SwapFree)      swap_free=$val ;;
        esac
    done < /proc/meminfo

    # Calculating used metrics (in kilobytes)
    mem_used=$((mem_total - mem_avail))
    swap_used=$((swap_total - swap_free))

    # Checking whether DivisonByZeroError occurs or not
	[[ -z "$mem_total" || "$mem_total" -eq 0 ]] && mem_total=1
    [[ -z "$swap_total" || "$swap_total" -eq 0 ]] && swap_total=1

    ram_pct=$(( mem_used * 100 / mem_total ))
    swap_pct=$(( swap_used * 100 / swap_total ))

    # Defining the visual bar width and other properties
    BAR_WIDTH=30
    ram_fill=$(( ram_pct * BAR_WIDTH / 100 ))
    swap_fill=$(( swap_pct * BAR_WIDTH / 100 ))
    ram_bar=$(printf "%-${BAR_WIDTH}s" "$(printf '#%.0s' $(seq 1 $ram_fill 2>/dev/null))" | tr ' ' '-')
    swap_bar=$(printf "%-${BAR_WIDTH}s" "$(printf '#%.0s' $(seq 1 $swap_fill 2>/dev/null))" | tr ' ' '-')

    # Converting raw kilobytes to Gigabytes for clean text presentation
    ram_used_gb=$(awk "BEGIN {printf \"%.2f\", $mem_used/1024/1024}")
    ram_total_gb=$(awk "BEGIN {printf \"%.2f\", $mem_total/1024/1024}")
    swap_used_gb=$(awk "BEGIN {printf \"%.2f\", $swap_used/1024/1024}")
    swap_total_gb=$(awk "BEGIN {printf \"%.2f\", $swap_total/1024/1024}")

    printf "  \e[1;36mRAM\e[0m  [\e[1;32m%s\e[0m] %3d%% (%sGB/%sGB)\n" "$ram_bar" "$ram_pct" "$ram_used_gb" "$ram_total_gb"
    printf "  \e[1;35mSwp\e[0m  [\e[1;32m%s\e[0m] %3d%% (%sGB/%sGB)\n" "$swap_bar" "$swap_pct" "$swap_used_gb" "$swap_total_gb"
}
# - - -
