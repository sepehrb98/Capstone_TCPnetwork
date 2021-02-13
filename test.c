#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


int main(void)
{
    int i=1;
    while(1) {
        i--;
        if(i == -1) {
            printf("ah");
            exit(0);
        }
        printf("%d\n", i);
    }
    return 0;
}