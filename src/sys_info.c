#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

#include "colors.h"

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

int getGPUVendor() {
	const char * gpu_path = "/sys/class/drm/card0/device/vendor";
    FILE * file = fopen(gpu_path, "r");

    if (file == NULL) {
		perror("Could not read GPU data. Ensure you are on native Linux.");
		return 1;
	}

	char vendor_hex[16];
	if (fgets(vendor_hex, sizeof(vendor_hex), file) != NULL) {
    	// The kernel outputs a hex code (e.g., 0x10de for NVIDIA, 0x8086 for Intel)
		printf("%sGPU Vendor PCI ID%s: %s", WHITE, vendor_hex, DEFAULT);
  	}
  	fclose(file);
  	return 0;
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

int main() {
    getOSInfo();
	getUptime();
	getGPUVendor();
    return 0;
}
