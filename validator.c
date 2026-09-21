#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>

#define FSIZE 128400

int main() {
    // shamelessly reusing the mmap sequence from culler.c
    int srcDesc = open("cullOutp.txt", O_RDONLY);
    if (srcDesc == -1) return 1;
    /*struct stat fs;
    if (fstat(srcDesc, &fs) == -1) { printf("Stat failure\n"); close(srcDesc); return 1; }
    printf("%zu\n", fs.st_size);*/
    unsigned char *startPtr = mmap(NULL, FSIZE, PROT_READ, MAP_PRIVATE, srcDesc, 0);
    if (startPtr == MAP_FAILED) { close(srcDesc); return 1; }
    unsigned char *nearA = startPtr,        *farA = startPtr + 13,
                  *nearB = startPtr + 16,   *farB = nearB,  *endPtr = startPtr + FSIZE;
    
    int line = 0;
    while (1) {
        while (1) {
            if (farB == 0xE0) {
                if ((*(++farB) & 0xFE) != 0xA4) {
                    printf("Line %d: Malformed Devanagari (second character)\n", line);
                    return 1;
                } else if ((*(++farB) & 0xC0) != 0x80) {
                    printf("Line %d: Malformed Devanagari (third character)\n", line);
                    return 1;
                } else
                    farB++;
            } else if (farB <= 0x7F) {
                if (farB == 0x0D) {
                    farB--;
                    if (farB - nearB <= 0) {
                        printf("Line %d: Empty/malformed line\n", line);
                        return 1;
                    }
                    // handle EOL
                    line++;
                } else
                    farB++;
            } else {
                printf("Line %d: Malformed character\n", line);
                return 1;
            }
        }
    }
    munmap(startPtr, FSIZE);
    close(srcDesc);
    return 0;
}