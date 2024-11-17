/**
 * low_open1.c
 *  - char buf[]에서 크기를 지정하지 않고, sizeof(buf)를 사용하여 파일로 저장한 경우
 *  - buf[] 배열 마지막에 NULL 문자가 자동으로 추가됨 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void error_handling(char* message);
int main(void)
{
    int fd;
    int size;    
    char buf[]="Let's go!\n";
    
    fd=open("data1.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    if(fd==-1)
        error_handling("open() error!");

	// strlen(buf): exclude NULL character
    printf("file descriptor: %d, strlen: %ld\n", fd, strlen(buf));
   	printf("sizeof(buf): %ld\n", sizeof(buf));

    size = write(fd, buf, sizeof(buf));
    printf("write size: %d\n", size);    
    if(size ==-1)
        error_handling("write() error!");

    close(fd);
    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
