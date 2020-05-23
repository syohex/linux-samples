#include <stdio.h>
#include <stdint.h>
#include <sys/random.h>

int main(void) {
    uint8_t buf[32];

    const unsigned int flags = 0;
    ssize_t ret = getrandom(buf, sizeof(buf), flags);
    if (ret == -1) {
        perror("getrandom");
        return 1;
    }

    for (ssize_t i = 0; i < ret; ++i) {
        printf("%02x", buf[i]);
    }
    puts("");

    return 0;
}