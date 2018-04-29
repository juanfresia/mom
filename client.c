#include <stdio.h>
#include "mom.h"

// Local broker deamon
int main(void) {
    printf("Starting client\n");
    int id = register_client();
    if (id < 0) {
        perror("client: register");
    }

    int r = subscribe(id, "placeholder");
    if (r < 0) {
        perror("client: register");
    }

    r = publish(id, "placeholder", "Esto es un mensaje de prueba");
    if (r < 0) {
        perror("client: register");
    }

    printf("Shutting down client\n");
    return 0;
}
