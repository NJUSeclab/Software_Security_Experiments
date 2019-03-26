// buf2.c
// gcc -z execstack -o buf2 buf2.c -fno-stack-protector

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void vul(char *str){
    char buffer[36];
    // buffer overflow
    strcpy(buffer, str);
}

int main(int argc, char **argv){
    char str[128];
    FILE *file;
    file = fopen("attack_input2", "r");
    fread(str, sizeof(char), 128, file);
    vul(str);
    printf("Returned Properly\n");
    return 0;
}
