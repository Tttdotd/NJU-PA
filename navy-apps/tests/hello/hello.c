#include <unistd.h>
#include <stdio.h>

int main() {
    write(1, "Hello World!\n", 13);
    int i = 2;
    volatile int j = 0;
    for (int index = 0; index < 100000; index ++) {
        j ++;
        if (j == 10000) {
            printf("Hello World from Navy-apps for the %dth time!\n", i ++);
            j = 0;
        }
    }
    return 0;
}
