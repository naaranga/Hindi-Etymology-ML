#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>

#define FSIZE 128402

int main() {
    // shamelessly reusing my own code from the other files yet again
    int srcDesc = open("randCullOutp.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0);
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    unsigned char *travPtr = startPtr, *endPtr = startPtr + FSIZE, outBuf[8192], *outTrav = outBuf;
    unsigned int inCounts[128] = {0}, asCounts[128] = {0};

    while (1) {
        unsigned char cmp = *travPtr;
        if (cmp == 0xE0) {
            inCounts[((*(travPtr + 1) & 1) << 6) | (*(travPtr + 2) & 0x3F)]++;
            travPtr += 3;
        } else if (cmp != 0x0D) {
            asCounts[cmp]++;
            travPtr++;
        } else {
            if ((travPtr += 2) == endPtr) {
                for (int i = 0; i < 64; i++)
                    outTrav += sprintf(outTrav, "\xE0\xA4%c\t%u\x0D\x0A", 0x80 | i, inCounts[i]);
                for (int i = 64; i < 128; i++)
                    outTrav += sprintf(outTrav, "\xE0\xA5%c\t%u\x0D\x0A", i ^ 0xC0, inCounts[i]);
                for (int i = 0; i < 33; i++)
                    outTrav += sprintf(outTrav, "0x%X\t%u\x0D\x0A", i, asCounts[i]);
                for (int i = 33; i < 127; i++)
                    outTrav += sprintf(outTrav, "%c\t%u\x0D\x0A", i, asCounts[i]);
                outTrav += sprintf(outTrav, "0x7F\t%u\x0D\x0A", asCounts[127]);
                FILE* outp = fopen("census.txt", "wb");
                fwrite(outBuf, sizeof(unsigned char), outTrav-outBuf, outp);
                fclose(outp);
                munmap(startPtr, FSIZE);
                close(srcDesc);
                return 0;
            }
        }
    }
}