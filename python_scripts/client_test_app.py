#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime

file_name = sys.argv[1]
file_count = int(sys.argv[2])

s = ServerProxy('http://localhost:8090')

for i in range(file_count):
    file_content =  s.Service_FileByName(sys.argv[1]);
    file = open(file_name+str(i), "wb")
    file.write(str(file_content[1]))
    print "Got file %s, count %d" %(file_name, i)
    file.close()

