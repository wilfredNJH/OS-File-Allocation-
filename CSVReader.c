#include "CSVReader.h"

/*
typedef struct
  char* op; // name of operation
  int* Data;  // data inside
  int DataCount; // number of data in the operation
} Operation;
*/



void printOperation(Operation * oper)
{
  printf("Operation : %s \nDataCount : %d \n ", oper->op, oper->DataCount);
  int i =0;

  printf("Data :  ");
  while(i != oper->DataCount)
  {
    printf(" %d ", oper->Data[i]);
    ++i;
  }
  printf("\n ");

}


//getting the data for the line based on the number of the field 
const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

Operation* CSVReader(const char* filepath, int* opCount)
{
  *opCount = 0;
  FILE* fp;
  fp = fopen(filepath, "r");//open file for reading
      
  if(!fp) 
  {
    printf("File : %s not found " , filepath );
    return NULL;
  }

  char buffer[255];
  
  //getting number of operation, 
  //counting the numbers of operation in CSV file

  while (fgets(buffer, 255, fp))
  {
    getfield(buffer, 1);//getting the operation of the file 
    ++(*opCount);//incrementing the operation count 
  }
  
  //allocating enough memory to store entire CSV operation
  Operation* opArray = (Operation*)malloc(sizeof(Operation)*(*opCount));
  fseek(fp, 0, SEEK_SET); //pointing back the pointer to beginning of file
  Operation* tempOpPtr = opArray; // To increment

  //Loop to store each operation in the allocated memory
  int i = 0;
  
  // For each line(operation)
  while (fgets(buffer, 255, fp))
  {
    // Store into operation
    char tmpBuffer[255];
    strcpy(tmpBuffer, buffer);
    char* token = strtok(tmpBuffer, ",");
    //char* tmptoken = strtok(tmpBuffer, ",");
    
    
    // Count num of data
    opArray[i].DataCount = 0; // -1 as first value is the operation
    
    while (token)
    {
      ++opArray[i].DataCount;
      token = strtok(NULL, ",");
    }
    --opArray[i].DataCount;
    // printf("%d\n", opArray[i].DataCount);
    // Allocate space for data
    opArray[i].Data = (int*)malloc(sizeof(int)*(opArray[i].DataCount));
    strcpy(tmpBuffer, buffer);
    token = strtok(tmpBuffer, ",");
    // For each data(Filename and File contents)
    //int* dataIterator = opArray[i].Data;
    int j = 0;
    // printf("tmptoken: %s\n", tmptoken);
    while(token)
    {
      if(0 == j) 
      { 
        //opArray[i].op = token;
        opArray[i].op = (char*)malloc(strlen(token));
        strcpy(opArray[i].op, token);
      }
      else
        //tmptoken = strtok(NULL, ",");
      {
        opArray[i].Data[j-1] = atoi(token);
      }
      ++j;
        //tmptoken = strtok(NULL, ",");
      token = strtok(NULL, ",");
    }
    ++i;
    
  }
    //++opArray;
  
  
  
  return opArray;
}