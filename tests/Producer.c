#include "../include/Database.h"

#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    Database local_db;

    setbuf(stdout,NULL);

    Database_INIT(&local_db);

    if(argc > 1){
        int Record_Position = atoi(argv[1]);
        if(Record_Position >= MAX_RECORDS_COUNT || Record_Position < 0) {
            perror("Index out of bouds");
            return 0;
        }
        
        printf("Data was %d\n", local_db.Records[Record_Position].Data);
        if (Change_Local_Record(&local_db, Record_Position, Make_New_Record(++local_db.Records[Record_Position].Data)) == ERROR) return ERROR;
        printf("Local record changed...\n");
        Write_Database_To_File(&local_db);
        printf("Data is %d\n", local_db.Records[Record_Position].Data);
    }
    else{
        printf("No parameter has been passed");
    }

    return 0;
}