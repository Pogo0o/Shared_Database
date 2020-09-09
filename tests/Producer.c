#include "../include/Database.h"

#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    Database local_db;
    pthread_t Sync_Thread;
    int Record_Position = atoi(argv[1]);

    setbuf(stdout,NULL);

    Database_INIT(&local_db);
    if (pthread_create(&Sync_Thread, NULL, Pthread_Synchronize_With_Remote, &local_db) != 0){
        perror("Thread creation failed");
        return ERROR;
    }

    if(argc > 1){
        printf("Data was %d\n", local_db.Records[Record_Position].Data);
        if (Change_Local_Record(&local_db, 2, Make_New_Record(++local_db.Records[Record_Position].Data)) == ERROR)
        {
            perror("Index out of bouds");
            return 0;
        }

        Write_Database_To_File(&local_db);
        printf("Data is %d\n", local_db.Records[atoi(argv[1])].Data);
    }

    return 0;
}