#include <string.h>
#include <stdio.h>
#include <stdlib.h> // malloc

typedef struct
{
  char* op; // name of operation
  int* Data;  // data inside
  int DataCount; // number of data in the operation
} Operation;

Operation* CSVReader(const char* filepath, int* opCount);

void printOperation(Operation * op);

