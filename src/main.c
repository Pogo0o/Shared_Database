#include "../include/Database.h"

int main(int argc, char* argv[]){
    Database local_db;
    pthread_t Sync_Thread;

    if (Database_JOIN(&local_db) == -1){
        Database_INIT(&local_db);
    }

    pthread_create(&Sync_Thread, NULL, Synchronize_With_Remote_Task, &local_db);

    for(;;){
        sleep(1);
    }

    pthread_exit(NULL);
    return 0;
}