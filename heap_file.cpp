
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <bits/stdc++.h>
#include <iostream>

using namespace std;

#include "BF.h"
#include "heap_file.h"

#define LENGTH 25


int HP_CreateFile (char* fileName, char attrType, char* attrName, int attrLength) {

    int fileDesc;    //o arithmos anoigmatos block 
    void* block;     //orizw ayton ton deikth 

    if (BF_CreateFile(fileName) != 0) { 
        BF_PrintError("ERROR: cannot create file");
         return-1; 
    }    //se periptwsh poy den mporei na anoiksei to arxeio 

    BF_Init();

    fileDesc = BF_OpenFile(fileName);
    if ( fileDesc < 0 ) { BF_PrintError("ERROR: cannot open file ");
        return -1; }

    if (BF_AllocateBlock(fileDesc) != 0) {
        BF_PrintError("ERROR: cannot allocate block");
        return -1;   
    }

    if (BF_ReadBlock(fileDesc, 0, &block) != 0) { BF_PrintError("ERROR: cannot read block ");
        return -1; }     //0 einai ekei poy yparxoyn oi plhrofories

    memcpy(block, &fileDesc, sizeof(int));
    block = (void*)((int*)block + sizeof(int));

    memcpy(block, &attrType, sizeof(char));
    block = (void*)((char*)block + sizeof(char));

    memcpy(block, attrName, sizeof(char)*LENGTH);
    block = (void*)((char*)block + sizeof(char)*LENGTH);

    memcpy(block, &attrLength, sizeof(int));

    if (BF_WriteBlock(fileDesc, 0) != 0) {
        BF_PrintError("Error: Cant write block");
        return -1;
    }

    return 0;
}

HP_info* HP_OpenFile (char* fileName) {

    int fileDesc;    //gia to arxeio 
    HP_info *HP = new HP_info;  
    void* block;

    if ( (fileDesc = BF_OpenFile(fileName)) < 0 ) { BF_PrintError("ERROR: cannot open file");
        return NULL; }      //arxika gia na exw prosvasi sto arxeio 


    if(BF_ReadBlock(fileDesc, 0, &block) != 0) { BF_PrintError("ERROR: cannot read block");
        return NULL; }

    HP->fileDesc = fileDesc;
    block = (void*)((int*)block + sizeof(int));

    memcpy(&HP->attrType, block, sizeof(char));
    block = (void*)((char*)block + sizeof(char));

    memcpy(&HP->attrName, block, sizeof(char)*LENGTH);
    block = (void*)((char*)block + sizeof(char)*LENGTH);

    memcpy(&HP->attrLength, block, sizeof(long int));

    return HP;   //epistrefw th domh plhroforiwn 
}


//---------------------------------------------------------------------------------------------------------//
//---------------------------------------HP_CloseFile----------------------------------------------------//

int HP_CloseFile (HP_info* header_info) {
    
    /*if (BF_CloseFile(header_info->fileDesc) != 0) {
        BF_PrintError("Error: Cannot close file");
        return -1;
    } */

    delete header_info; 

    cout << "File has closed" << endl;

    return 0;
}

//--------------------------------------------------------------------------------------------------------//
//------------------------------------HP_InsertEntry-----------------------------------------------------//

int HP_InsertEntry (HP_info header_info, Record record) {

    //we assume that the file is already open
    int NumOfBlocks, blockId, i;
    void* block;
    int x;

    NumOfBlocks = BF_GetBlockCounter(header_info.fileDesc);
    if ( NumOfBlocks < 0 ) { BF_PrintError("ERROR: cannot get BlockCounter");
        return -1;}
     //vlepw posa blocks exw , gia na exw prosvash 

    //psaxnoume gia ena block to opoio exei arketh mnhmh gia na eisagw mia eggrafh
    for (i = 1; i < NumOfBlocks; i++) {    //gia i=0 anferomaste sto mideniko block sto opoio den mpainoun entries 

        if (BF_ReadBlock(header_info.fileDesc, i, &block) != 0)  { BF_PrintError("ERROR: cannot read block");
        return -1;}

        blockId = i;       

        for (int j = 1; j <= NumOfEntries; j++) {   //edw o deikths block leitourgei san deikths sto kathe record mesa se kathe block

            memcpy(&x , block, sizeof(int));

            if (x == 0) {

                memcpy(block, &record.id, sizeof(int));             
                block = (void*)((int*)block + sizeof(int));

                memcpy(block, &record.name, sizeof(char)*15); 
                block = (void*)((char*)block + sizeof(char)*15);

                memcpy(block, &record.surname, sizeof(char)*25);
                block = (void*)((char*)block + sizeof(char)*25);

                memcpy(block, &record.address, sizeof(char)*50);
                
                if (BF_WriteBlock(header_info.fileDesc, i) != 0)  { BF_PrintError("ERROR: cannot write block");
                    return -1;
                } 
               
                return blockId;
            }
            else {
                block=(void*)((int*)block+sizeof(Record));
            }
        }
    }

    //se periptwsh pou den exw arketa block ftiaxnw kainourgio gia na mpei h eggrafh 
    if (i == NumOfBlocks) {


        if (BF_AllocateBlock(header_info.fileDesc) != 0)  { BF_PrintError("ERROR: cannot allocate block");
        return -1;}  

        if (BF_ReadBlock(header_info.fileDesc, NumOfBlocks, &block) != 0) { BF_PrintError("ERROR: cannot read block");
        return -1;}


        blockId = i;

        memcpy(block, &record.id, sizeof(int));
        block = (void*)((int*)block + sizeof(int));

        memcpy(block, &record.name, sizeof(char)*15); 
        block = (void*)((char*)block + sizeof(char)*15);

        memcpy(block, &record.surname, sizeof(char)*25);
        block = (void*)((char*)block + sizeof(char)*25);

        memcpy(block, &record.address, sizeof(char)*50);

        if (BF_WriteBlock(header_info.fileDesc, i) != 0)  { BF_PrintError("ERROR: cannot write block");
        return -1;};

        return blockId;
    }
    return -1;

}

//---------------------------------------------------------------------------------------------------//
//--------------------------------------Hp_DeleteEntry----------------------------------------------//

int HP_DeleteEntry (HP_info header_info, void* value) {

    int NumOfBlocks;
    void* block;
    int x = 0;
    int y, z;

    memcpy(&z, value, sizeof(int));
    NumOfBlocks = BF_GetBlockCounter(header_info.fileDesc);
    if ( NumOfBlocks< 0)  { BF_PrintError("ERROR: cannot get Block Counter");
        return -1;};

    for (int i = 1; i < NumOfBlocks; i++) {
    
        if (BF_ReadBlock(header_info.fileDesc, i, &block) != 0)  { BF_PrintError("ERROR: cannot read Block");
        return -1;}

        for (int j = 1; j <= NumOfEntries; j++) {
            
            memcpy(&y, block, sizeof(int));
            
            if (y == z) {   
                memcpy(block, &x, sizeof(int));
                block = (void*)((int*)block + sizeof(int));

                memcpy(block, &x, sizeof(char)*15);
                block = (void*)((char*)block + sizeof(char)*15);

                memcpy(block, &x, sizeof(char)*25);
                block = (void*)((char*)block + sizeof(char)*25);

                memcpy(block, &x, sizeof(char)*50);

                if (BF_WriteBlock(header_info.fileDesc, i) != 0)  { BF_PrintError("ERROR: cannot write block");
                return -1;}

                cout << "Entry has been deleted" << endl;
                return 0;
            }
            else {
                block=(void*)((int*)block+sizeof(Record));
            }
        }
    }
    
    return -1;
}

//--------------------------------------------------------------------------------------------------//
//--------------------------------------HP_GetAllEntries--------------------------------------------//

int HP_GetAllEntries (HP_info header_info, void* value) {

    int NumOfBlocks, y, z, counter = 0;
    int id;
    char name[15], surname[25], address[50];
    void *block, *block1;

    memcpy(&z, value, sizeof(int));

    NumOfBlocks = BF_GetBlockCounter(header_info.fileDesc);
    if ( NumOfBlocks < 0)  { BF_PrintError("ERROR: cannot get BlockCounter");
    return -1;}

    for (int i = 1; i < NumOfBlocks; i++) {
       
        if (BF_ReadBlock(header_info.fileDesc, i, &block) != 0)  { BF_PrintError("ERROR: cannot read block");
        return -1;}
        block1 = block;

        for (int j = 1; j <= NumOfEntries; j++) {

            counter = counter + 1;          
            memcpy(&id, block1, sizeof(int));
            block1 = (void*)((int*)block1 + sizeof(int));
            
            y = id;

            memcpy(&name, block1, sizeof(char)*15);
            block1 = (void*)((char*)block1 + sizeof(char)*15);

            memcpy(&surname, block1, sizeof(char)*25);
            block1 = (void*)((char*)block1 + sizeof(char)*25);

            memcpy(&address, block1, sizeof(char)*50);

            if (value == NULL) {    //an thelw na kanw print ola ta records 
                
                cout << "Block: " << i << ", Entry: " << j << endl;
                cout << "   ID:" << id << endl;
                cout << "   Name:" << name << endl;
                cout << "   Surname:" << surname << endl;
                cout << "   Address:" << address << endl; 

            }
            else if (y == z) {
             
                cout << "Block: " << i << ", Entry: " << j << endl;
                cout << "   ID:" << id << endl;
                cout << "   Name:" << name << endl;
                cout << "   Surname:" << surname << endl;
                cout << "   Address:" << address << endl;
                
                return counter;
            }
            else {
                block=(void*)((int*)block+sizeof(Record));
                block1 = block;
                
            }
        }
    }
    if (value == NULL) {
        return NumOfBlocks;
    }
    return -1;
}
