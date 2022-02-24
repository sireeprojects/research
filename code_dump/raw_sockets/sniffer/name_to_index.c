#include <stdio.h>
#include <net/if.h>

int main() {
    printf("virbr1: %d\n", if_nametoindex("tp0"));
    return 0;
}
