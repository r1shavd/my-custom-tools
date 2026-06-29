#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

volatile int keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
}

void* burn_cpu(void* arg) {
    long thread_id = (long)arg;
    printf("[+] Thread %ld started on a CPU core.\n", thread_id);
    
    while (keep_running) {
        volatile unsigned long long x = 1000ULL;
        x = x * x; 
    }
    
    printf("[-] Thread %ld stopping.\n", thread_id);
    return NULL;
}

int main() {
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("[*] Detected %ld online CPU cores. Starting stress test...\n", num_cores);
    printf("[*] Press Ctrl+C at any time to stop the test and cool down.\n\n");

    signal(SIGINT, handle_sigint);

    pthread_t* threads = malloc(num_cores * sizeof(pthread_t));
    
    for (long i = 0; i < num_cores; i++) {
        if (pthread_create(&threads[i], NULL, burn_cpu, (void*)i) != 0) {
            perror("[-] Failed to create thread");
            return 1;
        }
    }

    while (keep_running) {
        sleep(1);
    }

    printf("\n[*] Stopping all threads. Cooling down...\n");
    
    for (long i = 0; i < num_cores; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    printf("[+] System idling. Done.\n");
    return 0;
}
