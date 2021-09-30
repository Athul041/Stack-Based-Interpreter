#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>

void readFromFile(char fileName[], int* memPtr)         // Implement using loops for direct load
{
    FILE *f1 = fopen(fileName, "r");
    // printf("\nargv %s", fileName);
    if (f1)
    {
        fseek(f1, 0, SEEK_END);
        long filesize = ftell(f1);
        // printf("\n%d",filesize);
        fseek(f1, 0 ,SEEK_SET);
        int* buffer = malloc(filesize);
        // fread(*memPtr, 1, filesize, f1);             // Check to load directly to memory
        fread(buffer, 1, filesize, f1);
        fclose(f1);
        memPtr = *buffer;
        free(buffer);
    }    
    else
    {
        printf("Could not read file\n");
        return(0);
    }
    fclose(f1);
}

int main(int argc, char *argv[])
{
    int* memPtr = malloc(pow(2,3));
    if(memPtr == NULL)
    {
        printf("No Memory allocated to Interpreter\n");
        return(0);
    }
    // printf("argc %d, argv %s", argc, argv);

    if(argc < 3)
    {
        printf("Executable and/or constant_pool not given");
        return(0);
    }

    readFromFile(argv[1], memPtr[1024]);
    readFromFile(argv[2], memPtr[256]);

    if(argc == 4)
    {
        memPtr[0] = *argv[3];
    }

    int* varStack = memPtr[1048576];                // Variables on Stack Head
    int* stackHead = memPtr[1048576 + 32*16];       // Execution Stack Head
    int* PC = memPtr[1024];                         // Program Counter
    int* CP = memPtr[256];                          // Constant Pool Cunter

    printf("StackHead %d", stackHead);
    // while(stackHead >= 1048576 + 32*16)
    // {
    //     if(PC == CP)
    //     {
            
    //     }
    // }

}

