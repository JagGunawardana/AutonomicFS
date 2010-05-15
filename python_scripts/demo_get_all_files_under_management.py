#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime

if len(sys.argv) < 2:
    host = "localhost"
else:
    host = sys.argv[1]

s = ServerProxy("http://%s:8085" % (host))
files =  s.Client_GetAllFilesUnderMgt();
for this_file in files:
    print str(this_file)

