#include <stdio.h>
#include "MemFunctions.h"

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