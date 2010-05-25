#!/usr/bin/python
import sys
import string

from xmlrpclib import ServerProxy
import datetime
from xmlrpclib import Binary

file_name = sys.argv[1]
s = ServerProxy('http://localhost:8085')
ret_val = s.Client_DeleteFile(file_name)
print "Got return "+str(ret_val)

