#include "BF.h"

typedef struct{
    int fileDesc;
    char attrType;
    char* attrName;
    int attrLength;
} HP_info;

typedef struct{
    int id;
    char name[15];
    char surname[25];
    char address[50];
}Record;

static int NumOfEntries = BLOCK_SIZE/sizeof(Record);

int HP_CreateFile (char* fileName, char attrType, char* attrName, int attrLength);
HP_info* HP_OpenFile (char* fileName);
int HP_CloseFile (HP_info* header_info);
int HP_InsertEntry (HP_info header_info, Record record);
int HP_DeleteEntry (HP_info header_info, void* value);
int HP_GetAllEntries (HP_info header_info, void* value); 