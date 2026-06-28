#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <string.h>
#include <poll.h>

// Custom modules
#include "colors.h"

// Defining custom data structures
// - - -
typedef struct VendorMap {
    // Map - Dictionary like data structure stores in key:value pairs
    // Specialized for Vendor PCI id in hex form

    const char* key;
    const char* value;
}

struct CpuStats {
    // To store CPU time states - native mode
    
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};
// - - -

// Stats reading functions (CPU, Memory,)
// - - -

void print_statsbar(const char *LABEL_STATS, const char *LABEL_STATS_COLOR, long USED_STATS, long TOTAL_STATS, int PCT_STATS, int STATS_BAR_WIDTH) {
    /*
    This function prints a progress / status bar to the screen as per the arguments passed to the function.

    LABEL_STATS --> item for which we are monitoring
    LABEL_STATS_COLOR --> reads from "colors.h" (defaults = DEFAULT)
    USED_STATS --> (defaults: value in KBs)
    TOTAL --> (defaults: value in KBs)
    PCT --> percentage of the particular analysis
    STATS_BAR_WIDTH --> width of the status bar (defaults: 30)
    */

    if (LABEL_STATS_COLOR == NULL || strlen(LABEL_STATS_COLOR) == 0) {
        // If the LABEL_STATS COLOR hasnt been provided, then we assign default ANSII color

        LABEL_STATS_COLOR = DEFAULT;
    }
    if (STATS_BAR_WIDTH == NULL || STATS_BAR_WIDTH == 0) {
        // Setting a default STATS_BAR_WIDTH to 30 in case of any issue

        STATS_BAR_WIDTH = 30;
    }

    char bar[STATS_BAR_WIDTH + 1];
    int fill = (PCT_STATS * STATS_BAR_WIDTH) / 100;

    // Build the htop-style progress bar string
    for (int i = 0; i < STATS_BAR_WIDTH; i++) {
        bar[i] = (i < fill) ? '#' : '-';
    }
    bar[STATS_BAR_WIDTH] = '\0';

    // Convert KB to GB natively using floating-point math
    double USED_STATS_gb = (double)USED_STATS / (1024.0 * 1024.0);
    double TOTAL_STATS_gb = (double)TOTAL_STATS / (1024.0 * 1024.0);

    // Print matching your exact terminal ANSI styling
    printf("%-2s%s%s\033[0m [\033[1;32m%s\033[0m] %3d%% (%.2fGB/%.2fGB)\n", 
           "", LABEL_STATS_COLOR, LABEL_STATS, bar, PCT_STATS, USED_STATS_gb, TOTAL_STATS_gb);
}

int readCpuProc(struct CpuStats *stats) {
    /*
    This function reads the CPU stats from /proc/stat as in ticks.
    This function reads the instantaneous CPU output, thus it may
    result in reading zero on idle modes for an instant.
    Call twice to get values.

    Requires pointer to location of the CpuStats structure
    */

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp)        return 0;


    // Reading line-by-line grepping "cpu" metrics
    char label[32];
    int SUCCESS = 0;
    while (fgets(label, sizeof(label), fp) != NULL) {
        if (strncmp(label, "cpu ", 4) == 0) {
            // Return fetching the entire line if "cpu" metric is found

            fseek(fp, 0, SEEK_SET);
            int read = fscanf(
                fp,
                "%31s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                label,
                &stats->user,
                &stats->nice,
                &stats->system,
                &stats->idle,
                &stats->iowait,
                &stats->irq,
                &stats->softirq,
                &stats->steal,
                &stats->guest,
                &stats->guest_nice
            );
            if (read == 11)    SUCCESS = 1;
            break;
        }
    }
    fclose(fp);
        return SUCCESS;
}

int cpuUsage() {
    struct CpuStats s1, s2;

    // Reading the CPU stats from /proc
    if (!readCpuProc(&s1)) {
        perror("Error reading initial CPU stats");
        return 1;
    }

    // 100ms delay window to profile work ticks change rate
    poll(NULL, 0, 100);

    if (!readCpuProc(&s2)) {
        perror("Error reading secondary CPU stats");
        return 1;
    }

    // Calculate total idle time differences
    unsigned long long s1_idle_all = s1.idle + s1.iowait;
    unsigned long long s2_idle_all = s2.idle + s2.iowait;

    // Calculate active processing time differences
    unsigned long long s1_non_idle = s1.user + s1.nice + s1.system + s1.irq + s1.softirq + s1.steal;
    unsigned long long s2_non_idle = s2.user + s2.nice + s2.system + s2.irq + s2.softirq + s2.steal;

    unsigned long long total1 = s1_idle_all + s1_non_idle;
    unsigned long long total2 = s2_idle_all + s2_non_idle;

    unsigned long long total_diff = total2 - total1;
    unsigned long long idle_diff = s2_idle_all - s1_idle_all;

    // Checking if DivisionByZeroError
        if (total_diff == 0) total_diff = 1;

    // Calculating the CPU percentage
    double cpu_usage = ((double)(total_diff - idle_diff) / total_diff) * 100.0;
    printf("%-2s%sCPU Usage %s[%s%.1f%%%s]\n", "", WHITE, DEFAULT, YELLOW, cpu_usage, DEFAULT);
    return 0;
}

int memoryUsage() {
    /*
    This function reads the memory usage details from /proc/meminfo
    The virtual filesystem which reads all the stored information when
    stored due to the operation of "initramfs"

    This will be significantly faster than top or free -h.
    */

    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Error reading memory vfs");
        return 1;
    }

    long MEM_TOTAL_STATS = 0, MEM_AVAILABLE = 0, SWAP_TOTAL_STATS = 0, SWAP_FREE = 0;
    char key[64];
    long val;

    // Read the file natively line-by-line (Replaces the slow Bash while loop)
    while (fscanf(fp, "%63s %ld kB", key, &val) != EOF) {
        if (strcmp(key, "MemTotal:") == 0)       MEM_TOTAL_STATS = val;
        else if (strcmp(key, "MemAvailable:") == 0) MEM_AVAILABLE = val;
        else if (strcmp(key, "SwapTotal:") == 0)    SWAP_TOTAL_STATS = val;
        else if (strcmp(key, "SwapFree:") == 0)     SWAP_FREE = val;
    }
    fclose(fp);

    // Calculate USED_STATS metrics
    long MEM_USED_STATS = MEM_TOTAL_STATS - MEM_AVAILABLE;
    long SWAP_USED_STATS = SWAP_TOTAL_STATS - SWAP_FREE;

    // Division by zero safety guards
    long SAFE_MEM_TOTAL_STATS = (MEM_TOTAL_STATS <= 0) ? 1 : MEM_TOTAL_STATS;
    long SAFE_SWAP_TOTAL_STATS = (SWAP_TOTAL_STATS <= 0) ? 1 : SWAP_TOTAL_STATS;

    // Calculate percentages
    int RAM_PCT_STATS = (int)(MEM_USED_STATS * 100 / SAFE_MEM_TOTAL_STATS);
    int SWAP_PCT_STATS = (int)(SWAP_USED_STATS * 100 / SAFE_SWAP_TOTAL_STATS);

    // Render outputs
    print_statsbar("RAM", CYAN, MEM_USED_STATS, SAFE_MEM_TOTAL_STATS, RAM_PCT_STATS, 50);
    print_statsbar("Swp", BLUE, SWAP_USED_STATS, SAFE_SWAP_TOTAL_STATS, SWAP_PCT_STATS, 50);
    return 0;
}
// - - -

// Basic data fetch functions
// - - - 
void getUptime() {
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        double uptime_seconds;
        fscanf(fp, "%lf", &uptime_seconds);
        fclose(fp);
        int hours = uptime_seconds / 3600;
        int minutes = (int)(uptime_seconds / 60) % 60;
        printf("%sUptime%s:  %d hours, %d mins\n", WHITE, DEFAULT, hours, minutes);
    }
}

void getGPUVendor() {
    /*
    This function gets the GPU details from the native properties
    filesystem i.e., /sys/class/drm/card0/device/vendor
    /sys/class stores hardwares by their functional subsystem. like
    this one file stores the GPU hardware.

    It fetches the hex code stored there, and then maps according to
    the vendor.
    */
    
	const char * gpu_path = "/sys/class/drm/card0/device/vendor";
    FILE * file = fopen(gpu_path, "r");

    if (file == NULL) {
		perror("Could not read GPU data. Ensure you are on native Linux.");
        return;
	}

	char vendor_hex[16];
	if (fgets(vendor_hex, sizeof(vendor_hex), file) != NULL) {
    	// The kernel outputs a hex code (e.g., 0x10de for NVIDIA, 0x8086 for Intel)
		printf("%sGPU Vendor PCI ID%s: %s\n", WHITE, DEFAULT, vendor_hex);
  	}
  	fclose(file);
}

void getOSInfo() {
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        perror("uname");
        return;
    }
    printf("%sOS%s:      %s%s%s\n", WHITE, DEFAULT, YELLOW, buffer.sysname, DEFAULT);
    printf("%sKernel%s:  %s%s%s\n", WHITE, DEFAULT, CYAN, buffer.release, DEFAULT);
}
// - - - 

int main() {
    getOSInfo();
	getUptime();
	getGPUVendor();

    memoryUsage();
    cpuUsage();
    return 0;
}
