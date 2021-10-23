#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <unistd.h>
#include <limits.h>
#include "interpret.h"
#include "MemFunctions.h"
#define MEMORY_SIZE pow(2,32)/8 // 4294967296

void readFromFile(char *fileName, char *memPtr)
{
    FILE *f1 = fopen(fileName, "rb");
    if (f1)
    {
        fseek(f1, 0, SEEK_END);
        long filesize = ftell(f1);
        fseek(f1, 0 ,SEEK_SET);
        fread(memPtr, 1, filesize, f1);
    }
    else
    {
        printf("Could not read file\n");
        exit(0);
    }
    fclose(f1);
}

int main(int argc, char *argv[])
{
    size_t sz = MEMORY_SIZE;
    // printf("sizemax %u", sz);
    unsigned char *memory = malloc(sz);
    if(memory == NULL)
    {
        printf("Virtual Memory full\n");
        exit(0);
    }
    // Parsing command line arguments
    int opt;
    while((opt = getopt(argc, argv, ":e:c:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'e':
                readFromFile(optarg, &memory[1024]); 
                break; 
            case 'c':
                readFromFile(optarg, &memory[256]); 
                break;  
            case ':': 
                printf("Option needs a value\n"); 
                exit(0); 
            case '?': 
                printf("Unknown option: %c\n", optopt);
                exit(0);
        } 
    }
    int callStackStart = 1048576;                       // Execution Stack Start
    unsigned int stackHead = callStackStart;                     // Execution Stack Head
    unsigned int cp = 256;                                       // Constant Pool Cunter
    unsigned int currentOpStack = callStackStart + 72;           // Operand Stack
    unsigned int heapHead = MEMORY_SIZE;
    // printf("\nheapHead %u ", heapHead);
    pushIntToMem(&memory[stackHead], 1024);
    pushIntToMem(&memory[stackHead + 8 + memory[cp+4]*4], atoi(argv[optind]));  // Since argv[optind] is char[1]}


    while(stackHead >= callStackStart)
    {
        interpretInstructions(memory, &stackHead, &currentOpStack, &cp, &heapHead);
        printf("\n");
    }
}

