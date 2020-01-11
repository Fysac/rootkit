#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void *race(void *args) {
    struct timespec ts;
    ts.tv_sec = 50 / 1000;
    ts.tv_nsec = (50 % 1000) * 1000000;

    int fd = open("/dev/ttyO5", O_RDWR);

    unsigned long i = 0;

    while (1) {
        if (i % 3 == 0) {
            write(fd, "bye", 3);
        } else if (i % 3 == 1) {
            write(fd, "hi", 2);
        } else {
            write(fd, "w00tw00ticanhazr00t?", 20);
        }
        i++;
    }

    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t t1, t2, t3, t4;
    pthread_create(&t1, NULL, race, NULL);
    // pthread_create(&t2, NULL, race, NULL);
    // pthread_create(&t3, NULL, race, NULL);
    // pthread_create(&t4, NULL, race, NULL);

    pthread_join(t1, NULL);
    // pthread_join(t2, NULL);
    // pthread_join(t3, NULL);
    // pthread_join(t4, NULL);

    return 0;
}
