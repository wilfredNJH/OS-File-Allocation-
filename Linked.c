#include "CSVReader.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define EMPTY 0
#define DIRECTORYINDEX 10
#define FILEBLOCKS 100
#define ELEMENTSINBLOCK 5
#define ULLBITSIZE 64 

int hard_disk[500] = {EMPTY}; // init all members to 0
int totalFiles = 0;

enum InputOperation{
  ADD,
  READ,
  DELETE
};

unsigned long long int bitmap1 = 0;//bitmap 1 
unsigned long long int bitmap2 = 0;//bitmap 2

//easier to have accept 2 bit map as parameter
void set_bit(unsigned long long int* bitmap1, unsigned long long int* bitmap2, int const bitToSet)
{
  unsigned long long int mask = 0;
  // for(int i =0; i < bitToSet; ++i )
  //   mask |= (1UL << i); 
  
  mask |= (1 << bitToSet);

  if(bitToSet < 64)
  (*bitmap1) |= mask; // set the bitmap to be this
  else
  (*bitmap2) |= mask; // set the bitmap to be this
  
}

int ceilInt(float num) {
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
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

//getting the nearest free block from the 2 bitmaps 
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

//getting the number of free block available
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

void setDirStruct(char* fileDir, int* harddisk, int HDslot) 
{
  int mask =0;
  for(int i =0 ; i < 4; ++i)
  {
    // printf("setDirStruct fileDir[%d] : %d \n", i , fileDir[i]);
    mask |= fileDir[i] << (i * 8);
  }
  // printf("mask value : %d \n", mask);
  *(harddisk + HDslot) = mask;
}

void GetDirStruct(char* fileDir, int* harddisk, int HDslot) 
{
  int mask = harddisk[HDslot];
  for(int i =0 ; i < 4; ++i)
  {
    fileDir[i] = mask >> (i*8);
  }
  // *(harddisk + HDslot) = mask;
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
        printf("%-12s %-5s %-5s %-5s\n", "Header ", "Name", "Start", "End");
      }
      GetDirStruct(cfileDir, hard_disk, i);
      printf("%s ", "File Data : " );
      for(int k = 0; k < 3 ; ++k)
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


//check with the file
bool fileRead(const int * data)
{

  int foundIndex = 0;
  char cfileDir[4] = {0};
  for(int i =0 ; i < DIRECTORYINDEX; ++i)
  {
    GetDirStruct(cfileDir, hard_disk, i); // the directory found
    if(cfileDir[0] == data[0]) // file found within the the file directory
    {
      // printf("\n cfileDir[0] : %d , data[0] : %d \n\n ", cfileDir[0] , data[0]);
      foundIndex = i;
      break;
    }
    if(i == DIRECTORYINDEX){
      return false;
    }

  }
  
  // starting block is equal to ending block, file size is one block
  if(cfileDir[1] == cfileDir[2]) 
  {
    printf( "Contents : ");
    int Index = cfileDir[1] * ELEMENTSINBLOCK;
    while(hard_disk[Index] != -1)
    {
      printf( "%d ", hard_disk[Index++]);
    }
    printf( "\n" );
    return true;
  }
  
  int currentBlock = cfileDir[1]; //stores the current block index 
  int endBlock = cfileDir[2];     //stores the end block 
  //main reading
  printf( "Contents : ");

  while(true)
  {
    //read all 5 entries
    if(currentBlock == endBlock){
      
      int Index = currentBlock * ELEMENTSINBLOCK;
      
      while(hard_disk[Index] != -1)
      {
        printf( "%d ", hard_disk[Index++]);
      }
      break;//break out of the reading loop

    }else{
      int Index = currentBlock * ELEMENTSINBLOCK;//getting the starting index of the disk
      //read 4 entries
      for(int i = 0; i < ELEMENTSINBLOCK-1 ; ++i){
        printf( "%d ", hard_disk[Index++]); // prints out 4 spaces of the block
      }
      currentBlock = hard_disk[Index];//assign current block to the next block
    }
  }
  printf( "\n" );
  return true;
}

bool fileDelete(const int * data )
{
  printf("Deleting file: %d..\n", data[0]);
  char cfileDir[4] = {0};
  for(int i =0 ; i < DIRECTORYINDEX; ++i)
  {
    GetDirStruct(cfileDir, hard_disk, i); // the directory found
    if(cfileDir[0] == data[0]) // file found within the the file directory
    {
      hard_disk[i] = 0;
      break;
    }
  }
  
  // File size is one block, setting content to be 0
  if(cfileDir[1] == cfileDir[2]) 
  {
    int Index = cfileDir[1] * ELEMENTSINBLOCK;
    while(hard_disk[Index] != -1)
    {
      hard_disk[Index] = 0;//remove everything within the entry
      Index++;
    }
    hard_disk[Index] = 0;//removing the pointer
    clear_bit(&bitmap1, &bitmap2, cfileDir[1]); //reseting the bitmap
    return true;
  }
  
  int currentBlock = cfileDir[1]; //stores the current block index 
  int endBlock = cfileDir[2];     //stores the end block 
  //main reading
  printf( "File Name : %d, Content : ", cfileDir[0] );
  while(true)
  {
    //read all 5 entries
    if(currentBlock == endBlock){
      
      int Index = currentBlock * ELEMENTSINBLOCK; //index 
      while(hard_disk[Index] != -1)
      {
        hard_disk[Index] = 0; //remove everything within the entry
        Index++;
      }
      hard_disk[Index] = 0;//removing the pointer
      clear_bit(&bitmap1, &bitmap2, currentBlock); //reseting the bitmap
      break;//break out of the reading loop

    }
    else // deleting 4 elements
    {
      int Index = currentBlock * ELEMENTSINBLOCK;//getting the starting index of the disk
      //deleting 4 entries
      for(int i = 0; i < ELEMENTSINBLOCK-1 ; ++i)
      {
        hard_disk[Index] = 0; //remove everything within the entry
        Index++;
      }
      clear_bit(&bitmap1, &bitmap2, currentBlock); //reseting the bitmap
      currentBlock = hard_disk[Index];//assign current block to the next block
      hard_disk[Index] = 0;//deleting the 5th entry
    }
  }
  return false;
}


//operation add to the hard disk, 
bool fileAdd(const int * data, int dataCount)
{
  //sanity check that we have enough space to allocate 
  //getting the number of required blocks 
  int requiredBlocks = (dataCount % ELEMENTSINBLOCK) != 0 ? dataCount/ELEMENTSINBLOCK + 1 : dataCount/ELEMENTSINBLOCK;
  
  ///getting the number of empty block count 
  // printf("Number of Empty Blocks : %d\n",getNumberOfFreeBlocks());

  // First rejection test
  if (getNumberOfFreeBlocks() < requiredBlocks) { return false; }
  // Second rejection test
  if (requiredBlocks > ELEMENTSINBLOCK) { return false; }
 
  //store the file name and first and last block in to the directory structure
  bool allocated = false;
  //loop through the hard disk (first 10) to get the empty file directory
  for(int i = 0 ; i < DIRECTORYINDEX ; ++i)
  {
    if(hard_disk[i] == 0) // available directory index 
    {
      //format data and store it 
      double numOfData = dataCount - 1;
      //getting the number of blocks required 
      int blocksRequired = ceilInt(numOfData/((float)ELEMENTSINBLOCK-1));
      // printf("blocksRequired() : %d\n",blocksRequired); // returns blocksRequired
      int storageBlockIndex[blocksRequired];//stores the block index 
      int startIndex = 0, endIndex = 0;

      if(blocksRequired == 1) //only one block required
      {
        // printf("only one block required, getFreeBlock() : %d  \n", getFreeBlock());
        storageBlockIndex[0] = getFreeBlock();
        startIndex = storageBlockIndex[0];
        endIndex = storageBlockIndex[0];
        set_bit(&bitmap1,&bitmap2,storageBlockIndex[0]);
      }
      else // more then 1 block required
      {
        for(int j = 0 ; j < blocksRequired ; j++){
          //if it's the first block 
          if(j == 0)
          {
            storageBlockIndex[j] = getFreeBlock();//getting the index of the free blocks
            startIndex = storageBlockIndex[j];
          }
          else if(j == blocksRequired - 1)
          {
            //last block 
            storageBlockIndex[j] = getFreeBlock();//getting the index of the free blocks
            endIndex = storageBlockIndex[j];
            // printf("@@@@ INSIDE endIndex: %d , getFreeBlock() : %d  \n",endIndex, getFreeBlock());
          }
          else
          {
            storageBlockIndex[j] = getFreeBlock();//getting the index of the free blocks
          }
          set_bit(&bitmap1,&bitmap2,storageBlockIndex[j]);//setting the bit index
        }
      }

      //store all the formated data into the hard_disk 
      int dataIndex = 1;
      for(int k = 0; k < blocksRequired; k++){
        int startHardDiskIndex = storageBlockIndex[k] * ELEMENTSINBLOCK;//getting the starting hard disk index

        //loop through the block to store data in
        for(int m = 0; m < ELEMENTSINBLOCK; m++){
          //if it's the last one, store the pointer to the next block and it's the last block
          if(m == ELEMENTSINBLOCK-1 && k != blocksRequired){
            hard_disk[startHardDiskIndex++] = storageBlockIndex[k+1];//stores the pointer to next block
            // printf("Storing pointer to next block : %d\n",hard_disk[startHardDiskIndex-1]);
            break;//break the loop
          }else if(k == blocksRequired-1 && dataIndex == dataCount){
            //storing the last index indicator
            hard_disk[startHardDiskIndex++] = -1;
            // printf("Storing of the last data : %d\n",hard_disk[startHardDiskIndex-1]);
            break;//break the for loop
          }else{
            hard_disk[startHardDiskIndex++] = data[dataIndex];//storing of the data into the harddisk
            // printf("Storing normal file data : %d\n",hard_disk[startHardDiskIndex-1]);
          }
          dataIndex++;//incrementing
        }
      }

      char fileDir[4];
      fileDir[0] = data[0]; // 0111 1001 = 127
      fileDir[1] = startIndex; // 10 = 2
      fileDir[2] = endIndex;
      fileDir[3] = 0;

      setDirStruct(fileDir, hard_disk, i );
      // printf("harddisk[0] value : %d \n", hard_disk[0]);

      char cfileDir[4];

      allocated = true; //file is allocated 
      break;//break out of loop
    }  
  }

  return true;
}


int main(int argc, const char* argv[])
{

  int opCount = 0;
  Operation* opArray = CSVReader(argv[1], &opCount);

  //main body loop for all operations...
  for(int i = 0 ; i < opCount; ++i)
  {
    enum InputOperation inop;//input operations 
    if(strcmp(opArray[i].op, "add") == 0) inop = ADD;
    if(strcmp(opArray[i].op, "read") == 0) inop = READ;
    if(strcmp(opArray[i].op, "delete") == 0) inop = DELETE;
    
    //based on the operations, do
    switch(inop)
    {
      case ADD:
        printf("Adding File: %d...\n",(opArray+i)->Data[0]);
        if(fileAdd((opArray+i)->Data,(opArray+i)->DataCount)){
          printf("File %d added!\n",(opArray+i)->Data[0]);
          totalFiles++;
        }else{
          printf("Error, File %d could not be added!\n",(opArray+i)->Data[0]);
        }
        PrintHardDisk();
        break;
      case READ: 
      
        printf("Reading File %d...\n",(opArray+i)->Data[0]);
        if(fileRead((opArray+i)->Data)){}
        else
        {
          printf("File %d does not exist...\n",(opArray+i)->Data[0]);
        }
        // PrintHardDisk();
        break;      
      case DELETE:
        if(fileDelete((opArray+i)->Data))
        {
          printf("File %d is successfully deleted!\n",(opArray+i)->Data[0]);
          totalFiles--;
        }
        else
        {
          printf("Deleting failed\n");
        }
        PrintHardDisk();
        break;
      default: 
          printf("No such Operation exist!\n");
      break;
    }
  }
  
  // PrintHardDisk();

  //free operation
  for (int i = 0; i < opCount; ++i)
  {
    free(opArray[i].op);
    free(opArray[i].Data);
  }
  free(opArray);

return 0;
}

