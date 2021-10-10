#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <string.h> 
#include <unistd.h>
#include <winsock2.h>

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

void pushIntToMem(unsigned char *destination, int num)
{
    destination[0] = (num >> 24) & 0xFF;
    destination[1] = (num >> 16) & 0xFF;
    destination[2] = (num >> 8) & 0xFF;
    destination[3] = num & 0xFF;
}

int getIntFromMem(unsigned char *memPtr)
{
    return memPtr[3] + (memPtr[2] << 8) + (memPtr[1] << 16) + (memPtr[0] << 24);
}

void relocateInt(unsigned char *source, unsigned char *destination)
{
    pushIntToMem(destination, getIntFromMem(source));
}

void addIntValueToMem(unsigned char* source, int num)
{
    pushIntToMem(source, getIntFromMem(source)+num);
}

int popIntFromStack(char *memPtr, int *stackHead, int stackStart)
{
    *stackHead -= 4;
    if(*stackHead < stackStart)
    {
        printf("\nError:Stack Empty");
        exit(0);
    }
    return(getIntFromMem(&memPtr[*stackHead]));
}
/////////////////NEEDS TO GO////////////////////
void printStack(char *memPtr, int stackHead, int stackStart)
{
    printf("\n\nStack : StackHead(%d)", stackHead);
    while(stackHead >= stackStart)
    {
        printf("\t %d", htonl(getIntFromMem(&memPtr[stackHead])));
        stackHead -= 4;
    }
    printf("\n");
}








int main(int argc, char *argv[])
{
    unsigned char *memory = malloc(pow(2,32)/8);
    if(memory == NULL)
    {
        printf("Virtual Memory full\n");
        return(0);
    }

    // if(argc < 3)
    // {
    //     printf("Executable and/or constant_pool not given");
    //     return(0);
    // }

    int opt;
    while((opt = getopt(argc, argv, ":e:c:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'e':
                readFromFile(optarg, &memory[1024/8]); 
                break; 
            case 'c':
                readFromFile(optarg, &memory[256/8]); 
                break;  
            case ':': 
                printf("option needs a value\n"); 
                exit(0); 
            case '?': 
                printf("unknown option: %c\n", optopt);
                exit(0);
        } 
    }
    int callStackStart = 1048576/8;                       // Execution Stack Start
    int stackHead = callStackStart;                       // Execution Stack Head
    int pc = 1024/8;                                      // Program Counter
    int cp = 256/8;                                       // Constant Pool Cunter
    int currentOpStack = callStackStart + 40;             // Operand Stack
    pushIntToMem(&memory[stackHead], pc);

    while(stackHead >= callStackStart)
    {
        printf("\ncp %d", cp);
        if(getIntFromMem(&memory[stackHead]) == getIntFromMem(&memory[cp])/8)       // if pc == cp
        {
            if(cp == 256/8)
            {
                printf("\nLoading command line arg to memory");
                if(argc == 4)
                {
                    printf("\nlength %d", (int)strlen(argv[3]));
                    pushIntToMem(&memory[stackHead+8], atoi(argv[3]));  // Since argv[3] is char[1]
                }
            }
            else
            {
                for(int i = 0; i < memory[cp+4]; i++)           // memory[stackHead+4*i] is the localVariable array
                {
                    printf("\nLoading arg %d to memory", i);
                    relocateInt(memory+getIntFromMem(memory+stackHead)+4+i, &memory[stackHead+8+(4*i)]);
                }
            }
            for(int i = 0; i < memory[cp+4]; i++)           // memory[stackHead+4*i] is the localVariable array
            {
                printf("\t%d", getIntFromMem(&memory[stackHead+8+(4*i)]));
            }
        }
        // printf("\npc %d", pc);
        // printf("\nstackHead %d", stackHead);
        printf("\npc %d", getIntFromMem(&memory[stackHead]));
        unsigned char instr = *(memory+getIntFromMem(memory+stackHead));
        printf("\ninstr %02x", instr);
        switch(instr){
            case 0x10: // biPush(value)
                printf("\nBiPush");
                printf("\noperand %02x", memory[getIntFromMem(&memory[stackHead])+1]);
                pushIntToMem(&memory[currentOpStack], memory[getIntFromMem(&memory[stackHead])+1]);  // Push to opstack
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 2);  // Next instruction
                break;
            case 0x36: // iStore(index)
                printf("\niStore");
                currentOpStack -= 4;
                printf("\noperand %02x", memory[getIntFromMem(&memory[stackHead])+1]);
                relocateInt(&memory[currentOpStack], &memory[stackHead + 12 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]);
                // Pop from stack to lva
                printStack(memory, currentOpStack-1, stackHead+40);
                printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[stackHead])+1], getIntFromMem(&memory[stackHead + 12 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));
                addIntValueToMem(&memory[stackHead], 2);
                break;
            case 0x15: // iLoad(index)
                printf("\niLoad");
                printf("\noperand %02x", memory[getIntFromMem(&memory[stackHead])+1]);
                printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[stackHead])+1], getIntFromMem(&memory[stackHead + 12 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));
                pushIntToMem(&memory[currentOpStack], getIntFromMem(&memory[stackHead + 12 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));        // Load LV to stack
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 2);
                break;
            case 0x3: // iConst_0
                printf("\niconst0");
                currentOpStack += 4;
                pushIntToMem(&memory[currentOpStack], 0);
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x60: // iadd
                printf("\niadd");
                // push to stack (sum of stackhead.pop + stackhead.pop)
                printStack(memory, currentOpStack-1, stackHead+40);
                pushIntToMem(&memory[currentOpStack], popIntFromStack(memory, &currentOpStack, stackHead+40) + popIntFromStack(memory, &currentOpStack, stackHead+40));
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x64:
                printf("\nisub");
                // push to stack (dif of stackhead.pop + stackhead.pop)
                printStack(memory, currentOpStack-1, stackHead+40);
                pushIntToMem(&memory[currentOpStack], (popIntFromStack(memory, &currentOpStack, stackHead+40) - popIntFromStack(memory, &currentOpStack, stackHead+40)));
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x68:
                printf("\nimul");
                // push to stack (prod of stackhead.pop + stackhead.pop)
                printStack(memory, currentOpStack-1, stackHead+40);
                pushIntToMem(&memory[currentOpStack], popIntFromStack(memory, &currentOpStack, stackHead+40) * popIntFromStack(memory, &currentOpStack, stackHead+40));
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x6c:
                printf("\nidiv");
                // push to stack (quotient of stackhead.pop + stackhead.pop)
                printStack(memory, currentOpStack-1, stackHead+40);
                pushIntToMem(&memory[currentOpStack], (int)(((float)1 / popIntFromStack(memory, &currentOpStack, stackHead+40)) * popIntFromStack(memory, &currentOpStack, stackHead+40)));
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x7e:
                printf("\niand");
                // push to stack (bitwise and of stackhead.pop + stackhead.pop)
                printStack(memory, currentOpStack-1, stackHead+40);
                pushIntToMem(&memory[currentOpStack], (int)(popIntFromStack(memory, &currentOpStack, stackHead+40) & popIntFromStack(memory, &currentOpStack, stackHead+40)));
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x80:
                printf("\nior");
                // push to stack (bitwise and of stackhead.pop + stackhead.pop)
                printStack(memory, currentOpStack-1, stackHead+40);
                pushIntToMem(&memory[currentOpStack], (int)(popIntFromStack(memory, &currentOpStack, stackHead+40) | popIntFromStack(memory, &currentOpStack, stackHead+40)));
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x74:
                printf("\nineg");
                // push to stack (neg of stackhead.pop)
                printStack(memory, currentOpStack-1, stackHead+40);
                pushIntToMem(&memory[currentOpStack], -1*popIntFromStack(memory, &currentOpStack, stackHead+40));
                currentOpStack += 4;
                printStack(memory, currentOpStack-1, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x84:
                printf("\niinc"); // iinc(index, const)
                // make const signed int then add to val at index
                // read const to variable
                printf("\noperands %02x, %02x", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2]);
                printf("\nsign extended op2 %d", (signed int)memory[getIntFromMem(&memory[stackHead])+2]);
                pushIntToMem(&memory[stackHead + 12 + memory[cp+4]*4], getIntFromMem(&memory[stackHead + 12 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]) + (signed int)memory[getIntFromMem(&memory[stackHead])+2]);
                printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[stackHead])+1], getIntFromMem(&memory[stackHead + 12 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0xFF:
                printf("\nwrite");
                printStack(memory, currentOpStack-1, stackHead+40);
                printf("%d\n", popIntFromStack(memory, &currentOpStack, stackHead+40));
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x57:
                printf("\npop");
                printStack(memory, currentOpStack-1, stackHead+40);
                popIntFromStack(memory, &currentOpStack, stackHead+40);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0xb1:
                printf("\nreturn");
                printf("\nmemory[cp+6]%02x", memory[cp+6]);
                // if(memory[cp+6] != 0x02)
                // {
                //     printf("\nError!return type not void");
                //     exit(0);
                // }
                currentOpStack = stackHead;
                stackHead = getIntFromMem(&memory[stackHead + 4]);
                break;
            // case 
        }
        printf("\nNext instr\n");
        
        // cp += 7;
        char c; 
        scanf("%c",&c);
    }
}

