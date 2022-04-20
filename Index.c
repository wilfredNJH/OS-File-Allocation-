#define EMPTY 0
#define DIRECTORYINDEX 10
#define FILEBLOCKS 100
#define ELEMENTSINBLOCK 5
#define ULLBITSIZE 64 

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "CSVReader.h"
#include <limits.h>

int hard_disk[500] = {EMPTY}; // hard disk
unsigned long long int bitmap1 = 0; // bitmap to locate empty blocks
unsigned long long int bitmap2 = 0; // bitmap to locate empty blocks
int totalFiles = 0;


int getNumberOfFreeBlocks(void)
{
    int numberOfFreeBlocks = 0; // index of the free block
    int index = 2;
    // checking the first bitmap
    unsigned long long int bitMask = 0b1; // init bit mask
    for (int i = 2; i < ULLBITSIZE; ++i)
    {
        // shift the bitmask to check if the bitmap is set
        if (!(bitmap1 & (bitMask << i)))
        {
            numberOfFreeBlocks++; // increment the number of free blocks
        }
        bitMask = 0b1; // reset bit mask
        index++;
    }
    // checking for the second bitmap
    for (int i = 0; i < ULLBITSIZE; ++i)
    {
        // shift the bitmask to check if the bitmap is set
        if (!(bitmap2 & (bitMask << i)))
        {
            numberOfFreeBlocks++; // increment number of free blocks
        }
        bitMask = 0b1; // reset bit mask
        index++;//increment the index 
        //if index is 100 means all the blocks are set
        if(index == 100){
            return numberOfFreeBlocks;
        }
    }
    return numberOfFreeBlocks; // returning the number of free blocks
}

int getFreeBlock(void){
  int index = 2;//index of the free block 
  //checking the first bitmap 
  unsigned long long int bitMask = 0b1;//init bit mask 
  bool checkSecond = false;
  for(int i = 0 ; i < ULLBITSIZE-2  ; ++i){
    //shift the bitmask to check if the bitmap is set 
    if(bitmap1 & (bitMask << index)){
      index++;//increment the index 
      bitMask = 0b1;//reset bit mask 
      if(i == ULLBITSIZE-3){
          checkSecond = true;//time to check the second bitmap
      }
      continue;//continue checking the first bitmap
    }else{
        return index;//return the index
    }
  }
  //checking for the second bitmap
  if(checkSecond == true){
    for(int i = 0 ; i < ULLBITSIZE ; ++i){
      //shift the bitmask to check if the bitmap is set 
      if(bitmap2 & (bitMask << i)){
        index++;//increment the index 
        bitMask = 0b1;//reset bit mask 
        continue;
      }else{
        return index;//return the index
      }
      //if index is 100 means all the blocks are set
      if(index == FILEBLOCKS){
          return -1;
      }
    }
  }
    //all is full 
    return -1;
}

void set_bit(unsigned long long int* bitmap1, unsigned long long int* bitmap2, int const bitToSet)
{
  unsigned long long int mask = 0;
  
  mask |= (1 << bitToSet);
  if(bitToSet < 64)
  (*bitmap1) |= mask; // set the bitmap to be this
  else
  (*bitmap2) |= mask; // set the bitmap to be this
  
}

void clear_bit(unsigned long long int* bitmap1, unsigned long long int* bitmap2, int const bitToSet)
{
  unsigned long long int mask = 0;

  mask |= (1 << bitToSet);

  if(bitToSet < 64)
  (*bitmap1) &= ~mask; // clear bit map
  else
  (*bitmap2) &= ~mask;
  
}

bool check_bit(unsigned long long int* bitmap1, unsigned long long int* bitmap2, int const bitToCheck)
{
  unsigned long long int mask = 0;
  mask = 1UL << bitToCheck;

  if(bitToCheck < 64)
    return ((*bitmap1) & mask) ? true : false;
  else
    return ((*bitmap2) & mask) ? true : false;

  return true;
}

void setDirStruct(char* fileDir, int* harddisk, int HDslot) 
{
  int mask =0;
  for(int i =0 ; i < 4; ++i)
  {
    mask |= fileDir[i] << (i * 8);
  }
  *(harddisk + HDslot) = mask;
}

void GetDirStruct(char* fileDir, int* harddisk, int HDslot) 
{
  int mask = harddisk[HDslot];
  for(int i =0 ; i < 4; ++i)
  {
    fileDir[i] = mask >> (i*8);
  }
}

void PrintHardDisk(void)
{
  printf("\n********** HARD DISK INFO **********\n");

  // Print total added files
  printf("%13s: %d\n", "TOTAL FILES: ", totalFiles);

  // Count free integers
  int countInt = 0;
  for (int i = 0; i < 500; ++i)
  {
    if (hard_disk[i] == EMPTY) { ++countInt; }
  }
  printf("%-13s: %d\n", "OVERALL SIZE: ", countInt);

  char cfileDir[4] = {0};
    // Directory printing
    printf("\nPRINTING HARD DISK MAP...\n");
    for(int i = 0; i < DIRECTORYINDEX; ++i)
    {
      if ((i % 5) == 0)
      {
        printf("---------- Directory Block %-2d ---------\n", i/5);
        printf("%-12s %-5s %-5s\n", "Header ", "Name", "Index");
      }
      GetDirStruct(cfileDir, hard_disk, i);
      printf("%s ", "File Data : " );
      for(int k = 0; k < 2 ; ++k)
      {
        printf("%-5d ", cfileDir[k]);
      }
      printf(" \n" );
    }
    // Data printing
    for (int i = DIRECTORYINDEX; i < 500; ++i)
    {
        if ((i % 5) == 0) { printf("\n\n------------ File Block %-2d ------------\n", i/5); }
        printf("|%-3d ", hard_disk[i]);
    }
    printf("\n\n...PRINTING COMPLETE\n");
    printf("\n******** HARD DISK INFO END ********\n");
}

bool Add(const int* data, int dataCount)
{
    printf("Adding File: %d...\n", data[0]);
    // Find slot for directory data
    int directorySlot = 0;
    for (; directorySlot < DIRECTORYINDEX; ++directorySlot)
    {
        if (hard_disk[directorySlot] == EMPTY) { break; }
    }
    if (directorySlot == DIRECTORYINDEX) { return false; } // No empty directory slot avail

    char dirData[4] = {0, 0, 0, '\0'}; // Initialize data
    dirData[0] = data[0]; // Store file name
    int requiredBlocks = ((dataCount-1) % ELEMENTSINBLOCK) != 0 ? (dataCount-1)/ELEMENTSINBLOCK + 1 : (dataCount-1)/ELEMENTSINBLOCK;
    // Check if bitmap has enough space to allocate
    // First rejection test
    if (getNumberOfFreeBlocks() < requiredBlocks+1) { return false; }
    // Second rejection test
    if (requiredBlocks > ELEMENTSINBLOCK) { return false; } // Check if there is too many blocks to store in indexing block

    // Get the first block as indexing block
    int indexingBlock = getFreeBlock();
    set_bit(&bitmap1, &bitmap2, indexingBlock);
    dirData[1] = indexingBlock;

    // Allocate data to data blocks
    int dataIterator = 0;
    // For each required block to store the data...
    for (int blockNum = 0; blockNum < requiredBlocks; ++blockNum)
    {
        // Allocate free block
        int freeBlockNumber = getFreeBlock();
        set_bit(&bitmap1, &bitmap2, freeBlockNumber);
        hard_disk[indexingBlock*ELEMENTSINBLOCK + blockNum] = freeBlockNumber; // Store allocated block into indexing block
        // Insert data into block
        do
        {
            hard_disk[(freeBlockNumber*ELEMENTSINBLOCK)+(dataIterator%ELEMENTSINBLOCK)] = data[dataIterator+1];
            ++dataIterator;
        } while (((dataIterator%ELEMENTSINBLOCK) != 0) && dataIterator < (dataCount-1));
    }
    setDirStruct(dirData, hard_disk, directorySlot);
    printf("File %d added!\n", data[0]);
    ++totalFiles;
    return true;
}

bool Read(int fileName)
{
    printf("\nReading file: %d...\n", fileName);
    char dirData[4];
    int directorySlot = 0;
    for (; directorySlot < DIRECTORYINDEX; ++directorySlot)
    {
        
        GetDirStruct(dirData, hard_disk, directorySlot);
        if (dirData[0] == fileName) { break; }
        memset(dirData, 0, 4);
    }
    if (directorySlot == DIRECTORYINDEX) { return false; }
    
    for (int index = 0; index < ELEMENTSINBLOCK; ++index)
    {
        // Get block number written of each index entry
        int blockNum = hard_disk[(int)dirData[1]*ELEMENTSINBLOCK + index];
        if (blockNum == EMPTY) { break; } // No more indices left

        // Read each data in each element in the block
        printf("Contents: ");
        for (int elementNum = 0; elementNum < ELEMENTSINBLOCK; ++elementNum)
        {
            if (hard_disk[blockNum*ELEMENTSINBLOCK + elementNum] == EMPTY) { break; }
            printf("%d ", hard_disk[blockNum*ELEMENTSINBLOCK + elementNum]);
        }
        printf("\n");
    }
   
    return true;
}

bool Delete(int fileName)
{
    printf("\nDeleting File: %d...\n", fileName);
    char dirData[4];
    int directorySlot = 0;
    for (; directorySlot < DIRECTORYINDEX; ++directorySlot)
    {
        
        GetDirStruct(dirData, hard_disk, directorySlot);
        if (dirData[0] == fileName) { break; }
        memset(dirData, 0, 4);
    }
    if (directorySlot == DIRECTORYINDEX) { return false; }

    
    /*
        1) For each block entry in index, set each element to EMPTY
        2) Clear the bit in bitmap
    */
    for (int index = 0; index < ELEMENTSINBLOCK; ++index)
    {
        // Get block number written of each index entry
        int blockNum = hard_disk[(int)dirData[1]*ELEMENTSINBLOCK + index];
        if (blockNum == EMPTY) { break; } // No more indices left

        for (int elementNum = 0; elementNum < ELEMENTSINBLOCK; ++ elementNum)
        {
            hard_disk[blockNum*ELEMENTSINBLOCK + elementNum] = EMPTY;
        }
        clear_bit(&bitmap1, &bitmap2, blockNum);
        hard_disk[(int)dirData[1]*ELEMENTSINBLOCK + index] = 0;
    }
    clear_bit(&bitmap1, &bitmap2, (int)dirData[1]);
    hard_disk[directorySlot] = 0;
    --totalFiles;
    printf("File %d deleted!\n", fileName);
    return true;
    
}

int main(int argc, const char* argv[])
{
    int opCount = 0;
    Operation* operations = CSVReader(argv[1], &opCount);
    
    for (int i = 0; i < opCount; ++i)
    {
        if (strcmp(operations[i].op, "add") == 0)
        {
            if (!Add(operations[i].Data, operations[i].DataCount)) { printf("Adding failed\n");}
            else { PrintHardDisk(); }
        }
        else if (strcmp(operations[i].op, "read") == 0)
        {
            if (!Read(operations[i].Data[0])) { printf("Reading failed\n"); }
            else { PrintHardDisk(); }
        }
        else if (strcmp(operations[i].op, "delete") == 0)
        {
            if (!Delete(operations[i].Data[0])) { printf("Deleting failed\n"); }
            else { PrintHardDisk(); }
        }
        
    }

    for (int i = 0; i < opCount; ++i)
    {
        free(operations[i].op);
        free(operations[i].Data);
    }
    free(operations);
    
    return 0;
}