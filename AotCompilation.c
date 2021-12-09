#include "stdlib.h"
#include "AotCompilation.h"

methods initMethods(int size, char *fileHandle)
{
    methods newMethods;
    newMethods.size = size;
    newMethods.methods = malloc(sizeof(method)*newMethods.size);
    return newMethods;
}

void addNewMethod(methods *methods, int index)
{
    if (methods->size <= index)
    {
        methods->size = index + 1;
        methods->methods = realloc(methods, sizeof(method)*methods->size);
    }
    methods->methods[index].functionIndex = index;
    methods->methods[index].isCompiled = -1;
    methods->methods[index].init = 1;
}

int isAdded(methods *methods, int index)
{
    if(methods->methods[index].init == 1)
    {
        return 1;
    }
    return 0;
}

void printMethods(methods *methods)
{
    for (int i = 0; i < methods->size; i++)
    {
        if (methods->methods[i].init != -1)
        {
            printf("func_%d | I: %d | C: %d", methods->methods[i].functionIndex, methods->methods[i].interpretedCount, methods->methods[i].compiledCount);
        }
        printf("\n");
    }
    
}