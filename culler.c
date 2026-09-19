#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define FSIZE 294955
#define DISCARD *copyBuf = 0; \
    while (*(++travPtr) != 0x0D); \
    travPtr += 2; \
    prevCtr = 0;

int main() {
    int srcDesc = open("ec2.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0);
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    unsigned char *travPtr = startPtr, *endPtr = startPtr + FSIZE;
    FILE* outp = fopen("cullOutp.txt", "wb");

    unsigned char copyBuf[384] = {0xE0, 0xA4, 0xB5, 0xE0, 0xA4, 0xBF, 0xE0, 0xA4, 0xB6, 0xE0, 0xA5, 0x8D, 0xE0, 0xA4, 0xB5},
    *cbTrav = copyBuf;
    // initialization of rest of array is not necessary (big sad)
    
    char trailer[3] = {0, 0x0D, 0x0A}; // 1 = none, 2 = Sanskrit, 3 = Persian, 4 = English
    int prevCtr = 0, currCtr = 3, prevDiscard = 0;
    while (1) {
        unsigned char cmp = *travPtr;
        if (cmp == 0xE0) {
            if ((cmp = *(++travPtr)) != *(++cbTrav)) {
                // DUMP + COPY MODE
                if (prevCtr != 0)
                    fwrite(copyBuf, sizeof(unsigned char), prevCtr, outp);
                if ((cmp & 0xFE) == 0xA4) {
                    *(cbTrav++) = cmp;
                    cmp = *(++travPtr);
                    while (1) {   
                        if (cmp == 0xE0) {
                            if ((cmp = *(++travPtr)) != *(++cbTrav)) {
                                if ((cmp & 0xFE) == 0xA4)
                                    *(cbTrav++) = cmp;
                                else {
                                    DISCARD
                                }
                            } else if ((cmp = *(++travPtr)) != *(++cbTrav)) {
                                if ((cmp & 0xC0) != 0x80)
                                    *(cbTrav++) = cmp;
                                else {
                                    DISCARD
                                }
                            } else {
                                currCtr += 3;
                                travPtr++;
                                cbTrav++;
                            }
                        } else if (cmp == 0x7C) {
                            // HANDLE CBUF TRAILER
                            // LANG ANALYSIS MODE
                        } else if (cmp <= 0x7F) {
                            if (cmp != *cbTrav) {
                                // DUMP + COPY MODE
                            } else {
                                currCtr++;
                                travPtr++;
                                cbTrav++;
                            }
                        } else {

                        }
                    }
                } else {
                    DISCARD
                }

            } else if ((cmp = *(++travPtr)) != *(++cbTrav)) {
                if ((cmp & 0xC0) != 0x80) {
                    
                } else {
                    DISCARD
                }
                // DUMP + COPY MODE
            } else {
                currCtr += 3;
                travPtr++;
                cbTrav++;
            }
        } else if (cmp == 0x7C) {
            // HANDLE CBUF TRAILER
            if (cbTrav != copyBuf) {
                memcpy(copyBuf, trailer, sizeof(unsigned char)*3); // questionable methodology maybe
                cbTrav = copyBuf;
            } else {
                DISCARD
            }
            // LANG ANALYSIS MODE
        } else if (cmp <= 0x7F) {
            if (cmp != *cbTrav) {
                // DUMP + COPY MODE
            } else {
                currCtr++;
                travPtr++;
                cbTrav++;
            }
        } else {
            DISCARD
        }
    }
    munmap(startPtr, FSIZE);
    close(srcDesc);
    return 0;
}