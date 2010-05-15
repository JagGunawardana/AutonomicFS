#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime
from xmlrpclib import Binary

file_name = sys.argv[1]
if len(sys.argv) != 3:
    file_count = 1
else:
    file_count = int(sys.argv[2])

s = ServerProxy('http://localhost:8085')

for i in range(file_count):
    file_struct =  s.Client_RequestFileByName(sys.argv[1]);
    file_content = file_struct[3]
    print file_struct
    print file_content
    bin = Binary(file_content)
    bin.decode(file_content)
    file = open(file_name+str(i), "wb")
    file.write(str(bin))
    print "Got file %s, count %d, length %d" %(file_name, i, len(str(bin)))
    file.close()

