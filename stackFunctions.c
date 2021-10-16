#include "memFunctions.h"
#include "stackFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int popIntFromStack(char *memPtr, unsigned long *stackHead, unsigned long stackStart)
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
/////////////////NEEDS TO GO////////////////////
void printStack(unsigned char *memPtr, unsigned long stackHead, unsigned long stackStart)
{
    stackHead -= 4;
    // // printf("\nInt in stack as hex %02x %02x %02x %02x", memPtr[stackHead], memPtr[stackHead+1], memPtr[stackHead+2], memPtr[stackHead+3]);
    // printf("\n\nStack :");
    while(stackHead >= stackStart)
    {
        printf("[%d] | ", stackHead);
        printf("%d(%02x %02x %02x %02x)\t", getIntFromMem(&memPtr[stackHead]), memPtr[stackHead], memPtr[stackHead+1], memPtr[stackHead+2], memPtr[stackHead+3]);
        stackHead -= 4;
    }
    // printf("\n");
}

void printHeap(unsigned char *memPtr, unsigned long heapHead)
{
    heapHead += 1;
    // // printf("\nInt in stack as hex %02x %02x %02x %02x", memPtr[stackHead], memPtr[stackHead+1], memPtr[stackHead+2], memPtr[stackHead+3]);
    // printf("\n\nStack :");
    while(heapHead <= pow(2,32))
    {
        printf("[%d] | ", heapHead);
        printf("%d\t", getIntFromMem(&memPtr[heapHead]));
        heapHead += 4;
    }
    // printf("\n");
}