#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <string>

struct dir_entry {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[];
};

int copy_file(const char *src, const char *dest) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("open(src_file)");
        return -1;
    }

    struct stat st;
    int ret = fstat(src_fd, &st);
    if (ret == -1) {
        perror("fstat(src_file)");
        return -1;
    }

    // TODO same permission
    int dest_fd = open(dest, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (dest_fd == -1) {
        perror("open(dest_file)");
        return -1;
    }

    off_t size = st.st_size;
    do {
        ssize_t bytes = copy_file_range(src_fd, NULL, dest_fd, NULL, size, 0);
        if (bytes == -1) {
            perror("copy_from_range");
            return -1;
        }
        if (bytes == 0) {
            break;
        }

        size -= bytes;
    } while (size > 0);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s src_dir dest_dir\n", argv[0]);
        return 1;
    }

    // TODO validation
    std::string src_dir(argv[1]);
    std::string dest_dir(argv[2]);
    int fd = open(src_dir.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror("open(src_dir)");
        return 1;
    }

    struct stat st;
    int ret = stat(dest_dir.c_str(), &st);
    if (ret == -1) {
        if (errno != ENOENT) {
            perror("stat(dest_dir)");
            return 1;
        }

        // TODO recursive makedir
        ret = mkdir(dest_dir.c_str(), 0755);
        if (ret == -1) {
            perror("mkdir(dest_dir");
            return 1;
        }

        printf("@@ make directory: %s\n", dest_dir.c_str());
    }

    char buf[1024];
    while (1) {
        ssize_t bytes = syscall(SYS_getdents, fd, buf, sizeof(buf));
        if (bytes == -1) {
            perror("getdents");
            return 1;
        }
        if (bytes == 0) {
            break;
        }

        for (ssize_t pos = 0; pos < bytes;) {
            struct dir_entry *entry = (struct dir_entry *)(buf + pos);
            const std::string entry_name(entry->d_name);
            if (entry_name != "." && entry_name != "..") {
                // TODO copy directory

                // TODO check trailing slash
                std::string src_path(src_dir + "/" + entry_name);
                std::string dest_path(dest_dir + "/" + entry_name);

                printf("@@ Copy %s -> %s\n", src_path.c_str(), dest_path.c_str());
                copy_file(src_path.c_str(), dest_path.c_str());
            }
            pos += entry->d_reclen;
        }
    }

    return 0;
}