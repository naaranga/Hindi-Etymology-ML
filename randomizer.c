#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>

#define FSIZE 128402

struct line {
    unsigned char* stPtr;
    ptrdiff_t length;
};

int main() {
    // shamelessly reusing the mmap sequence from culler.c and validator.c again
    int srcDesc = open("cullOutp.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0), *travPtr = startPtr;
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    struct line lines[6639], *lTrav = lines, *lOne = lines+1, *lEnd = lines+6639;
    
    *lTrav = (struct line){ .stPtr = startPtr };
    unsigned char* tSub = startPtr;
    while (1) {
        while (*(++travPtr) != 0x0D);
        (*lTrav).length = (travPtr += 2) - tSub;
        if ((++lTrav) == lEnd)
            break;
        tSub = travPtr;
        (*lTrav).stPtr = tSub;
    }

    unsigned char outputBuf[FSIZE], *outpTrav = outputBuf;
    int modulo = 6639;
    srand(882662705); // literally just generated this with a trng
    struct line tmpCpy;
    while (1) {
        struct line *randLine = lines + rand() % modulo;
        ptrdiff_t derefLen = randLine->length;
        strncpy(outpTrav, randLine->stPtr, derefLen);
        *randLine = *(--lTrav);
        outpTrav += derefLen;
        if (lTrav == lOne)
            break;
        modulo--;
    }
    strncpy(outpTrav, lines->stPtr, lines->length);
    FILE* outp = fopen("randCullOutp.txt", "wb");
    fwrite(outputBuf, sizeof(unsigned char), FSIZE, outp);
    fclose(outp);
    munmap(startPtr, FSIZE);
    close(srcDesc);
    return 0;
}