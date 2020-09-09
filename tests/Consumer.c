#include "../include/Database.h"

int main(int argc, char* argv[]){
    Database local_db;
    pthread_t Sync_Thread;

    Database_INIT(&local_db, &Sync_Thread, NULL);

    while(1){
        sleep(3);
        for (int i = 0; i < MAX_RECORDS_COUNT ; ++i){
            printf("Record %d is now: %d\n", i, local_db.Records[i].Data);
        }
    }

    return 0;
}