#include "AotCompilation.h"

extern void interpretInstructions(unsigned char *memory, unsigned int *stackHead, unsigned int *currentOpStack, unsigned int *cp, unsigned int *heapHead, int threshold, methods *methods);