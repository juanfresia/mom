#include <stdio.h>
#include "mom.h"

// Local broker deamon
int main(void) {
    printf("Starting client\n");
    int id = register_client();

    subscribe(id, "placeholder");
    publish(id, "placeholder", "Esto es un mensaje de prueba");

    printf("Shutting down client\n");
    return 0;
}
