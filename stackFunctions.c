#include "memFunctions.h"
#include "stackFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define MEMORY_SIZE pow(2,32)/8 // 4294967296

int popIntFromStack(unsigned char *memPtr, unsigned int *stackHead, unsigned int stackStart)
{
    *stackHead -= 4;
    // // printf("\nstackhead %d", *stackHead);
    // // printf("\nstackStart %d", stackStart);
    if(*stackHead < stackStart)
    {
        // printf("\nError:Stack Empty");
        exit(0);
    }
    // // printf("\nInt in stack as hex %02x %02x %02x %02x", memPtr[*stackHead], memPtr[*stackHead+1], memPtr[*stackHead+2], memPtr[*stackHead+3]);
    return(getIntFromMem(&memPtr[*stackHead]));
}
unsigned int popRefFromStack(unsigned char *memPtr, unsigned int *stackHead, unsigned int stackStart)
{
    *stackHead -= 4;
    // printf("\nstackhead %d", *stackHead);
    // // printf("\nstackStart %d", stackStart);
    if(*stackHead < stackStart)
    {
        // printf("\nError:Stack Empty");
        exit(0);
    }
    // // printf("\nInt in stack as hex %02x %02x %02x %02x", memPtr[*stackHead], memPtr[*stackHead+1], memPtr[*stackHead+2], memPtr[*stackHead+3]);
    return(getRefFromMem(&memPtr[*stackHead]));
}
/////////////////NEEDS TO GO////////////////////
void printStack(unsigned char *memPtr, unsigned int stackHead, unsigned int stackStart)
{
    // stackHead -= 4;
    printf("\nstackhead %d", stackHead);
    // // printf("\nInt in stack as hex %02x %02x %02x %02x", memPtr[stackHead], memPtr[stackHead+1], memPtr[stackHead+2], memPtr[stackHead+3]);
    printf("\n\nStack :");
    while(stackHead >= stackStart)
    {
        printf("[%d]=>", stackHead);
        printf("%d(%u)\t", getIntFromMem(&memPtr[stackHead]), getRefFromMem(&memPtr[stackHead]));
        stackHead -= 4;
    }
    printf("\n");
}

void printHeap(unsigned char *memPtr, unsigned int heapHead)
{
    // heapHead += 1;
    // // printf("\nInt in stack as hex %02x %02x %02x %02x", memPtr[stackHead], memPtr[stackHead+1], memPtr[stackHead+2], memPtr[stackHead+3]);
    printf("\n\nHeap :");
    while(heapHead < MEMORY_SIZE)
    {
        printf("[%u]=>", heapHead);
        printf("%d\t", getIntFromMem(&memPtr[heapHead]));
        heapHead += 4;
    }
    // printf("\n");
}