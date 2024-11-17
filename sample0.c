#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef struct {
    int num;
    char name[10];
    float mid;
    float final;
    float total;
    double average;
} SCORE;

int main(void) {
    int size = 0;
    SCORE* p = NULL;
    printf("sizeof(SCORE): %ld, size: %d\n", sizeof(SCORE), size);
    
    p = (SCORE*)malloc(size); // 잘못된 메모리 할당
    // p = (SCORE*)malloc(sizeof(SCORE) * size); // 정상적인 메모리 할당
    
    printf("allocated memory size: %ld\n", malloc_usable_size(p));
    
    if (p == NULL) {
        printf("메모리 할당 오류\n");
        exit(1);
    }
    
    free(p);
    return 0;
}