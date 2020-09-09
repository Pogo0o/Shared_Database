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

static int Initialize_Shared_Memory();
static int Initialize_Thread_Synchronization();
static int Initialize_Remote_Database(Database*);
static void* Pthread_Synchronize_With_Remote(void *);

static inline int Compare_Records(Database*, Database*);
static inline int Remote_DB_Comparison_Check(Database*);
static void Synchronize_Local_DB(FILE*, Database*, Sync_State*);

int Database_INIT(Database* local_db, pthread_t* Synchronization_Thread, pthread_attr_t* Synchronization_Attributes){
    
    if (Initialize_Shared_Memory() == ERROR){
        perror("Shared memory initialization failed");
        return ERROR;
    }
    
    if (Initialize_Thread_Synchronization() == ERROR){
        perror("Thread synchronization initialization failed");
        return ERROR;
    }

    if (Initialize_Remote_Database(local_db) == ERROR){
        perror("Remote Database initialization failed");
        return ERROR;
    }

    if (pthread_create(Synchronization_Thread,Synchronization_Attributes,Pthread_Synchronize_With_Remote,local_db) != 0){
        perror("Synchronization thread initialization failed");
        return ERROR;
    }
    return SUCCESS;
}

static int Initialize_Shared_Memory(){
    int Shared_Mutex_FD;
    int Shared_Condition_FD;

    Shared_Mutex_FD = shm_open(SHARED_MUTEX_NAMETAG, O_RDWR, 0666);
    if (Shared_Mutex_FD == -1) Shared_Mutex_FD = shm_open(SHARED_MUTEX_NAMETAG, O_CREAT|O_RDWR, 0666);
    if (Shared_Mutex_FD == -1) return ERROR;
    ftruncate(Shared_Mutex_FD,sizeof(pthread_mutex_t));
    Database_Synchronization_Lock = mmap(0, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED, Shared_Mutex_FD, 0);


    Shared_Condition_FD = shm_open(SHARED_CONDITION_NAMETAG, O_RDWR, 0666);
    if (Shared_Condition_FD == -1) Shared_Condition_FD = shm_open(SHARED_CONDITION_NAMETAG, O_CREAT|O_RDWR, 0666);
    if (Shared_Condition_FD == -1) return ERROR;
    ftruncate(Shared_Condition_FD, sizeof(pthread_cond_t));
    Database_Changed_Signal = mmap(0, sizeof(pthread_cond_t), PROT_READ|PROT_WRITE, MAP_SHARED, Shared_Condition_FD, 0);
    return SUCCESS;
}

static int Initialize_Thread_Synchronization(){
    pthread_mutexattr_t Sync_Mutex_Attr;
    pthread_condattr_t Sync_Cond_Attr;

    //Initialize a shared memory mutex
    if (pthread_mutexattr_init(&Sync_Mutex_Attr) != 0) return ERROR;
    pthread_mutexattr_setpshared(&Sync_Mutex_Attr,PTHREAD_PROCESS_SHARED);
    if (pthread_mutex_init(Database_Synchronization_Lock, &Sync_Mutex_Attr) != 0) return ERROR;
    pthread_mutexattr_destroy(&Sync_Mutex_Attr);

    //Initialize a shared memory condition variable
    if (pthread_condattr_init(&Sync_Cond_Attr) != 0) return ERROR;
    pthread_condattr_setpshared(&Sync_Cond_Attr,PTHREAD_PROCESS_SHARED);
    if (pthread_cond_init(Database_Changed_Signal, &Sync_Cond_Attr) != 0) return ERROR;
    pthread_condattr_destroy(&Sync_Cond_Attr);

    return SUCCESS;
}

static int Initialize_Remote_Database(Database* local_db){
    if (Read_Database_From_File(local_db) == ERROR){
        for (int i = 0; i<MAX_RECORDS_COUNT; ++i){
            local_db->Records[i].Data = 0;
        }
        Write_Database_To_File(local_db);
    }
    return SUCCESS;
}

int Read_Database_From_File(Database* local_db){
    FILE* Database_FD;
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

int Write_Database_To_File(Database* local_db){
    FILE* Database_FD;
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

static void *Pthread_Synchronize_With_Remote(void * Database_input){
    FILE* Database_FD = NULL;
    Database Remote_Database_Comparison_Copy;
    while(1){
        Read_Database_From_File(&Remote_Database_Comparison_Copy);

        pthread_mutex_lock(Database_Synchronization_Lock);
        while(Remote_DB_Comparison_Check((Database*)Database_input)){
            pthread_cond_wait(Database_Changed_Signal, Database_Synchronization_Lock);
        }
        Synchronize_Local_DB(Database_FD, (Database*)Database_input, &Changed_Records_Report);
        pthread_mutex_unlock(Database_Synchronization_Lock);
    }
    return NULL;
}
static inline int Compare_Records(Database* First, Database* Second){
    for(int i =0; i<MAX_RECORDS_COUNT; ++i){
        if (memcmp((void*)&First->Records[i], (void*)&Second->Records[i],sizeof(Record)) != 0)
            return RECORDS_DIFFERENT;
    }
    return RECORDS_SAME;
}

static inline int Remote_DB_Comparison_Check(Database* Database_input){
    Database Remote_Database_Comparison_Copy;
    Read_Database_From_File(&Remote_Database_Comparison_Copy);

    return Compare_Records(Database_input,&Remote_Database_Comparison_Copy);
}

static void Synchronize_Local_DB(FILE* Database_FD, Database* local_db, Sync_State* Sync_Report){
    Database Compared_DB;
    Read_Database_From_File(&Compared_DB);

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