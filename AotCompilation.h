#ifndef AOTCOMPILATION_H
#define AOTCOMPILATION_H
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int init;
    int functionIndex;
    bool isCompiled;
    int interpretedCount;
    int compiledCount;
} method;

typedef struct {
    method *methods;
    void *fileHandle;
    int size;
} methods;

extern methods initMethods(int size, char *fileHandle);
extern int checkInLibrary(int index);
extern void addNewMethod(methods *methods, int index);
extern int isAdded(methods *methods, int index);
extern int isCompiledMethod(methods *methods, int index);
extern void printMethods(methods *methods);
#endif