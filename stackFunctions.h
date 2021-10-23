extern int popIntFromStack(unsigned char *memPtr, unsigned int *stackHead, unsigned int stackStart);
extern unsigned int popRefFromStack(unsigned char *memPtr, unsigned int *stackHead, unsigned int stackStart);
extern void printStack(unsigned char *memPtr, unsigned int stackHead, unsigned int stackStart);
extern void printHeap(unsigned char *memPtr, unsigned int heapHead);