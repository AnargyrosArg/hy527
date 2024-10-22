#include <stdio.h>
#include <stdlib.h>
#include "thread.h"

int n;

int f(void *argp) {
        int *p = argp;

        while ((*p)--) {  n = n + 1; }
        return 0;
}

main(int argc, char *argv[]) {
        int i, m = 5;
        unsigned count = 10000;

        Thread_init();
        if (argc >= 2)
                m = atoi(argv[1]);
        for (i = 0; i < m; i++)
                Thread_new(f, &count, sizeof count);
        Thread_join(0);
        printf("%d == %d\n", n, m*count);
        Thread_exit(0);
        return 0;
}
