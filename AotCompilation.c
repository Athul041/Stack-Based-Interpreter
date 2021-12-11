#include "stdlib.h"
#include "AotCompilation.h"

methods initMethods(methods *newMethods, int size, void *fileHandle)
{
    newMethods->methods = malloc(sizeof(method)*size);
    newMethods->used = 0;
    newMethods->size = size;
    newMethods->fileHandle = fileHandle;
}

void addNewMethod(methods *allmethods, int index)
{
    if (allmethods->size <= allmethods->used)
    {
        allmethods->size = allmethods->size + 1;
        allmethods->methods = realloc(allmethods->methods, sizeof(method)*allmethods->size);
    }
    if (allmethods->size <= index)
    {
        allmethods->size = index + 1;
        allmethods->methods = realloc(allmethods->methods, sizeof(method)*allmethods->size);
    }
    allmethods->methods[index].functionIndex = index;
    allmethods->methods[index].isCompiled = -1;
    allmethods->methods[index].init = 1;
    allmethods->methods[index].interpretedCount = 0;
    allmethods->methods[index].compiledCount = 0;
}

int isAdded(methods *methods, int index)
{
    if (methods->size <= index)
    {
        return 0;
    }
    if(methods->methods[index].init == 1)
    {
        return 1;
    }
    return 0;
}

void printMethods(methods *methods)
{
    printf("\nMethods Table\n");
    int i;
    for (i = 0; i < methods->size; i++)
    {
        if (methods->methods[i].functionIndex != 0 && methods->methods[i].init != -1)
        {
            printf("func_%d | Interpreted: %d | Compiled: %d", methods->methods[i].functionIndex, methods->methods[i].interpretedCount, methods->methods[i].compiledCount, methods->methods[i].init);
        }
        printf("\n");
    }
    
}