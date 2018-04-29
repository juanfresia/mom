/*
 * Broker database implementation through bash commands.
 */

 #include "broker_db.h"

#define UNUSED(x) (void)(x)

int db_init() {
    return 0;
}

int db_subscribe(long id, char *topic) {
    UNUSED(id);
    UNUSED(topic);
    return 0;
}

int db_unsubscribe(long id, char *topic) {
    UNUSED(id);
    UNUSED(topic);
    return 0;
}

int db_get_subscriptors(char *topic, long **id_list) {
    UNUSED(id_list);
    UNUSED(topic);
    return 0;
}

int db_store_message(char *topic, char *message, long publisher) {
    UNUSED(topic);
    UNUSED(message);
    UNUSED(publisher);
    return 0;
}

int db_close() {
    return 0;
}
