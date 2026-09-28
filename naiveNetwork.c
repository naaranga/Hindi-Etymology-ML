#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>

#define FSIZE 128402

int main() {
    // you guessed it, reusing the same code again (yippee)
    int srcDesc = open("randCullOutp.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0);
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    unsigned char *travPtr = startPtr;
    int embedCull[128] = {65535, 0, 1, 2, 65535, 3, 4, 5, 6, 7, 8, 9, 65535, 10, 65535, 11, 12, 13, 65535, 14, 15, 16, 17, 18, 19, 65535, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 65535, 35, 36, 37, 38, 39, 40, 41, 65535, 42, 65535, 65535, 43, 44, 45, 46, 47, 65535, 65535, 48, 65535, 49, 50, 51, 52, 53, 54, 65535, 55, 65535, 56, 57, 58, 65535, 59, 60, 61, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 62, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 63, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535};
    double embedData[192];
    srand(525354492); // still keeping it a trng constant for debugging and whatever
    double randFactor = 2/(double)RAND_MAX;
    for (int i = 0; i < 192; i++)
        embedData[i] = rand() * randFactor - 1;
    double embedLayer[57];
    
    // planned architecture (nodes per layer): 57 (embed layer) -> 19 (hidden layer) -> 8 (hidden layer) -> 3 (output layer)
    
    munmap(startPtr, FSIZE);
    close(srcDesc);
    return 0;
}