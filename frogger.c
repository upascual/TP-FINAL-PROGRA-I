#include <stdio.h>

int main(void) {
    puts("Frogger");

    #ifdef RASPBERRY
    puts("Raspberry!");
    #else
    puts("Allegro!");
    #endif

    return 0;
}
