#include "../include/Database.h"

#include <unistd.h>

int main(int argc, char* argv[]){
    Database local_db;
    
    if (Database_JOIN(&local_db) == -1){
        Database_INIT(&local_db);
    }
    
    Write_To_Database_With_Notify(&local_db);

    pthread_exit(NULL);
    return 0;
}