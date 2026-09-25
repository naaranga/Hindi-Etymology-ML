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
    // shamelessly reusing my own code from the other files once more
    int srcDesc = open("randCullOutp.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0);
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    unsigned char *travPtr = startPtr, *endPtr = startPtr + FSIZE;
    int fileAccumulator = 0;
    while (1) {
        unsigned char cmp = *travPtr;
        int lineAccumulator = 0;
        while (1) {
            if (cmp == 0xE0) {
                srand(lineAccumulator);
                srand((lineAccumulator = rand() + *(travPtr + 1))); // idk if this pointer access is more optimal than what i was doing in culler.c but maybe it is (probably doesn't matter)
                lineAccumulator = rand() + *(travPtr + 2);
                cmp = *(travPtr += 3);
            } else if (cmp == 0x0D) {
                travPtr += 2;
                if (travPtr == endPtr) {
                    printf("%u\n", fileAccumulator + lineAccumulator);
                    munmap(startPtr, FSIZE);
                    close(srcDesc);
                    return 0;
                } else {
                    fileAccumulator += lineAccumulator;
                    break;
                }
            } else {
                srand(lineAccumulator);
                lineAccumulator = rand() + *travPtr;
                cmp = *(++travPtr);
            }
        }
    }
}