#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

// Custom headers
#include "colors.h"

#define BUF_SIZE 1024

struct linux_dirent64 {
    unsigned long long d_ino;
    long long          d_off;
    unsigned short     d_reclen;
    unsigned char      d_type;
    char               d_name[];
};

int getConnectedUserSessions() {
    int fd = open("/dev/pts", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd == -1) return 1;

    char buf[BUF_SIZE];
    int pts_count = 0;

    long nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
    close(fd);

    if (nread <= 0) return 1;

    for (long bpos = 0; bpos < nread; ) {
        struct linux_dirent64 *d = (struct linux_dirent64 *) (buf + bpos);
        
        if (d->d_type == 2 && d->d_name[0] >= '0' && d->d_name[0] <= '9') {
            pts_count++;
        }
        
        bpos += d->d_reclen;
    }

    printf("%sTotal Live Hardware Terminal Connections%s: %s%d%s\n", WHITE, DEFAULT, GREEN, pts_count, DEFAULT);
    return 0;
}

int main(int argc, char** argv[]) {
	getConnectedUserSessions();
	return 0;
}
