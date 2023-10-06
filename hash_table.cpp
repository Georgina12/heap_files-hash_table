#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <bits/stdc++.h>
#include <iostream>

using namespace std;

#include "hash_table.h"
#include "BF.h"


#define LENGTH 25

//-------------------------------------------------------------------------------------------------------//
//--------------------------------------HT_CreateIndex--------------------------------------------------//

int HT_CreateIndex (char* fileName, char attrType, char* attrName, int attrLength, long int buckets) {

    int fileDesc;
    void* block;

    if (BF_CreateFile(fileName) != 0) {
        BF_PrintError("Error: Cannot create file");
        return -1;
    }

    BF_Init();

    fileDesc = BF_OpenFile(fileName);
    if ( fileDesc < 0 ) {
        BF_PrintError("Error: Cannot open file");
        return -1;
    }

    for (int i = 0; i <= buckets; i++) {
        if (BF_AllocateBlock(fileDesc) != 0) {
            BF_PrintError("Error: Cannot allocate block");
            return -1;
        }
    }

    if (BF_ReadBlock(fileDesc, 0, &block) != 0) {
        BF_PrintError("Error: Cannot read block");
        return -1;
    }

    memcpy(block, &fileDesc, sizeof(int));
    block = (void*)((int*)block + sizeof(int));

    memcpy(block, &attrType, sizeof(char));
    block = (void*)((char*)block + sizeof(char));

    memcpy(block, attrName, sizeof(char)*LENGTH);
    block = (void*)((char*)block + sizeof(char)*LENGTH);

    memcpy(block, &attrLength, sizeof(int));
    block = (void*)((int*)block + sizeof(int));

    memcpy(block, &buckets, sizeof(long int));

    if (BF_WriteBlock(fileDesc, 0) != 0) {
        BF_PrintError("Error: Cant write block");
        return -1;
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------//
//---------------------------------------HT_OpenIndex-----------------------------------------------------//

HT_info* HT_OpenIndex (char* fileName) {

    int fileDesc;
    HT_info *HT = new HT_info;
    void* block;

    fileDesc = BF_OpenFile(fileName);
    if ( fileDesc < 0 ) {
        BF_PrintError("Error: Cannot open file");
        return NULL;
    }

    if(BF_ReadBlock(fileDesc, 0, &block) != 0) {
        BF_PrintError("Error: Cannot read block");
        return NULL;
    }

    HT->fileDesc = fileDesc;
    block = (void*)((int*)block + sizeof(int));

    memcpy(&HT->attrType, block, sizeof(char));
    block = (void*)((char*)block + sizeof(char));

    memcpy(&HT->attrName, block, sizeof(char)*LENGTH);
    block = (void*)((char*)block + sizeof(char)*LENGTH);

    memcpy(&HT->attrLength, block, sizeof(int));
    block = (void*)((int*)block + sizeof(int));

    memcpy(&HT->numBuckets, block, sizeof(long int));

    return HT;
}

//---------------------------------------------------------------------------------------------------------//
//---------------------------------------HT_CloseIndex----------------------------------------------------//

int HT_CloseIndex (HT_info* header_info) {

    if (BF_CloseFile(header_info->fileDesc) != 0) {
        BF_PrintError("Error: Cannot close file");
        return -1;
    }

    delete header_info; 

    cout << "File has closed" << endl;

    return 0;
}

//--------------------------------------------------------------------------------------------------------//
//------------------------------------HT_InsertEntry-----------------------------------------------------//

int HT_InsertEntry (HT_info header_info, Record record) {
    
    int bucket, x, NumOfBlocks, blockid, count = 1, flag = 0, y;
    void* block;

    bucket = record.id % header_info.numBuckets;  //This is hash function

    if (BF_ReadBlock(header_info.fileDesc, bucket+1, &block) != 0) {
        BF_PrintError("Error: Cannot read block");
        return -1;
    }

    memcpy(&x, block, sizeof(int));

    while (flag == 0) {

        if ( (count == NumOfEntries + 1) && (x == 0) ) {  //1st case

            if (BF_AllocateBlock(header_info.fileDesc)!= 0) {
                BF_PrintError("Error: Cannot allocate block");
                return -1;
            }

            NumOfBlocks = BF_GetBlockCounter(header_info.fileDesc);
            if (NumOfBlocks < 0) {
                BF_PrintError("Error: Cannot get block counter");
                return -1;
            }

            memcpy(block, &NumOfBlocks, sizeof(int));

            if (BF_ReadBlock(header_info.fileDesc, NumOfBlocks, &block) != 0) {
                BF_PrintError("Error: Cannot read block");
                return -1;
            }

            blockid = NumOfBlocks;

            memcpy(block, &record.id, sizeof(int));
            block = (void*)((int*)block + sizeof(int));

            memcpy(block, &record.name, sizeof(char)*15); 
            block = (void*)((char*)block + sizeof(char)*15);

            memcpy(block, &record.surname, sizeof(char)*25);
            block = (void*)((char*)block + sizeof(char)*25);

            memcpy(block, &record.address, sizeof(char)*50);
                
            if (BF_WriteBlock(header_info.fileDesc, blockid) != 0) {
                BF_PrintError("Error: Cannot write block");
                return -1;
            }

            flag = 1;
            y = blockid;

        }
        else if ( (count < NumOfEntries + 1) && (x == 0) ) {  //2nd case

            blockid = bucket;

            memcpy(block, &record.id, sizeof(int));
            block = (void*)((int*)block + sizeof(int));

            memcpy(block, &record.name, sizeof(char)*15); 
            block = (void*)((char*)block + sizeof(char)*15);

            memcpy(block, &record.surname, sizeof(char)*25);
            block = (void*)((char*)block + sizeof(char)*25);

            memcpy(block, &record.address, sizeof(char)*50);
                
            if (BF_WriteBlock(header_info.fileDesc, blockid) != 0) {
                BF_PrintError("Error: Cannot write block");
                return -1;
            }

            flag = 1;
            y = blockid;
        }
        else if ( (count == NumOfEntries + 1) && (x != 0) ) {  //3rd case

            if (BF_ReadBlock(header_info.fileDesc, x, &block) != 0) {
                BF_PrintError("Error: Cannot read block");
                return -1;
            }

            bucket = x;
            count= 1;
        }
        else {  //4th case

            block = (void*)((int*)block + sizeof(Record));
            memcpy(&x, block, sizeof(int));
            count = count + 1;
        }
    }
    return y;
}

//---------------------------------------------------------------------------------------------------//
//--------------------------------------HT_DeleteEntry----------------------------------------------//

int HT_DeleteEntry (HT_info header_info, void* value) {
    int val, bucket, blockval, flag = 0, x = 0, counter = 1, y;
    void* block;

    memcpy(&val, value, sizeof(int));

    bucket = val % header_info.numBuckets;  //this is hash function

    if (BF_ReadBlock(header_info.fileDesc, bucket+1, &block) != 0) {
        BF_PrintError("Error: Cannot read block");
        return -1;
    }

    memcpy(&blockval, block, sizeof(int));

    while (flag == 0) {
        if ( (blockval == val) && (counter < NumOfEntries + 1) ) {
            memcpy(block, &x, sizeof(int));
            block = (void*)((int*)block + sizeof(int));

            memcpy(block, &x, sizeof(char)*15);
            block = (void*)((char*)block + sizeof(char)*15);

            memcpy(block, &x, sizeof(char)*25);
            block = (void*)((char*)block + sizeof(char)*25);

            memcpy(block, &x, sizeof(char)*50);

            if (BF_WriteBlock(header_info.fileDesc, bucket) != 0) {
                BF_PrintError("Error: Cannot write block in memory");    
                return -1;
            }

            flag = 1;
            y = 0;
        }
        else if ( (blockval == 0) && (counter == NumOfEntries + 1) ) {
            
            flag = 1;
            y = -1;

        }
        else if ( (blockval != 0) && (counter == NumOfEntries + 1) ) {
            
            if (BF_ReadBlock(header_info.fileDesc, blockval, &block) != 0) {

                BF_PrintError("Error: Cannot read block");
                return -1;
            }

            bucket = blockval;

            memcpy(&blockval, block, sizeof(int));

            counter = 1;
        }
        else {
            block = (void*)((int*)block + sizeof(Record)); 
            counter = counter + 1;

            memcpy(&blockval, block, sizeof(int));
        }
    }

    cout << "Entry has been deleted" << endl;
    return y;
}

//--------------------------------------------------------------------------------------------------//
//--------------------------------------HT_GetAllEntries--------------------------------------------//

int HT_GetAllEntries (HT_info header_info, void* value) {
    int val, bucket, blockval, id, counter, flag = 0, blockcounter = 1, x;
    void* block;
    char name[15], surname[25], address[50];
    
    if (value == NULL) {

        for (int i =1; i <= header_info.numBuckets; i++) {
            
            if (BF_ReadBlock(header_info.fileDesc, i, &block) != 0) {
                BF_PrintError("Error: Cannot read block");
                return -1;
            }

            memcpy(&blockval, block, sizeof(int));

            counter = 1;
            
            cout << "Bucket: " << i-1 << endl;
            
            while ( flag == 0 ) { 
                 
                if ( (blockval != 0) && (counter < NumOfEntries + 1) ) {
                    memcpy(&id, block, sizeof(int));
                    block = (void*)((int*)block + sizeof(int));

                    memcpy(&name, block, sizeof(char)*15);
                    block = (void*)((char*)block + sizeof(char)*15);

                    memcpy(&surname, block, sizeof(char)*25);
                    block = (void*)((char*)block + sizeof(char)*25);

                    memcpy(&address, block, sizeof(char)*50);

                    cout << "   Entry: " << counter << endl;
                    cout << "       ID: " << id << endl;
                    cout << "       Name: " << name << endl;
                    cout << "       Surname: " << surname << endl;
                    cout << "       Address: " << address << endl; 

                    block = (void*)((int*)block + sizeof(Record));
                    counter= counter + 1;
                }
                else if ( (blockval == 0) && (counter == NumOfEntries + 1) ) {
                    flag =1;
                }
                else if ( (blockval != 0) && (counter == NumOfEntries + 1) ) {
                    
                    if (BF_ReadBlock(header_info.fileDesc, blockval, &block) != 0) {
                        BF_PrintError("Error: Cannot read block");
                        return -1;
                    }

                    counter = 1;
                    memcpy(&blockval, block, sizeof(int));
                }
                else {

                    block = (void*)((int*)block + sizeof(Record));
                    memcpy(&blockval, block, sizeof(int));
                }
            }
        }
        blockcounter = BF_GetBlockCounter(header_info.fileDesc);
        if (blockcounter < 0) {
            BF_PrintError("Error: Cannot get block counter");
        }
        
        x = blockcounter;
    }
    else {

        memcpy(&val, value, sizeof(int));

        bucket = val % header_info.numBuckets;  //this is a hash function

        if (BF_ReadBlock(header_info.fileDesc, bucket + 1, &block) != 0) {
            BF_PrintError("Error: Cannot read block");
            return -1;
        }

        memcpy(&blockval, block, sizeof(int));

        while (flag == 0) {
            if ( (blockval == val) && (counter < NumOfEntries) ) {
                memcpy(&id, block, sizeof(int));
                block = (void*)((int*)block + sizeof(int));

                memcpy(&name, block, sizeof(char)*15);
                block = (void*)((char*)block + sizeof(char)*15);

                memcpy(&surname, block, sizeof(char)*25);
                block = (void*)((char*)block + sizeof(char)*25);
                
                memcpy(&address, block, sizeof(char)*50);

                cout << "Bucket: " << bucket << endl;
                cout << "   Entry: " << counter << endl;
                cout << "       ID: " << id << endl;
                cout << "       Name: " << name << endl;
                cout << "       Surname: " << surname << endl;
                cout << "       Address: " << address << endl; 

                x = blockcounter;
                flag = 1;
            }
            else if ( (blockval == 0) && (counter == NumOfEntries +1) ) {
                return -1;
            }
            else if ( (blockval != 0) && (counter == NumOfEntries + 1) ) {
                if (BF_ReadBlock(header_info.fileDesc, blockval, &block) != 0) {
                    BF_PrintError("Error: Cannot read block");
                    return -1;
                }

                blockcounter = blockcounter + 1;
                memcpy(&blockval, block, sizeof(int));
                counter = 1;
            }
            else if ( (blockval != 0) && (counter != NumOfEntries + 1) ) {
                block = (void*)((int*)block + sizeof(Record));
                memcpy(&blockval, block, sizeof(int));
                counter = counter + 1;
            }
        }
    }
    return x;
    
}


int HashStastistics (char* filename) {
    int NumOfBlocks, min = -1, max = -1, count, counter = 1, counterb = 1, counterc = 1, i, blockval, flag = 0, count_entries;
    void* block;
    HT_info* HT;


    // a) number of blocks
    
    HT = HT_OpenIndex(filename);
    if (HT->fileDesc < 0) {
        BF_PrintError("Error: Cannot open file");
        return -1;
    }

    NumOfBlocks = BF_GetBlockCounter(HT->fileDesc);
    if (HT->fileDesc < 0) {
        BF_PrintError("Error: Cannot get block counter");
        return -1;
    }

    cout << "This file has " << NumOfBlocks <<" blocks." << endl;

    // b) min, aver and max number of entries of each bucket

    for (i = 1; i <= HT->numBuckets; i++){
        if (BF_ReadBlock(HT->fileDesc, i, &block) != 0) {
            BF_PrintError("Error: Cannot read block");
            return -1;
        }

        count_entries = 1;

        memcpy(&blockval, block, sizeof(int));

        while ( flag == 0 ) {
            if ( (blockval == 0) && (counter == NumOfEntries + 1) ) {
                
                flag = 1;

            }
            else if ( (blockval == 0) && (counter < NumOfEntries + 1) ) {
                
                block = (void*)((int*)block + sizeof(Record));

            }
            else if ( (blockval!= 0) && (counter == NumOfEntries + 1) ) {
                
                if (BF_ReadBlock(HT->fileDesc, blockval, &block) != 0) {
                    BF_PrintError("Error: Cannot read block");
                    return -1;
                }
                counter = 1;
                counterb = counterb + 1;

            }
            else if ( (blockval != 0) && (counter < NumOfEntries + 1) ) {
                
                count = count + 1;
                count_entries = count_entries + 1;
                counter = counter + 1;
                block = (void*)((int*)block + sizeof(Record));

            } 
        }

        if (counterb > 0) {
            counterc = counterc + 1;
        }

        cout << "Bucket " << i << " has: " << counterb << " blocks that overflow" << endl;

        if (count_entries < min) {
            min = count_entries;
        }

        if (count_entries > max) {
            max = count_entries;
        }
    }

    cout << "The minimum number of entries are " << min << endl;
    cout << "The maximum number of entries are " << max << endl;
    cout << "The average number of entries is " << count/HT->numBuckets << endl;

    // c) average number of blocks in each bucket

    cout << "The average number of blocks in each bucket is " << NumOfBlocks/HT->numBuckets << endl;

        
    // d) number of buckets that overflow to a new block and how many overflow blocks each bucket has
    
    cout << counterc << " buckets overflow" << endl;

    if (HT_CloseIndex(HT) != 0) {
        return -1;
    }

    return 0;
}