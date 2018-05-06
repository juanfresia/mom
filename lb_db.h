/*
 * Local broker database interface.
 * It manages local clients and maps its global_id to its local_id.
 * There are 5 basic functions:
 *  - db_init()
 *  - db_set_local_id(int local_id, long global_id)
 *  - db_get_local_id(long global_id)
 *  - db_get_global_id(int local_id)
 *  - db_close()
 */

#ifndef LB_DB_H_
#define LB_DB_H_

/*
 * Perform any initialization the database may need.
 */
int db_init();

/*
 * Maps a local_id to a global_id
 */
int db_set_local_id(long local_id, long global_id);

/*
 * Retrieves the local_id of a global_id
 */
long db_get_local_id(long global_id);

/*
 * Retrieves the global_id of a local_id
 */
long db_get_global_id(long local_id);

/*
 * Closes the database and perform any cleaning.
 */
int db_close();

#endif // LB_DB_H_
