global _start
_start:
xor eax,eax
push eax
push "//sh"
push "/bin" ; push "/bin//sh"
mov ebx,esp ; first argument, points to the address of "/bin//sh"
mov ecx,eax ; second argument
xor edx,edx ; third argument
mov al,0Bh  ; execve system call number
int 80h
