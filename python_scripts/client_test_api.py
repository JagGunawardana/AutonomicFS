#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime

s = ServerProxy('http://localhost:8085')
files =  s.Service_GetAllFilesUnderMgt();
for this_file in files:
    print str(this_file)

