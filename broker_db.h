#ifndef BROKER_DB_H
#define BROKER_DB_H
/*
 * Broker database interface
 */

 int db_init();

long db_next_id();

 int db_subscribe(long id, char *topic);

 int db_unsubscribe(long id, char *topic);

 int db_get_subscriptors(char *topic, long **id_list);

 int db_store_message(char *topic, char *message, long publisher);

 int db_close();

 #endif // BROKER_DB_H
