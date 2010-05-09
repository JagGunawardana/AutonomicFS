#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime
from xmlrpclib import Binary


s = ServerProxy('http://localhost:8085')

all_servers =  s.Client_GetAllNameServers();
print str(all_servers)

