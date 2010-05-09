#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime
from xmlrpclib import Binary

file_hash = sys.argv[1]
if len(sys.argv) != 3:
    file_count = 1
else:
    file_count = int(sys.argv[2])

s = ServerProxy('http://localhost:8085')

for i in range(file_count):
    file_struct =  s.Service_RequestFileByHash(sys.argv[1]);
    print str(file_struct)
    file_content = file_struct[2]
    bin = Binary(file_content)
    bin.decode(file_content)
    print str(bin)

