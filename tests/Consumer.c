#include "../include/Database.h"

int main(int argc, char* argv[]){
    Database local_db;
    pthread_t Sync_Thread;

    setbuf(stdout, NULL);

    Database_INIT(&local_db);
    if (pthread_create(&Sync_Thread, NULL, Pthread_Synchronize_With_Remote, &local_db) != 0){
        perror("Thread creation failed");
        return ERROR;
    }

    while(1){
        sleep(3);
        for (int i = 0; i < MAX_RECORDS_COUNT ; ++i){
            printf("Record %d is now: %d\n", i, local_db.Records[i].Data);
        }
    }

    pthread_exit(NULL);
    return 0;
}