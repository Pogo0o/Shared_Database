#include "../include/Database.h"

#include <unistd.h>

int main(int argc, char* argv[]){

    Database local_db;
    pthread_t Sync_Thread;

    Database_INIT(&local_db, &Sync_Thread, NULL);

    if (argc > 1){
        local_db.Records['0' - *argv[1]].Data++;
        Write_Database_To_File(&local_db);
    }

    return 0;
}