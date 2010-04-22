#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime

file_name = sys.argv[1]
file_count = int(sys.argv[2])

s = ServerProxy('http://localhost:8085')

for i in range(file_count):
    file_content =  s.Service_RequestFile(sys.argv[1]);
    file = open(file_name+str(i), "wb")
    file.write(str(file_content))
    print "Got file %s, count %d, length %d" %(file_name, i, len(str(file_content)))
    file.close()

