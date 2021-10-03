#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>

char * getOpCode(char *instruction){
    switch(*instruction){
        case 0x10:
            return("BIPUSH");
    }
}
struct stackFrame{
    int pc;
    int *opStack;
    int lva[16];                                                // Local Variable Array
};

void readFromFile(char *fileName, char *memPtr)
{
    FILE *f1 = fopen(fileName, "rb");
    if (f1)
    {
        fseek(f1, 0, SEEK_END);
        long filesize = ftell(f1);
        fseek(f1, 0 ,SEEK_SET);
        fread(memPtr, 1, filesize, f1); 
        fclose(f1);
    }    
    else
    {
        printf("Could not read file\n");
        return;
    }
    fclose(f1);
}

void pushIntToStack(char *memPtr, int num)
{
    memcpy(memPtr, (unsigned char *)&num, 4);
}

void copyIntFromMemtoMem(char *source, char *destination)
{
    int num = __builtin_bswap32(atoi(source));             // Check on linux for big endian
    memcpy(destination,(unsigned char*)&num,4);
}

void popIntFromStack(char *memPtr, char *stack)
{
    for(int i=0; i<4;i++)
    {
        *(memPtr+i)
    }
}

int main(int argc, char *argv[])
{
    char *memPtr = malloc(pow(2,30));                        // not enough ram? pow(2,32)
    if(memPtr == NULL)
    {
        printf("Virtual Memory full\n");
        return(0);
    }

    if(argc < 3)
    {
        printf("Executable and/or constant_pool not given");
        return(0);
    }
    readFromFile(argv[1], &memPtr[1024]);
    readFromFile(argv[2], &memPtr[256]);

    int callStackStart = 1048576 + 4*16;                // Execution Stack Start
    int stackHead = callStackStart;                     // Execution Stack Head
    int pc = 1024;                                      // Program Counter
    int cp = 256;                                       // Constant Pool Cunter
    int opStackHead = 1048576-1;                        // Operand Stack 4*16 bits
    int stackFrameSize = 17;

    struct stackFrame sf;
    sf.lva[0] = __builtin_bswap32(atoi(argv[3]));       // Check on linux for big endian
    while(stackHead >= callStackStart)
    {
        printf("\ncp %d", cp);
        if(cp == 256)
        {
            printf("\nLoading arg to memory");
            if(argc == 4)
            {
                copyIntFromMemtoMem(argv[3], memPtr+callStackStart);
            }
        }
        else if(pc == *(int *) &memPtr[cp])
        {
            printf("\nLoading 1st arg to memory");
            for(int i = 0; i < memPtr[cp+4]; i++)           // memPtr[stackHead+4*i] is the localVariable stack
            {
                memPtr[stackHead+4*i]=__builtin_bswap32(atoi(memPtr+cp+4+i));       // Check on linux for big endian
            }
            printf("\nparams");
            for(int i = 0; i < memPtr[cp+4]; i++)
            {
                printf("\n%08x", *(memPtr+stackHead+4*i));
            }
        }
        cp += 56;
        // opstack = memPtr[opStackHead]
        // 
        memPtr[stackHead] = pc;
        printf("\npc %d", pc);
        // printf("\nmemPtr[stackHead] %d", __builtin_bswap32(atoi(memPtr+stackHead)));
        // printf("\nstackHead %d", stackHead);
        char instr = *(memPtr+pc);
        printf("\ninstr %02x", instr);
        switch(instr){
            case 0x10: // biPush(value)
                printf("\nBiPush");
                opStackHead += 4;
                pushIntToStack(&memPtr[opStackHead], memPtr[pc+1]);  // Push to opstack
                pc += 2;                 // Next instruction
            case 0x36: // iStore(index)
                printf("\niStore");
                copyIntFromMemtoMem(&memPtr[opStackHead], &memPtr[stackHead+memPtr[pc+1]]);
                // Pop from stack to lva
                opStackHead -= 4;
                pc += 2;
            case 0x15: // iLoad(index)
                printf("\niLoad");
                opStackHead += 4;
                pushIntToStack(memPtr[opStackHead], memPtr[stackHead+memPtr[pc+1]]);        // Load LV to stack
                pc += 2;
            case 0x3: // iConst_0
                printf("\niconst0");
                opStackHead += 4;
                memPtr[opStackHead] = 0;
                pc += 1;
            // case 0xa7: //GOTO(branchbyte1.branchbyte2) branchbyte1 << 8 | branchbyte2
            //     printf("\ngoto");
            //     pc = memPtr[pc+1] << 8 | memPtr[pc+1];
            //     pc += 2;
            // case 0:
            //     printf("\n0");
        }
        char c; 
        scanf("%c",c);
    }
}

