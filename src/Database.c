#include "../include/Database.h"

/*///////////////////////////////////////////////////////////////////////////////
                            VARIABLES:
///////////////////////////////////////////////////////////////////////////////*/

static pthread_mutex_t Database_Access_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *Database_Synchronization_Lock; 
pthread_cond_t *Database_Changed_Signal;

const char* SHARED_MUTEX_NAMETAG = "SHARED_MEMORY_MUTEX_DB";
const char* SHARED_CONDITION_NAMETAG = "SHARED_MEMORY_CONDITION_DB";
const char* REMOTE_DATABASE = "/tmp/Database.db";

Sync_State Changed_Records_Report;

/*///////////////////////////////////////////////////////////////////////////////
                            FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////*/

inline int Read_Database_From_File(FILE* Database_FD, Database* local_db){
    int Operation_State = ERROR;

    pthread_mutex_lock(&Database_Access_Lock);
    Database_FD = fopen(REMOTE_DATABASE,"rb");
    if(Database_FD != NULL){
        fread(local_db, sizeof(Database), 1, Database_FD);
        fclose(Database_FD);
        Operation_State = SUCCESS;
    }
    pthread_mutex_unlock(&Database_Access_Lock);

    return Operation_State;
}

inline int Write_Database_To_File(Database* local_db, FILE* Database_FD){
    int Operation_State = ERROR;

    pthread_mutex_lock(&Database_Access_Lock);
    Database_FD = fopen(REMOTE_DATABASE,"wb");
    if(Database_FD != NULL){
        fwrite(local_db, sizeof(Database), 1, Database_FD);
        fclose(Database_FD);
        Operation_State = SUCCESS;
    }
    pthread_mutex_unlock(&Database_Access_Lock);
    pthread_cond_broadcast(Database_Changed_Signal);

    return Operation_State;
}

int Change_Local_Record(Database* local_db, const __uint8_t recordID, const Record localRecord){
    if (recordID < MAX_RECORDS_COUNT){
        local_db->Records[recordID] = localRecord;
        return SUCCESS;
    }
    return ERROR;
}

int Database_INIT(Database* local_db){
    FILE* Database_FD = NULL;
    int Shared_Mutex_FD;
    int Shared_Condition_FD;

    //Initialize sharem memory fragments to be available for all processes.
    Shared_Mutex_FD = shm_open(SHARED_MUTEX_NAMETAG, O_CREAT|O_RDWR, 0666);
    if (Shared_Mutex_FD == -1) return ERROR;
    ftruncate(Shared_Mutex_FD,sizeof(pthread_mutex_t));
    Database_Synchronization_Lock = mmap(0, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED, Shared_Mutex_FD, 0);

    Shared_Condition_FD = shm_open(SHARED_CONDITION_NAMETAG, O_CREAT|O_RDWR, 0666);
    if (Shared_Condition_FD == -1) return ERROR;
    ftruncate(Shared_Condition_FD, sizeof(pthread_cond_t));
    Database_Changed_Signal = mmap(0, sizeof(pthread_cond_t), PROT_READ|PROT_WRITE, MAP_SHARED, Shared_Condition_FD, 0);

    //Initialize Database Synchronization related mutex and condition.
    pthread_mutexattr_t Sync_Mutex_Attr;
    pthread_mutexattr_init(&Sync_Mutex_Attr);
    pthread_mutexattr_setpshared(&Sync_Mutex_Attr,PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(Database_Synchronization_Lock, &Sync_Mutex_Attr);
    pthread_mutexattr_destroy(&Sync_Mutex_Attr);

    pthread_condattr_t Sync_Cond_Attr;
    pthread_condattr_init(&Sync_Cond_Attr);
    pthread_condattr_setpshared(&Sync_Cond_Attr,PTHREAD_PROCESS_SHARED);
    pthread_cond_init(Database_Changed_Signal, &Sync_Cond_Attr);
    pthread_condattr_destroy(&Sync_Cond_Attr);

    //Initialize the local datase before the lock in order to limit the delay of a main thread
    if (Read_Database_From_File(Database_FD,local_db) < 1){
        for (int i = 0; i<MAX_RECORDS_COUNT; ++i){
            local_db->Records[i].Data = 0;
        }
        Write_Database_To_File(local_db,Database_FD);
        return NEW_DATABASE;
    }
    return SUCCESS;
}

int Database_JOIN(Database* local_db){
    FILE* Database_FD = NULL;
    int Shared_Mutex_FD;
    int Shared_Condition_FD;

    Shared_Mutex_FD = shm_open(SHARED_MUTEX_NAMETAG, O_RDWR, 0666);
    if (Shared_Mutex_FD == -1) return ERROR;
    ftruncate(Shared_Mutex_FD,sizeof(pthread_mutex_t));
    Database_Synchronization_Lock = mmap(0, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED, Shared_Mutex_FD, 0);


    Shared_Condition_FD = shm_open(SHARED_CONDITION_NAMETAG, O_RDWR, 0666);
    if (Shared_Condition_FD == -1) return ERROR;
    ftruncate(Shared_Condition_FD, sizeof(pthread_cond_t));
    Database_Changed_Signal = mmap(0, sizeof(pthread_cond_t), PROT_READ|PROT_WRITE, MAP_SHARED, Shared_Condition_FD, 0);

    //Initialize the local datase before the lock in order to limit the delay of a main thread
    if (Read_Database_From_File(Database_FD,local_db) < 1){
        for (int i = 0; i<MAX_RECORDS_COUNT; ++i){
            local_db->Records[i].Data = 0;
        }
        Write_Database_To_File(local_db,Database_FD);
        return NEW_DATABASE;
    }
    return SUCCESS;
}

static int Compare_Records(Database* First, Database* Second){
    for(int i =0; i<MAX_RECORDS_COUNT; ++i){
        if (memcmp((void*)&First->Records[i], (void*)&Second->Records[i],sizeof(Record)) != 0)
            return RECORDS_DIFFERENT;
    }
    return RECORDS_SAME;
}

static int Remote_DB_Comparison_Check(Database* Database_input){
    FILE* Database_FD = NULL;
    Database temp;
    Read_Database_From_File(Database_FD, &temp);

    return Compare_Records(Database_input,&temp);
}

static void Synchronize_Local_DB(FILE* Database_FD, Database* local_db, Sync_State* Sync_Report){
    Database Compared_DB;
    Read_Database_From_File(Database_FD, &Compared_DB);

    for(int i =0; i<MAX_RECORDS_COUNT; ++i){
        if (memcmp((void*)&Compared_DB.Records[i], (void*)&local_db->Records[i], sizeof(Record)) != 0){
            memcpy((void*)&local_db->Records[i], (void*)&Compared_DB.Records[i], sizeof(Record));
            Sync_Report->ChangedRecord[i] = true;
        }
        else {
            Sync_Report->ChangedRecord[i] = false;
        }
    }
}

void* Synchronize_With_Remote_Task(void * Database_input){
    FILE* Database_FD = NULL;
    Database temp;
    while(1){
        Read_Database_From_File(Database_FD, &temp);

        pthread_mutex_lock(Database_Synchronization_Lock);
        while(Remote_DB_Comparison_Check((Database*)Database_input)){
            pthread_cond_wait(Database_Changed_Signal, Database_Synchronization_Lock);
        }
        Synchronize_Local_DB(Database_FD, (Database*)Database_input, &Changed_Records_Report);
        pthread_mutex_unlock(Database_Synchronization_Lock);
    }
}

void* Write_To_Database_With_Notify(void* Database_input){
    FILE* Database_FD = NULL;

    Write_Database_To_File((Database*)Database_input,Database_FD);
    return NULL;
}