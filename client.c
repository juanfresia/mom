#include <stdio.h>
#include "mom.h"

// Local broker deamon
int main(void) {
    printf("Starting client\n");
    register_client();

    subscribe(2, "placeholder");

    printf("Shutting down client\n");
    return 0;
}
