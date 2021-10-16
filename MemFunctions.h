extern void pushIntToMem(unsigned char *destination, signed int num);
extern signed int getIntFromMem(unsigned char *memPtr);
extern signed int getIntFromLoadedMem(unsigned char *memPtr);
extern void relocateInt(unsigned char *source, unsigned char *destination);
extern void addIntValueToMem(unsigned char* source, signed int num);