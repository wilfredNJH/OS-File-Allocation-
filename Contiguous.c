#include "CSVReader.h"
#include <stdbool.h>

#define EMPTY 0
#define DIRECTORYINDEX 10
#define FILEBLOCKS 100
#define ELEMENTSINBLOCK 5
#define ULLBITSIZE 64 

int totalFiles = 0;


int hard_disk[500] = {EMPTY};
unsigned long long int bitmap1 = 0;//bitmap 1 
unsigned long long int bitmap2 = 0;//bitmap 2

//easier to have accept 2 bit map as parameter
void set_bit(unsigned long long int* bitmap1, unsigned long long int* bitmap2, int const bitToSet)
{
  unsigned long long int mask = 0;
  
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
        printf("%-12s %-5s %-5s %-5s\n", "Header ", "Name", "Start", "Length");
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

bool ContiguousAllocate(const int* data, int dataCount)
{
  printf("Adding file: %d...\n", data[0]);
  //Finding out how many blocks we need 
  int requiredBlocks = (dataCount % ELEMENTSINBLOCK) != 0? dataCount/ELEMENTSINBLOCK + 1 : dataCount/ELEMENTSINBLOCK;

  //Check if bitmap has enough space to allocate
  int emptyBlockCount = getNumberOfFreeBlocks();

  // First rejection test
  if (emptyBlockCount < requiredBlocks) {  
    printf("Not enough space left in memory.\n");
    return false; }

  //Second rejection test
  if(requiredBlocks > ELEMENTSINBLOCK){
    printf("Required Blocks more than memory can hold");
    return false;
  }

  //Initializations
  int startBlock = 0, endBlock = 0;
  int contiguousBlocks = 0, endOfFileIndex = 0, checker = 0;
  bool allocated = false;
  //Iterate through the blocks in memory
  for(int i = 2; i < FILEBLOCKS; ++i)
  {
    int k = 1;
    //Check if block is empty
    if(check_bit(&bitmap1, &bitmap2, i) == 0)
    {
        ++contiguousBlocks;
        //Check if contiguous blocks is equal to the number of blocks
        if(contiguousBlocks == requiredBlocks)
        {
            //Starting block 
            startBlock = i + 1 - requiredBlocks;
            //Iterate through the block
            for(int j = startBlock * 5; k < dataCount; ++j, ++k)
            {
              hard_disk[j] = data[k];
              if((k % 6) == 0)
              {
                //Set that element of the block to the data
                ++checker;
              }
            }
            endBlock = startBlock + checker; //Endblock
            allocated = true;
            break;
        }
    }
    else
    {
      contiguousBlocks = 0;
    }
  }

  //Set bitmap to those blocks that are occupied
  for(int i = startBlock; i <= endBlock; ++i)
  {
    set_bit(&bitmap1, &bitmap2, i);
  }

  char dirData[4];
  dirData[0] = data[0];  //Store file name
  dirData[1] = startBlock;
  dirData[2] = dataCount - 1; 

  for(int i = 0; i < DIRECTORYINDEX; ++i)
  {
    if(hard_disk[i] == EMPTY)
    {
      setDirStruct(dirData, hard_disk, i);
      allocated = true;
      break;
    }
  }
  ++totalFiles;
  printf("File %d added!\n", data[0]);
  return allocated;
}

//delete 
//read

bool ReadAllocation(int FileNum)
{
  printf("\nReading File %d...\n",FileNum);
  char dirData[4];
  int directorySlot = 0;
  //Get the directory information
  for (; directorySlot < DIRECTORYINDEX; ++directorySlot)
  {
      GetDirStruct(dirData, hard_disk, directorySlot);
      if (dirData[0] == FileNum) { break; }
      memset(dirData, 0, 4);
  }
  if (directorySlot == DIRECTORYINDEX) { return false; }

  printf("Contents: ");
  //Iterate through the blocks
  for(int i = 0; i < dirData[2]; ++i)
  {
    int elementNum = dirData[1] * 5;
    //Iterate through the elements
    //Print data
    printf("%d ", hard_disk[elementNum + i]);
  }
  printf("\n");
  return true;
}

bool DeleteAllocation(int FileNum)
{
  printf("\nDeleting file: %d...\n", FileNum);
  char dirData[4];
  int directorySlot = 0;
  //Iterate through file directory to find filename
  for (; directorySlot < DIRECTORYINDEX; ++directorySlot)
  {
    GetDirStruct(dirData, hard_disk, directorySlot);
    if (dirData[0] == FileNum) { break; }
    memset(dirData, 0, 4);
  }
  //If cannot find return false
  if (directorySlot == DIRECTORYINDEX) { return false; }

  //Iterate through the blocks
  for(int i = dirData[1]; i <= dirData[2]; ++i)
  {
    int elementNum = i * 5;
    int maxElements = elementNum + 5;
    //Clear the bits for that block
    clear_bit(&bitmap1, &bitmap2, i);
    //Iterate through the elements and set to 0
    for(;elementNum < maxElements; ++elementNum)
    {
      //If reach last element break
      if(hard_disk[elementNum] == -1)
      {
        hard_disk[elementNum] = 0;
        break;
      }
      hard_disk[elementNum] = 0;
    }
  }
  //Delete file directory
  hard_disk[directorySlot] = 0;
  --totalFiles;
  printf("File %d deleted!\n", FileNum);
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
        if(ContiguousAllocate(operations[i].Data, operations[i].DataCount)){
          
          PrintHardDisk();
        }else{
          printf("\nFile %d is unsuccessfully added!\n",operations[i].Data[0]);
        }
      }
      else if (strcmp(operations[i].op, "read") == 0)
      {
        ReadAllocation(operations[i].Data[0]);
      }
      else if (strcmp(operations[i].op, "delete") == 0)
      {
        if(DeleteAllocation(operations[i].Data[0])){
          
          PrintHardDisk();
        }else{
          printf("File %d is unsuccessfully deleted!\n",operations[i].Data[0]);
        }
      }
    }
 
    PrintHardDisk();

    for (int i = 0; i < opCount; ++i)
    {
        free(operations[i].op);
        free(operations[i].Data);
    }
    free(operations);

    return 0;
}