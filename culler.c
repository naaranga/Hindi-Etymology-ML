#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
#define _GNU_SOURCE

#define FSIZE 295003
#define FINDACCEPTABLE { \
        while (1) { \
            while (*(++farPtr) != 0x0D); \
            if ((farPtr += 2) == fileEndPtr) \
                goto terminateCull; \
            prevNearPtr = farPtr; \
            while (1) { \
                if (*farPtr == 0xE0) { \
                    if ((*(++farPtr) & 0xFE) != 0xA4) \
                        break; \
                    else if ((*(++farPtr) & 0xC0) != 0x80) \
                        break; \
                    else \
                        farPtr++; \
                } else if (*farPtr == 0x7C) { \
                    prevCmpSize = farPtr-prevNearPtr; \
                    nearPtr = farPtr+1; \
                    goto foundAcceptable; \
                } else if (*farPtr <= 0x7F) { \
                    farPtr++; \
                } else { \
                    break; \
                } \
            } \
        } \
    }
#define MNGTRAILER if (*trailer) { \
        farPtr = (nearPtr += 2); \
        while (1) { \
            while (*(++farPtr) != 0x7C); \
            if (strncmp(prevNearPtr, nearPtr, (cmpSize = farPtr-nearPtr))) \
                goto postDiscard; \
            else { \
                while (*(++farPtr) != 0x0D); \
                if ((farPtr += 2) == fileEndPtr) \
                    goto terminateCull; \
                nearPtr = farPtr; \
            } \
        } \
    } else \
        *trailer =

int main() {
    int srcDesc = open("ec2.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0);
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    unsigned char *prevNearPtr = startPtr,  *prevFarPtr = startPtr + 15,
                  *nearPtr = startPtr + 26, *farPtr = startPtr + 44,        *fileEndPtr = startPtr + FSIZE;
    FILE* outp = fopen("cullOutp.txt", "wb");
    unsigned char tempFBuf[16384], *fBufTrav = tempFBuf;
    ptrdiff_t bufConsumption = 0;

    unsigned char trailer[3] = {83, 0x0D, 0x0A}; // 0 = none, 83 [S] = Sanskrit, 80 [P] = Persian, 69 [E] = English
    ptrdiff_t prevCmpSize = prevFarPtr-prevNearPtr, cmpSize, addend;
    //for (int i = 0; i < 100; i++) {
    while (1) {
        if ((cmpSize = farPtr-nearPtr) != prevCmpSize || strncmp(prevNearPtr, nearPtr, cmpSize)) {
            if (*trailer) {
                if ((bufConsumption += (addend = prevCmpSize + 3)) >= 16384) {
                    fwrite(tempFBuf, sizeof(unsigned char), bufConsumption - addend, outp);
                    bufConsumption = addend;
                    fBufTrav = tempFBuf;
                }
                memcpy(fBufTrav, prevNearPtr, prevCmpSize);
                memcpy(fBufTrav += prevCmpSize, trailer, sizeof(unsigned char)*3);
                fBufTrav += 3;
            }
            postDiscard:
            prevNearPtr = nearPtr;
            *trailer = 0;
            while (nearPtr != farPtr) {
                if (*nearPtr == 0xE0) {
                    if ((*(++nearPtr) & 0xFE) != 0xA4)
                        FINDACCEPTABLE
                    else if ((*(++nearPtr) & 0xC0) != 0x80)
                        FINDACCEPTABLE
                    else
                        nearPtr++;
                } else if (*nearPtr <= 0x7F)
                    nearPtr++;
                else
                    FINDACCEPTABLE
            }
            nearPtr++;
            prevCmpSize = cmpSize;
            foundAcceptable:
            prevFarPtr = farPtr;
        }
        while (*(++farPtr) != 0x0D);
        ptrdiff_t searchSize = farPtr-nearPtr;
        if (memmem(nearPtr, searchSize, "Sanskrit", 8)) {
            MNGTRAILER 83;
        } else if (memmem(nearPtr, searchSize, "Persian", 7)) {
            MNGTRAILER 80;
        } else if (memmem(nearPtr, searchSize, "English", 7)) {
            MNGTRAILER 69;
        }
        if ((farPtr += 2) == fileEndPtr)
            goto appendFinal;
        while (*farPtr == 0x7C) {
            while (*(++farPtr) != 0x0D);
            farPtr += 2;
        }
        nearPtr = farPtr;
        while (*(++farPtr) != 0x7C);
    }
    appendFinal: // might be some logic errors here idk it works
    if ((bufConsumption += (addend = prevCmpSize + 1)) >= 16384) {
        fwrite(tempFBuf, sizeof(unsigned char), bufConsumption - addend, outp);
        bufConsumption = addend;
        fBufTrav = tempFBuf;
    }
    memcpy(fBufTrav, prevNearPtr, prevCmpSize);
    *(fBufTrav += prevCmpSize) = *trailer;
    fBufTrav++;
    terminateCull:
    if (bufConsumption) // unnecessary comparison if appendFinal runs but whatever
        fwrite(tempFBuf, sizeof(unsigned char), bufConsumption, outp);
    munmap(startPtr, FSIZE);
    close(srcDesc);
    return 0;
}