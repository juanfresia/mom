#ifndef MOM_H
#define MOM_H

#define MOM_SUCCESS 0
#define MOM_ERROR -1

/*
 * Register as a client of the mq.
 * Returns an id to be used in the rest of the operations
 * or MOM_ERROR in case of error.
 */
int register_client();

/*
 * Subscribes a client to a topic.
 * Returns MOM_SUCCESS if the subscription was sucessful, MOM_ERROR
 * otherwise.
 */
int subscribe(int id, char* topic);

/*
 * Revokes a client subscription to a topic.
 * Returns MOM_SUCCESS if the unsubscription was sucessful, MOM_ERROR
 * otherwise.
 */
int unsubscribe(int id, char* topic);

/*
 * Publish message into topic's queue as client id.
 * MOM_SUCCESS is returned if publish was successful, otherwise
 * MOM_ERROR is returned.
 */
int publish(int id, char* topic, char* message);

/*
 * Attempts to read a message with specified topic for client with id
 * _id_ and store it in msg_store.
 * It will block until a message is received and return MOM_SUCCESS.
 * If an error occurs, MOM_ERROR is returned and msg_store is pointed to null.
 */
int retrieve(int id, char* topic, char** msg_store);

/*
 * Unregister client with id _id_ from the mq.
 * No read messages will be lost, and subscription will be rolled back.
 * It returns MOM_ERROR if an error occurs, MOM_SUCCESS otherwise.
 */
int unregister(int id);

#endif // MSG_H
