#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>

#define FSIZE 128402

int main() {
    // shamelessly reusing the mmap sequence from culler.c
    int srcDesc = open("cullOutp.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0);
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    unsigned char *nearL = startPtr,        *farL = startPtr + 15,
                  *nearU = startPtr + 18,   *farU = nearU,  *endPtr = startPtr + FSIZE;

    int line = 2, diffL = farL - nearL, diffU;
    while (1) {
        while (1) {
            unsigned char cmp = *farU;
            if (cmp == 0xE0) {
                if ((*(++farU) & 0xFE) != 0xA4) {
                    printf("Line %d: Malformed Devanagari (second character)\n", line);
                    return 1;
                } else if ((*(++farU) & 0xC0) != 0x80) {
                    printf("Line %d: Malformed Devanagari (third character)\n", line);
                    return 1;
                } else
                    farU++;
            } else if (cmp <= 0x7F) {
                if (cmp == 0x0D) {
                    farU--;
                    if ((diffU = farU - nearU) <= 0) {
                        printf("Line %d: Empty/malformed line\n", line);
                        return 1;
                    } else if (diffL == diffU && !strncmp(nearL, nearU, diffL)) {
                        printf("Line %d: Duplicate line\n", line);
                        return 1;
                    }
                    nearL = nearU;
                    farL = farU;
                    diffL = diffU;
                    if (*(farU += 2) != 0x0A) {
                        printf("Line %d: Malformed CRLF (second character)\n", line);
                        return 1;
                    } else if (++farU == endPtr) {
                        printf("Processing successful\n");
                        munmap(startPtr, FSIZE);
                        close(srcDesc);
                        return 0;
                    }
                    nearU = farU;
                    line++;
                    break;
                } else
                    farU++;
            } else {
                printf("Line %d: Malformed character\n", line);
                return 1;
            }
        }
    }
}