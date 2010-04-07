#!/usr/bin/python

from xmlrpclib import ServerProxy
import datetime

s = ServerProxy('http://localhost:8085')

print s.Ping(10)
