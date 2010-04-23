#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime

s = ServerProxy('http://localhost:8090')

files =  s.Service_GetAllFilesUnderMgt();
print str(files)
for this_file in files:
    print str(this_file)

