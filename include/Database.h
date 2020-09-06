#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAX_RECORDS_COUNT 16
#define SUCCESS 1
#define ERROR -1
#define NEW_DATABASE 0

#define RECORDS_DIFFERENT 0
#define RECORDS_SAME 1

typedef struct {
        int Data;
} Record;

typedef struct {
    Record Records[MAX_RECORDS_COUNT];
} Database;

typedef struct {
    bool ChangedRecord[MAX_RECORDS_COUNT];
} Sync_State;

extern Sync_State Changed_Records_Report;

extern pthread_mutex_t *Database_Synchronization_Lock;
extern pthread_cond_t *Database_Changed_Signal;

extern const char* SHARED_MUTEX_NAMETAG;
extern const char* SHARED_CONDITION_NAMETAG;
extern const char* REMOTE_DATABASE;

/*
*   Main functionality of a Database:
*   - Update the local database with remote file contents
*   - Write the local database to a remote file
*   - Change a single record in the local database
*/

int Read_Database_From_File(FILE*, Database*);
int Write_Database_To_File(Database*, FILE*);
int Change_Local_Record(Database*, const __uint8_t, const Record);

/*
*   Database setup - crucial for the .so file to work properly.
*   
*   Example:
*       if (Database_JOIN(local_db_ptr) == -1){
*            Database_INIT(local_db_ptr)
*       }
*/
int Database_INIT(Database*);
int Database_JOIN(Database*);

/*
*   These functions are used for the synchronization mechanism,
*   Dedicate a single thread that is going to be woken up each time a remote database gets changes
*   
*   Example:
*       pthread_create(Synchronization_Thread, NULL, Synchronize_With_Remote_Task, local_DB_ptr);
*/

void* Synchronize_With_Remote_Task(void *);
void* Write_To_Database_With_Notify(void*);

#endif