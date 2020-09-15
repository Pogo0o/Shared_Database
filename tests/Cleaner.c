#include "../include/Database.h"

int main(int argc, char* argv[]){
    if (Database_CLOSE())
        printf("Database closing completed...\n");
    else
        printf("Database closing failed...\n");

    return 0;
}