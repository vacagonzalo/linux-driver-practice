#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    FILE *fp;
    int fd;
    fp = fopen("/dev/myled_char","w");
    if(fp == NULL)
    {
        printf("Error!");   
        exit(1);             
    }
    fd = fileno(fp);

    for(int i = 0; i < 5; ++i) {
        write(fd, "1", sizeof(char));
        sleep(1);
        write(fd, "0", sizeof(char));
        sleep(1);
    }

    fclose(fp);

   return 0;
}
