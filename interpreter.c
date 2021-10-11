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

void pushIntToMem(unsigned char *destination, signed int num)
{
    // printf("\nPushing %d to memory", num);
    // // printf("\nInt as hex %02x %02x %02x %02x", num, (num >> 8), (num >> 16), (num >> 24));
    destination[3] = (num >> 24);
    destination[2] = (num >> 16);
    destination[1] = (num >> 8);
    destination[0] = num;
    // // printf("\nInt after push %02x %02x %02x %02x", destination[0], destination[1], destination[2], destination[3]);
}

signed int getIntFromMem(unsigned char *memPtr)
{
    signed int num = (signed int)memPtr[0] | (memPtr[1] << 8) | (memPtr[2] << 16) | (memPtr[3] << 24);
    return num;
}
signed int getIntFromLoadedMem(unsigned char *memPtr)
{
    signed int num = (signed int)memPtr[3] | (memPtr[2] << 8) | (memPtr[1] << 16) | (memPtr[0] << 24);
    return num;
}

void relocateInt(unsigned char *source, unsigned char *destination)
{
    pushIntToMem(destination, getIntFromMem(source));
}

void addIntValueToMem(unsigned char* source, signed int num)
{
    pushIntToMem(source, getIntFromMem(source)+num);
}

int popIntFromStack(char *memPtr, int *stackHead, int stackStart)
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
void printStack(unsigned char *memPtr, int stackHead, int stackStart)
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








int main(int argc, char *argv[])
{
    unsigned char *memory = malloc(pow(2,32)/8);
    if(memory == NULL)
    {
        printf("Virtual Memory full\n");
        return(0);
    }
    // Parsing command line arguments
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
                printf("Option needs a value\n"); 
                exit(0); 
            case '?': 
                printf("Unknown option: %c\n", optopt);
                exit(0);
        } 
    }
    int callStackStart = 1048576/8;                       // Execution Stack Start
    int stackHead = callStackStart;                       // Execution Stack Head
    int cp = 256/8;                                       // Constant Pool Cunter
    int currentOpStack = callStackStart + 72;             // Operand Stack
    pushIntToMem(&memory[stackHead], 1024/8);
    // printf("\nLoading command line arg to memory");
    pushIntToMem(&memory[stackHead + 8 + memory[cp+4]*4], atoi(argv[optind]));  // Since argv[optind] is char[1]}
    int val = 0;

    while(stackHead >= callStackStart)
    {
        // printf("\ncp %d", cp);
        // // printf("\nstackHead %d", stackHead);
        // printf("\npc %d", getIntFromMem(&memory[stackHead]));
        unsigned char instr = memory[getIntFromMem(&memory[stackHead])];
        // printf("\ninstr %02x", instr);
        switch(instr){
            case 0x10: // biPush(value)
                // printf("\nBiPush");
                // printf("\noperand %02x", memory[getIntFromMem(&memory[stackHead])+1]);
                pushIntToMem(&memory[currentOpStack], (int32_t)((int8_t)memory[getIntFromMem(&memory[stackHead])+1]));  // Push to opstack
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 2);  // Next instruction
                break;
            case 0x36: // iStore(index)
                // printf("\niStore");
                // printf("\noperand %02x", memory[getIntFromMem(&memory[stackHead])+1]);
                pushIntToMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4], popIntFromStack(memory, &currentOpStack, stackHead+72));
                // Pop from stack to lva
                // printStack(memory, currentOpStack, stackHead+72);
                // printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[stackHead])+1], getIntFromMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));
                addIntValueToMem(&memory[stackHead], 2);
                break;
            case 0x15: // iLoad(index)
                // printf("\niLoad");
                // printf("\noperand %02x", memory[getIntFromMem(&memory[stackHead])+1]);
                // printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[stackHead])+1], getIntFromMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));
                pushIntToMem(&memory[currentOpStack], getIntFromMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));        // Load LV to stack
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 2);
                break;
            case 0x2e: // iaload
                // printf("\niaload");
                // pop arrayref, index from stack, push value
                pushIntToMem(&memory[currentOpStack], getIntFromLoadedMem(&memory[popIntFromStack(memory, &currentOpStack, stackHead+72) + popIntFromStack(memory, &currentOpStack, stackHead+72)/8]));
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break; 
            case 0x4f: // iastore
                // printf("\niastore");
                // pop arrayref, index, value from stack, set val to arrayref/8 + index
                val = popIntFromStack(memory, &currentOpStack, stackHead+72);
                pushIntToMem(&memory[popIntFromStack(memory, &currentOpStack, stackHead+72) + popIntFromStack(memory, &currentOpStack, stackHead+72)/8], val);
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;   
            case 0x3: // iConst_0
                // printf("\niconst0");
                pushIntToMem(&memory[currentOpStack], 0);
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x60: // iadd
                // printf("\niadd");
                // push to stack (sum of stackhead.pop + stackhead.pop)
                // printStack(memory, currentOpStack, stackHead+72);
                pushIntToMem(&memory[currentOpStack], popIntFromStack(memory, &currentOpStack, stackHead+72) + popIntFromStack(memory, &currentOpStack, stackHead+72));
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x64:
                // printf("\nisub");
                // push to stack (dif of stackhead.pop + stackhead.pop)
                // printStack(memory, currentOpStack, stackHead+72);
                val = popIntFromStack(memory, &currentOpStack, stackHead+72);
                pushIntToMem(&memory[currentOpStack], (popIntFromStack(memory, &currentOpStack, stackHead+72) - val));
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x68:
                // printf("\nimul");
                // push to stack (prod of stackhead.pop + stackhead.pop)
                // printStack(memory, currentOpStack, stackHead+72);
                pushIntToMem(&memory[currentOpStack], popIntFromStack(memory, &currentOpStack, stackHead+72) * popIntFromStack(memory, &currentOpStack, stackHead+72));
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x6c:
                // printf("\nidiv");
                // push to stack (quotient of stackhead.pop + stackhead.pop)
                // printStack(memory, currentOpStack, stackHead+72);
                pushIntToMem(&memory[currentOpStack], (int)(((float)1 / popIntFromStack(memory, &currentOpStack, stackHead+72)) * popIntFromStack(memory, &currentOpStack, stackHead+72)));
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x7e:
                // printf("\niand");
                // push to stack (bitwise and of stackhead.pop + stackhead.pop)
                // printStack(memory, currentOpStack, stackHead+72);
                pushIntToMem(&memory[currentOpStack], (int)(popIntFromStack(memory, &currentOpStack, stackHead+72) & popIntFromStack(memory, &currentOpStack, stackHead+72)));
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x80:
                // printf("\nior");
                // push to stack (bitwise and of stackhead.pop + stackhead.pop)
                // printStack(memory, currentOpStack, stackHead+72);
                pushIntToMem(&memory[currentOpStack], (int)(popIntFromStack(memory, &currentOpStack, stackHead+72) | popIntFromStack(memory, &currentOpStack, stackHead+72)));
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x74:
                // printf("\nineg");
                // push to stack (neg of stackhead.pop)
                // printStack(memory, currentOpStack, stackHead+72);
                val = popIntFromStack(memory, &currentOpStack, stackHead+72);
                pushIntToMem(&memory[currentOpStack], -val);
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x84:
                // printf("\niinc"); // iinc(index, const)
                // make const signed int then add to val at index
                // read const to variable
                // printf("\noperands %02x, %02x", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2]);
                // printf("\nsign extended op2 %d", (int32_t)((int8_t)memory[getIntFromMem(&memory[stackHead])+2]));
                // printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[stackHead])+1], getIntFromMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));
                pushIntToMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4], getIntFromMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]) 
                            + (int32_t)((int8_t)memory[getIntFromMem(&memory[stackHead])+2]));
                // printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[stackHead])+1], getIntFromMem(&memory[stackHead + 8 + memory[cp+4]*4 + memory[getIntFromMem(&memory[stackHead])+1]*4]));
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0x9f:
                // printf("\nif_icmpeq"); // if_icmpeq(branchbyte1, branchbyte2)
                // if ints are equal, branch to instruction at branchoffset (signed short constructed 
                // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
                if(popIntFromStack(memory, &currentOpStack, stackHead+72) == popIntFromStack(memory, &currentOpStack, stackHead+72))
                {
                    // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2], 
                                    // (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    break;
                }
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0x99:
                // printf("\nifeq"); // ifeq(branchbyte1, branchbyte2)
                // if value is zero, branch to instruction at branchoffset (signed short constructed 
                // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
                if(popIntFromStack(memory, &currentOpStack, stackHead+72) == 0)
                {
                    // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2], 
                                    // (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    break;
                }
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0x9b:
                // printf("\niflt"); // iflt(branchbyte1, branchbyte2)
                // if value is less than zero, branch to instruction at branchoffset (signed short constructed 
                // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
                if(popIntFromStack(memory, &currentOpStack, stackHead+72) < 0)
                {
                    // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2], 
                                    // (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    break;
                }
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0x9c:
                // printf("\nifge"); // ifge(branchbyte1, branchbyte2)
                // if value is greater than or equal to zero, branch to instruction at branchoffset (signed short constructed 
                // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
                if(popIntFromStack(memory, &currentOpStack, stackHead+72) >= 0)
                {
                    // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2], 
                                    // (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    break;
                }
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0x9d:
                // printf("\nifgt"); // ifgt(branchbyte1, branchbyte2)
                // if value is greater than zero, branch to instruction at branchoffset (signed short constructed 
                // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
                if(popIntFromStack(memory, &currentOpStack, stackHead+72) > 0)
                {
                    // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2], 
                                    // (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    break;
                }
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0x9e:
                // printf("\nifle"); // ifle(branchbyte1, branchbyte2)
                // if value is less than or equal to zero, branch to instruction at branchoffset (signed short constructed 
                // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
                if(popIntFromStack(memory, &currentOpStack, stackHead+72) <= 0)
                {
                    // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2], 
                                    // (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    break;
                }
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0xc6:
                // printf("\nifnull"); // ifnull(branchbyte1, branchbyte2)
                // if value is null, branch to instruction at branchoffset (signed short constructed 
                // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
                if(popIntFromStack(memory, &currentOpStack, stackHead+72))
                {
                    // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[stackHead])+1], memory[getIntFromMem(&memory[stackHead])+2], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                    break;
                }
                addIntValueToMem(&memory[stackHead], 3);
                break;
            case 0xff:
                // printf("\nwrite");
                // printStack(memory, currentOpStack, stackHead+72);
                printf("%d\n", popIntFromStack(memory, &currentOpStack, stackHead+72));
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0x57:
                // printf("\npop");
                // printStack(memory, currentOpStack, stackHead+72);
                popIntFromStack(memory, &currentOpStack, stackHead+72);
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0xb1:
                // printf("\nreturn");
                // // printf("\nmemory[cp+6]%02x", memory[cp+6]);
                // if(memory[cp+6] != 0x02)
                // {
                //     // printf("\nError!return type not void");
                //     exit(0);
                // }
                currentOpStack = stackHead;
                stackHead = getIntFromMem(&memory[stackHead + 4]);
                break;
            case 0xa7:
                // printf("\ngoto"); // goto(branchbyte1, branchbyte2 )
                //goto offset
                // from unsigned bytes branchbyte1 << 8 | branchbyte2)
                addIntValueToMem(&memory[stackHead], (int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]));
                break;
            case 0xac:
                // printf("\nireturn");
                // int popped from opstack of current frame
                // to opstack of previous frame
                
                // // printf("\ncurrent Stackframe");
                // // printf("\n stackHead %d", stackHead);
                // // printf("\n currentOpstack %d", currentOpStack);
                // // printStack(memory, currentOpStack, stackHead+72);
                // // printf("\n SP %d", getIntFromMem(&memory[stackHead + 4]));

                pushIntToMem(&memory[stackHead], popIntFromStack(memory, &currentOpStack, stackHead+72));
                currentOpStack = stackHead + 4;
                stackHead = getIntFromMem(&memory[stackHead + 4]);
                // // printf("\ngetIntFromMem(&memory[stackHead + 4]) %d", getIntFromMem(&memory[stackHead + 4]));

                // // printf("\nOld Stackframe");
                // // printf("\n stackHead %d", stackHead);
                // // printf("\n currentOpstack %d", currentOpStack);
                // // printStack(memory, currentOpStack, stackHead+72);
                // // printf("\n SP %d", getIntFromMem(&memory[stackHead + 4]));
                break;
            case 0xFE:
                // printf("\niread"); // iread
                // printf("\nInput integer value :");
                scanf("%d", &val);
                pushIntToMem(&memory[currentOpStack], val);
                // printStack(memory, currentOpStack, stackHead+72);
                currentOpStack += 4;
                addIntValueToMem(&memory[stackHead], 1);
                break;
            case 0xb8:
                // printf("\ninvokestatic"); // invokestatic(indexbyte,indexbyte)
                // (indexbyte1 << 8) | indexbyte2 gives index of constant pool
                // goto that method with new stackframe
                // pop no. of args times from old opstack and load to lva
                // set pc, stackhead, currentopstack
                cp = 256/8 + 7*(int16_t)(memory[getIntFromMem(&memory[stackHead])+1] << 8 | memory[getIntFromMem(&memory[stackHead])+2]);
                int arr[memory[cp+4]];
                // printStack(memory, currentOpStack, stackHead+72);
                for(int i = 0; i < memory[cp+4]; i++)
                {
                    arr[i] = popIntFromStack(memory, &currentOpStack, stackHead+72);
                    // // printf("\narr[%d] %d", i, arr[i]);
                }
                pushIntToMem(&memory[currentOpStack+4], stackHead);
                addIntValueToMem(&memory[stackHead], 3); // incrementing pc before changing stackhead
                stackHead = currentOpStack;
                for(int i = memory[cp+4]-1; i >= 0; i--)
                {
                    // // printf("\npushing %d", arr[i]);
                    pushIntToMem(&memory[stackHead + 8 + memory[cp+4]*4 + (4*i)], arr[i]);
                }
                currentOpStack = stackHead + 72;
                pushIntToMem(&memory[stackHead], (1024 + 8*(getIntFromLoadedMem(&memory[cp]) - 1024))/8);
                break;
        }
        // // printf("\nNext instr\n");
    }
}

