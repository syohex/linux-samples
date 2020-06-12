#include <stdio.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int ConnectServer(const char *socket_path) {
    int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, socket_path);

    int ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret != 0) {
        perror("connect");
        return -1;
    }

    return fd;
}

static int ReceiveFileDescriptor(int sock) {
    struct msghdr message;
    struct iovec iov[1];
    char control_buf[CMSG_SPACE(sizeof(int))];
    char data[1];

    memset(&message, 0, sizeof(message));
    memset(control_buf, 0, CMSG_SPACE(sizeof(int)));

    iov[0].iov_base = data;
    iov[0].iov_len = sizeof(data);

    message.msg_name = NULL;
    message.msg_namelen = 0;
    message.msg_control = control_buf;
    message.msg_controllen = CMSG_SPACE(sizeof(int));
    message.msg_iov = iov;
    message.msg_iovlen = 1;

    ssize_t res = recvmsg(sock, &message, 0);
    if (res < 0) {
        perror("rcvmsg");
        return -1;
    }

    struct cmsghdr *control_message;
    for (control_message = CMSG_FIRSTHDR(&message); control_message != NULL;
         control_message = CMSG_NXTHDR(&message, control_message)) {
        if (control_message->cmsg_level == SOL_SOCKET && control_message->cmsg_type == SCM_RIGHTS) {
            return *((int *)CMSG_DATA(control_message));
        }
    }

    return -1;
}

int main(void) {
    int sock = ConnectServer("/tmp/test_unix_socket");
    if (sock < 0) {
        return 1;
    }

    int file_descriptor = ReceiveFileDescriptor(sock);
    if (file_descriptor < 0) {
        return 1;
    }

    char buf[] = "hello world\n";
    if (write(file_descriptor, buf, sizeof(buf) - 1) < 0) {
        perror("write");
    }
    close(file_descriptor);
    return 0;
}
