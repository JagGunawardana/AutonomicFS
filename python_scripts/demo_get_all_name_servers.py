#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime
from xmlrpclib import Binary

if len(sys.argv) < 2:
    host = "localhost"
else:
    host = sys.argv[1]

s = ServerProxy('http://%s:8085' %(host))

all_servers =  s.Client_GetAllNameServers();
print str(all_servers)

