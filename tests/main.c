#include "../include/Database.h"

#include <unistd.h>

int main(int argc, char* argv[]){
    Database local_db;
    
    if (Database_JOIN(&local_db) == -1){
        if (Database_INIT(&local_db) == ERROR){
            perror("Database initialization failed");
        }
    }
    
    if(argc > 1){
        Record myNewRecord = local_db.Records[2];
        myNewRecord.Data++;
        if (Change_Local_Record(&local_db, 2, myNewRecord) == ERROR)
            perror("Local change was not applied");
    }
    
    Write_To_Database_With_Notify(&local_db);

    pthread_exit(NULL);
    return 0;
}