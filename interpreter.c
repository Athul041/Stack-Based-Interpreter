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

void pushFrameToStack(struct stackFrame sf)
{
    // TODO: push to stack
}

int main(int argc, char *argv[])
{
    char *memPtr = malloc(pow(2,32)/8);
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
    readFromFile(argv[1], &memPtr[1024/8]);
    readFromFile(argv[2], &memPtr[256/8]);

    if(argc == 4)
    {
        int num = __builtin_bswap32(atoi(argv[3]));             // Check on linux for big endian
        memcpy(memPtr,(unsigned char*)&num,4);
    }

    int stackStart = 1048576 + 4*16;                    // Execution Stack Start
    int stackHead = stackStart;                         // Execution Stack Head
    int pc = 1024;                                      // Program Counter
    int cp = 256;                                       // Constant Pool Cunter

    struct stackFrame sf;
    sf.lva[0] = __builtin_bswap32(atoi(argv[3]));       // Check on linux for big endian
    while(stackHead >= stackStart)
    {
        printf("\nStackHead %d", stackHead);
        printf("\npc = %d", pc);
        printf("\ncp = %d", *(int *) &memPtr[cp]);
        if(pc == *(int *) &memPtr[cp])
        {
            // push to stack
                // stack.pc = *cp
                // stack.params.count = cp+4
                // stack.localVars[i] = *(pc+4*i) i<stack.params.count
            cp += 7;
            sf.pc = pc;
            printf("\nsf.pc %d", memPtr[sf.pc]);
            for(int i = 0; i < memPtr[cp+4]; i++)
            {
                sf.lva[i]=__builtin_bswap32(atoi(memPtr+cp+4+i));       // Check on linux for big endian
            }
            printf("\nparams");
            for(int i = 0; i < memPtr[cp+4]; i++)
            {
                printf("\n%d", sf.lva[i]);
            }
            pushFrameToStack(sf);
        }
        printf("\nHere %08x", memPtr[sf.pc]);
        char instr = memPtr[sf.pc];
        printf("\ninstr %x", instr);
        switch(instr){
            case 0x10: // biPush(value)
                printf("\nBiPush");
                *sf.opStack = (memPtr[sf.pc+1]);  // Push to op stack
                sf.pc += 2;                 // Next instruction
            case 0x36: // iStore(index)
                printf("\niStore");
                sf.lva[memPtr[sf.pc+1]] = *sf.opStack; // Pop from stack to lva
                sf.opStack -= 1;
                sf.pc += 2;
            case 0x15: // iLoad(index)
                printf("\niLoad");
                *(sf.opStack+1) = sf.lva[memPtr[sf.pc+1]];
                sf.pc += 2;
            case 0x3: // iConst_0
                printf("\niconst0");
                sf.opStack += 1;
                *sf.opStack = 0;
                sf.pc += 1;
            case 0xa7: //GOTO(branchbyte1.branchbyte2) branchbyte1 << 8 | branchbyte2
                printf("\ngoto");
                sf.pc = memPtr[sf.pc+1] << 8 | memPtr[sf.pc+1];
                sf.pc += 2;
            case 0:

        }
        char c;
        scanf("%c",c);
    }
}

