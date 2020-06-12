#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <stdio.h>

static int CreateServer(const char *socket_path) {
    int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    unlink(socket_path);
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, socket_path);

    int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret != 0) {
        perror("bind");
        return -1;
    }

    ret = listen(fd, 5);
    if (ret != 0) {
        perror("listen");
        return -1;
    }

    return fd;
}

static int SendFileDescriptor(int client_sock, int fd) {
    struct msghdr message;
    struct iovec iov[1];
    struct cmsghdr *control_message = NULL;
    char control_buf[CMSG_SPACE(sizeof(int))];
    char data[1];

    memset(&message, 0, sizeof(message));
    memset(control_buf, 0, CMSG_SPACE(sizeof(int)));

    data[0] = ' ';
    iov[0].iov_base = data;
    iov[0].iov_len = sizeof(data);

    message.msg_name = NULL;
    message.msg_namelen = 0;
    message.msg_iov = iov;
    message.msg_iovlen = 1;
    message.msg_controllen = CMSG_SPACE(sizeof(int));
    message.msg_control = control_buf;

    control_message = CMSG_FIRSTHDR(&message);
    control_message->cmsg_level = SOL_SOCKET;
    control_message->cmsg_type = SCM_RIGHTS;
    control_message->cmsg_len = CMSG_LEN(sizeof(int));

    *((int *)CMSG_DATA(control_message)) = fd;

    ssize_t ret = sendmsg(client_sock, &message, 0);
    if (ret < 0) {
        perror("sendmsg");
        return -1;
    }

    return 0;
}

int main() {
    int sock = CreateServer("/tmp/test_unix_socket");
    if (sock == -1) {
        return 1;
    }

    struct sockaddr_un addr;
    socklen_t size;
    char buf[16];
    int file = 0;
    while (1) {
        int client = accept(sock, (struct sockaddr *)&addr, &size);
        if (client < 0) {
            continue;
        }

        printf("Got request\n");

        sprintf(buf, "file%d.txt", file++);
        int fd = open(buf, O_TRUNC | O_CREAT | O_WRONLY, 0644);
        if (fd < 0) {
            continue;
        }

        printf("Send %s file descriptor\n", buf);
        SendFileDescriptor(client, fd);
    }

    return 0;
}
