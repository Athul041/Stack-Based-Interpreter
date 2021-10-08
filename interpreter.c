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
// struct stackFrame{
//     int pc;
//     int *opStack;
//     int lva[16];                                                // Local Variable Array
// };

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

void pushIntToMem(char *destination, int num)
{
    memcpy(destination, (unsigned char *)&num, 4);
}

int getIntFromMem(char *memPtr)
{
    int num = __builtin_bswap32(atoi(memPtr));             // Check on linux for big endian
    return num;
}

void relocateInt(char *source, char *destination)
{
    int num = getIntFromMem(source);
    memcpy(destination,(unsigned char*)&num,4);
}

int popIntFromStack(char *memPtr)
{
    //TODO
    return 1;
}

void printStack(char *memPtr, int stackHead, int stackStart)
{
    printf("\n Stack -->");
    while(stackHead >= stackStart)
    {
        printf("\t %d", getIntFromMem(memPtr+stackHead));
        stackHead -= 4;
    }
}

int main(int argc, char *argv[])
{
    char *memory = malloc(pow(2,32)/8);
    if(memory == NULL)
    {
        printf("Virtual Memory full\n");
        return(0);
    }

    if(argc < 3)
    {
        printf("Executable and/or constant_pool not given");
        return(0);
    }
    readFromFile(argv[1], &memory[1024/8]);
    readFromFile(argv[2], &memory[256/8]);

    int callStackStart = 1048576/8;                       // Execution Stack Start
    int stackHead = callStackStart;                       // Execution Stack Head
    int pc = 1024/8;                                      // Program Counter
    int cp = 256/8;                                       // Constant Pool Cunter
    int currentOpStack = callStackStart + 40;             // Operand Stack

    // struct stackFrame sf;
    // sf.lva[0] = __builtin_bswap32(atoi(argv[3]));       // Check on linux for big endian
    while(stackHead >= callStackStart)
    {
        printf("\ncp %d", cp);
        if(cp == 256/8)
        {
            printf("\nLoading command line arg to memory");
            if(argc == 4)
            {
                relocateInt(argv[3], memory+callStackStart);
            }
        }
        else if(pc == *(int *) &memory[cp])
        {
            for(int i = 0; i < memory[cp+4]; i++)           // memory[stackHead+4*i] is the localVariable stack
            {
                printf("\nLoading arg %d to memory", i);
                memory[stackHead+8+(4*i)]=__builtin_bswap32(atoi(memory+getIntFromMem(memory+stackHead)+4+i));       // Check on linux for big endian
                printf("\n%d", memory[stackHead+8+(4*i)]);
            }
        }
        pushIntToMem(&memory[stackHead], pc);
        printf("\npc %d", pc);
        printf("\nmemory[stackHead] %d", getIntFromMem(&memory[stackHead]));
        char instr = *(memory+getIntFromMem(memory+stackHead));
        printf("\ninstr %02x", instr);
        switch(instr){
            case 0x10: // biPush(value)
                printf("\nBiPush");
                currentOpStack += 4;
                pushIntToMem(&memory[currentOpStack], memory[pc+1]);  // Push to opstack
                printStack(memory, currentOpStack, stackHead+40);
                memory[stackHead] += 2;         // Next instruction
            case 0x36: // iStore(index)
                printf("\niStore");
                relocateInt(&memory[currentOpStack], &memory[stackHead+memory[pc+1]]);
                // Pop from stack to lva
                printStack(memory, currentOpStack, stackHead+40);
                currentOpStack -= 4;
                pc += 2;
            case 0x15: // iLoad(index)
                printf("\niLoad");
                currentOpStack += 4;
                pushIntToMem(memory+currentOpStack, memory[stackHead+memory[pc+1]]);        // Load LV to stack
                printStack(memory, currentOpStack, stackHead+40);
                pc += 2;
            case 0x3: // iConst_0
                printf("\niconst0");
                currentOpStack += 4;
                memory[currentOpStack] = 0;
                printStack(memory, currentOpStack, stackHead+40);
                pc += 1;
            // case 0xa7: //GOTO(branchbyte1.branchbyte2) branchbyte1 << 8 | branchbyte2
            //     printf("\ngoto");
            //     pc = memory[pc+1] << 8 | memory[pc+1];
            //     pc += 2;
            // case 0:
            //     printf("\n0");
        }
        
        // cp += 7;
        char c; 
        scanf("%c",c);
    }
}

