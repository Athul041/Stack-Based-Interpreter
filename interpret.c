#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <dlfcn.h>
#include "MemFunctions.h"
#include "stackFunctions.h"
#include "interpret.h"

void interpretInstructions(unsigned char *memory, unsigned int *stackHead, unsigned int *currentOpStack, unsigned int *cp, unsigned int *heapHead, int threshold, methods *methods)
{
    unsigned char instr = memory[getIntFromMem(&memory[*stackHead])];
    int val = 0;
    int i = 0;
    int arr[16];
    int index;
    int (*func_ptr)();
    char *fun = malloc(sizeof(char)*256);
    char s;
    switch(instr){
        case 0x10: // biPush(value)
            // printf("\nBiPush");
            // printf("\noperand %02x", memory[getIntFromMem(&memory[*stackHead])+1]);
            pushIntToMem(&memory[*currentOpStack], (int32_t)((int8_t)memory[getIntFromMem(&memory[*stackHead])+1]));  // Push to opstack
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 2);  // Next instruction
            break;
        case 0x36: // iStore(index)
            // printf("\niStore");
            pushIntToMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4], popIntFromStack(memory, currentOpStack, *stackHead+72));
            // Pop from stack to lva
            appendPC(&memory[*stackHead], 2);
            break;
        case 0x15: // iLoad(index)
            // printf("\niLoad");
            pushIntToMem(&memory[*currentOpStack], getIntFromMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4]));        // Load LV to stack
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 2);
            break;
        case 0x2e: // iaload
            // printf("\niaload");
            // pop arrayref, index from stack, push value
            pushIntToMem(&memory[*currentOpStack], getIntFromMem(&memory[4*popIntFromStack(memory, currentOpStack, *stackHead+72) + popRefFromStack(memory, currentOpStack, *stackHead+72)]));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break; 
        case 0x4f: // iastore
            // printf("\niastore");
            // pop arrayref, index, value from stack, set val to arrayref + index
            val = popIntFromStack(memory, currentOpStack, *stackHead+72);
            pushIntToMem(&memory[4*popIntFromStack(memory, currentOpStack, *stackHead+72) + popRefFromStack(memory, currentOpStack, *stackHead+72)], val);
            appendPC(&memory[*stackHead], 1);
            break;   
        case 0x3: // iConst_0
            // printf("\niconst0");
            pushIntToMem(&memory[*currentOpStack], 0);
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x60: // iadd
            // printf("\niadd");
            // push to stack (sum of *stackHead.pop + *stackHead.pop)
            pushIntToMem(&memory[*currentOpStack], popIntFromStack(memory, currentOpStack, *stackHead+72) + popIntFromStack(memory, currentOpStack, *stackHead+72));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x64:
            // printf("\nisub");
            // push to stack (dif of *stackHead.pop + *stackHead.pop)
            val = popIntFromStack(memory, currentOpStack, *stackHead+72);
            pushIntToMem(&memory[*currentOpStack], (popIntFromStack(memory, currentOpStack, *stackHead+72) - val));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x68:
            // printf("\nimul");
            // push to stack (prod of *stackHead.pop + *stackHead.pop)
            pushIntToMem(&memory[*currentOpStack], popIntFromStack(memory, currentOpStack, *stackHead+72) * popIntFromStack(memory, currentOpStack, *stackHead+72));
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x6c:
            // printf("\nidiv");
            // push to stack (quotient of *stackHead.pop + *stackHead.pop)
            pushIntToMem(&memory[*currentOpStack], (int)(((float)1 / popIntFromStack(memory, currentOpStack, *stackHead+72)) * popIntFromStack(memory, currentOpStack, *stackHead+72)));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x7e:
            // printf("\niand");
            // push to stack (bitwise and of *stackHead.pop + *stackHead.pop)
            // printStack(memory, *currentOpStack, *stackHead+72);
            pushIntToMem(&memory[*currentOpStack], (int)(popIntFromStack(memory, currentOpStack, *stackHead+72) & popIntFromStack(memory, currentOpStack, *stackHead+72)));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x80:
            // printf("\nior");
            // push to stack (bitwise and of *stackHead.pop + *stackHead.pop)
            // printStack(memory, *currentOpStack, *stackHead+72);
            pushIntToMem(&memory[*currentOpStack], (int)(popIntFromStack(memory, currentOpStack, *stackHead+72) | popIntFromStack(memory, currentOpStack, *stackHead+72)));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x74:
            // printf("\nineg");
            // push to stack (neg of *stackHead.pop)
            // printStack(memory, *currentOpStack, *stackHead+72);
            val = popIntFromStack(memory, currentOpStack, *stackHead+72);
            pushIntToMem(&memory[*currentOpStack], -val);
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x84:
            // printf("\niinc"); // iinc(index, const)
            // make const signed int then add to val at index
            // read const to variable
            pushIntToMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4], getIntFromMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4]) 
                        + (int32_t)((int8_t)memory[getIntFromMem(&memory[*stackHead])+2]));
            // printf("\nLocal variable at index#%d = %d", memory[getIntFromMem(&memory[*stackHead])+1], getIntFromMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4]));
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x9f:
            // printf("\nif_icmpeq"); // if_icmpeq(branchbyte1, branchbyte2)
            // if ints are equal, branch to instruction at branchoffset (signed short constructed 
            // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
            if(popIntFromStack(memory, currentOpStack, *stackHead+72) == popIntFromStack(memory, currentOpStack, *stackHead+72))
            {
                // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[*stackHead])+1], memory[getIntFromMem(&memory[*stackHead])+2], 
                                // (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                break;
            }
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x99:
            // printf("\nifeq"); // ifeq(branchbyte1, branchbyte2)
            // if value is zero, branch to instruction at branchoffset (signed short constructed 
            // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
            if(popIntFromStack(memory, currentOpStack, *stackHead+72) == 0)
            {
                // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[*stackHead])+1], memory[getIntFromMem(&memory[*stackHead])+2], 
                                // (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                break;
            }
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x9b:
            // printf("\niflt"); // iflt(branchbyte1, branchbyte2)
            // if value is less than zero, branch to instruction at branchoffset (signed short constructed 
            // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
            if(popIntFromStack(memory, currentOpStack, *stackHead+72) < 0)
            {
                // printf("\nbranchbyte1 %02x, branchbyte2 %02x, offset %d", memory[getIntFromMem(&memory[*stackHead])+1], memory[getIntFromMem(&memory[*stackHead])+2], 
                                // (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                break;
            }
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x9c:
            // printf("\nifge"); // ifge(branchbyte1, branchbyte2)
            // if value is greater than or equal to zero, branch to instruction at branchoffset (signed short constructed 
            // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
            if(popIntFromStack(memory, currentOpStack, *stackHead+72) >= 0)
            {
                appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                break;
            }
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x9d:
            // printf("\nifgt"); // ifgt(branchbyte1, branchbyte2)
            // if value is greater than zero, branch to instruction at branchoffset (signed short constructed 
            // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
            if(popIntFromStack(memory, currentOpStack, *stackHead+72) > 0)
            {
                appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                break;
            }
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x9e:
            // printf("\nifle"); // ifle(branchbyte1, branchbyte2)
            // if value is less than or equal to zero, branch to instruction at branchoffset (signed short constructed 
            // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
            if(popIntFromStack(memory, currentOpStack, *stackHead+72) <= 0)
            {
                appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                break;
            }
            appendPC(&memory[*stackHead], 3);
            break;
        case 0xc6:
            // printf("\nifnull"); // ifnull(branchbyte1, branchbyte2)
            // if value is null, branch to instruction at branchoffset (signed short constructed 
            // from unsigned bytes branchbyte1 << 8 | branchbyte2) 
            if(!popIntFromStack(memory, currentOpStack, *stackHead+72))
            {
                appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
                break;
            }
            appendPC(&memory[*stackHead], 3);
            break;
        case 0xff:
            // printf("\nwrite");
             printf("%d\n", popIntFromStack(memory, currentOpStack, *stackHead+72));
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x57:
            // printf("\npop");
            popIntFromStack(memory, currentOpStack, *stackHead+72);
            appendPC(&memory[*stackHead], 1);
            break;
        case 0xb1:
            // printf("\nreturn");
            *currentOpStack = *stackHead;
            *stackHead = getIntFromMem(&memory[*stackHead + 4]);
            break;
        case 0xa7:
            // printf("\ngoto"); // goto(branchbyte1, branchbyte2 )
            //goto offset
            // from unsigned bytes branchbyte1 << 8 | branchbyte2)
            appendPC(&memory[*stackHead], (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]));
            break;
        case 0xac:
            // printf("\nireturn");
            pushIntToMem(&memory[*stackHead], popIntFromStack(memory, currentOpStack, *stackHead+72));
            *currentOpStack = *stackHead + 4;
            *stackHead = getIntFromMem(&memory[*stackHead + 4]);
            break;
        case 0xFE:
            // printf("\niread"); // iread
            scanf("%d", &val);
            pushIntToMem(&memory[*currentOpStack], val);
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0xb8:
            // printf("\ninvokestatic"); // invokestatic(indexbyte,indexbyte)
            index = (int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]);
            *cp = 256 + 7*index;
            for(i = 0; i < memory[*cp+4]; i++)
            {
                arr[i] = popIntFromStack(memory, currentOpStack, *stackHead+72);
            }
            // check for AOT 
            if(methods->methods[index].init != 1)
            {
                addNewMethod(methods, index);
                // printf("\nMethod added");
                // printMethods(methods);
            }
            if(methods->methods[index].interpretedCount >= threshold)
            {
                if(methods->methods[index].isCompiled == -1)
                {
                    snprintf(fun, 10, "func_%d", index);
                    func_ptr =  dlsym(methods->fileHandle, fun);
                    if (func_ptr == NULL)
                    {
                        fprintf(stderr, "dlopen failed: %s\n", dlerror());
                        methods->methods[index].isCompiled = 0;
                    }
                    else
                    {
                        methods->methods[index].isCompiled = 1;
                    }
                }
                if(methods->methods[index].isCompiled == 1)
                {
                    // printf("\nExecuting compiled method");
                    snprintf(fun, 25, "func_%d", index);
                    func_ptr =  dlsym(methods->fileHandle, fun);
                    
                    // TODO: Find a better way to unpack array to function call
                    switch ((int32_t)memory[*cp+4])
                    {
                        case 0:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr();
                            }
                            else
                            {
                                val = func_ptr();
                            }
                            break;
                        case 1:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr(arr[memory[*cp+4]-1-0]);
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0]);
                            }
                            break;
                        case 2:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1]);
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1]);
                            }
                            break;
                        case 3:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2]);
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2]);
                            }
                            break;
                        case 4:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3]);
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-4]);
                            }
                            break;
                        case 5:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4]);
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4]);
                            }
                            break;
                        case 6:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                if((memory[*cp+5] >> 3) == 1)
                                {
                                    func_ptr(arr[5], arr[4], arr[3], arr[2], (int32_t *)&memory[arr[1]], arr[0]);
                                }
                                else if((memory[*cp+5] >> 2) == 1)
                                {
                                    func_ptr(arr[5], arr[4], arr[3], arr[2], arr[1], (int32_t *)&memory[arr[0]]);
                                }
                                else
                                {
                                    func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4], arr[memory[*cp+4]-1-5]);
                                }
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4], arr[memory[*cp+4]-1-5]);
                            }
                            break;
                        case 7:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4], arr[memory[*cp+4]-1-5], arr[memory[*cp+4]-1-6]);
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4], arr[memory[*cp+4]-1-5], arr[memory[*cp+4]-1-6]);
                            }
                            break;
                        case 8:
                            if ((int32_t)memory[*cp+6] == 2)
                            {
                                func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4], arr[memory[*cp+4]-1-5], arr[memory[*cp+4]-1-6], arr[memory[*cp+4]-1-7]);
                            }
                            else
                            {
                                val = func_ptr(arr[memory[*cp+4]-1-0], arr[memory[*cp+4]-1-1], arr[memory[*cp+4]-1-2], arr[memory[*cp+4]-1-3], arr[memory[*cp+4]-1-4], arr[memory[*cp+4]-1-5], arr[memory[*cp+4]-1-6], arr[memory[*cp+4]-1-7]);
                            }
                            break;
                    }
                    if ((int32_t)memory[*cp+6] == 0)
                    {
                        pushIntToMem(&memory[*currentOpStack], val);
                        *currentOpStack += 4;
                    }
                    if ((int32_t)memory[*cp+6] == 1)
                    {
                        pushRefToMem(&memory[*currentOpStack], val);
                        *currentOpStack += 4;
                        printStack(memory, *currentOpStack, *stackHead+72);
                    }
                    methods->methods[index].compiledCount += 1;
                    appendPC(&memory[*stackHead], 3);
                    break;
                }
            }
            methods->methods[index].interpretedCount += 1;
            pushIntToMem(&memory[*currentOpStack+4], *stackHead);
            appendPC(&memory[*stackHead], 3); // incrementing pc before changing *stackHead
            *stackHead = *currentOpStack;
            for(i = 0; i < memory[*cp+4]; i++)
            {
                pushIntToMem(&memory[*stackHead + 8 + (4*i)], arr[memory[*cp+4]-1-i]);
            }
            *currentOpStack = *stackHead + 72;
            pushIntToMem(&memory[*stackHead], getIntFromMem(&memory[*cp]));
            methods->methods->interpretedCount++;
            break;
        case 0xbb:
            // printf("\nnew"); // new(indexbyte,indexbyte)
            val = 768 + 4*(int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]);
            *heapHead -= 4*(int)memory[val] - 1;
            // printf("\nheapHead %d", *heapHead);
            for(i=0;i<(int)memory[val];i++)
            {
                // printf("\npushing to mem at %d", *heapHead + 4*i);
                pushIntToMem(&memory[*heapHead + 4*i], 0);
            }
            pushRefToMem(&memory[*currentOpStack], *heapHead);
            *heapHead -= 1;
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            // printHeap(memory, *heapHead+1);
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x3a:
            // printf("\nastore"); // astore(index)
            pushIntToMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4], popRefFromStack(memory, currentOpStack, *stackHead+72));
            // printf("\nLocal variable at index#%d = %u", memory[getIntFromMem(&memory[*stackHead])+1], getRefFromMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4]));
            appendPC(&memory[*stackHead], 2);
            break;
        case 0x19: // aload(index)
            // printf("\naLoad");
            pushRefToMem(&memory[*currentOpStack], getRefFromMem(&memory[*stackHead + 8 + memory[getIntFromMem(&memory[*stackHead])+1]*4]));        // Load LV to stack
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 2);
            break;
        case 0xb4: // getfield(indexbyte1, indexbyte2)
            // printf("\ngetfield");
            pushIntToMem(&memory[*currentOpStack], getIntFromMem(&memory[popRefFromStack(memory, currentOpStack, *stackHead+72) 
                                    + 4*((int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]))]));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 3);
            break;
        case 0xb5: // putfield(indexbyte1, indexbyte2)
            // printf("\nputfield");
            // printStack(memory, *currentOpStack, *stackHead+72);
            val = popIntFromStack(memory, currentOpStack, *stackHead+72);
            pushIntToMem(&memory[popRefFromStack(memory, currentOpStack, *stackHead+72)
                                    + 4*((int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]))], val);
            // printStack(memory, *currentOpStack, *stackHead+72);
            // printHeap(memory, *heapHead+1);
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x59: // dup
            // printf("\ndup");
            pushIntToMem(&memory[*currentOpStack], getIntFromMem(&memory[*currentOpStack-4]));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0xbc: // newarray(atype)
            // printf("\nnewarray");
            // printf("\nheapHead %u", *heapHead);
            val = popIntFromStack(memory, currentOpStack, *stackHead+72);
            // printf("\nval %d", val);
            *heapHead -= (4*val) - 1;
            // printf("\nheapHead %u", *heapHead);
            if(memory[getIntFromMem(&memory[*stackHead])+1])
            {
                for(i=0;i<val;i++)
                {
                    pushRefToMem(&memory[*heapHead + 4*i], 0);
                }
            }
            else if(memory[getIntFromMem(&memory[*stackHead])+1] == 0)
            {
                for(i=0;i<val;i++)
                {
                    pushIntToMem(&memory[*heapHead + 4*i], 0);
                }
            }
            pushRefToMem(&memory[*currentOpStack], *heapHead);
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            *heapHead -= 1;
            // printHeap(memory, *heapHead+1);
            appendPC(&memory[*stackHead], 2);
            break;
        case 0xbd:
            // printf("\nanewarray"); // anewarray(indexbyte1,indexbyte2)
            val = 768 + 4*(int16_t)(memory[getIntFromMem(&memory[*stackHead])+1] << 8 | memory[getIntFromMem(&memory[*stackHead])+2]);
            // printf("\nheapHead %u", *heapHead);
            *heapHead -= 4*(int)memory[val]*popIntFromStack(memory, currentOpStack, *stackHead+72) - 1;
            // printf("\nheapHead %u", *heapHead);
            // printf("\ncurrentOpStack %u", *currentOpStack);
            pushRefToMem(&memory[*currentOpStack], *heapHead);
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            *heapHead -= 1;
            // printHeap(memory, *heapHead+1);
            appendPC(&memory[*stackHead], 3);
            break;
        case 0x32:
            // printf("\naaload"); // aaload
            pushRefToMem(&memory[*currentOpStack], getRefFromMem(&memory[4*popIntFromStack(memory, currentOpStack, *stackHead+72) + popRefFromStack(memory, currentOpStack, *stackHead+72)]));
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x53:
            // printf("\naastore"); // aastore
            val = popRefFromStack(memory, currentOpStack, *stackHead+72);
            pushRefToMem(&memory[4*popIntFromStack(memory, currentOpStack, *stackHead+72) + popRefFromStack(memory, currentOpStack, *stackHead+72)], val);
            // printStack(memory, *currentOpStack, *stackHead+72);
            // printHeap(memory, *heapHead+1);
            appendPC(&memory[*stackHead], 1);
            break;
        case 0x1:
            // printf("\naconst_null"); // aconst_null
            pushRefToMem(&memory[*currentOpStack], 0);
            // printStack(memory, *currentOpStack, *stackHead+72);
            *currentOpStack += 4;
            appendPC(&memory[*stackHead], 1);
            break;
        default:
            // printf("\n Unknown Instruction %02x!!", instr);
            exit(0);
    }
}
