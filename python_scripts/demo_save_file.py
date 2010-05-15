#!/usr/bin/python
import sys

from xmlrpclib import ServerProxy
import datetime
from xmlrpclib import Binary

file_name = sys.argv[1]

s = ServerProxy('http://localhost:8085')

print "Trying to save file: "+file_name
file=open(file_name, "r")
file_content=file.read()
print "Original file: "+file_content
bin=Binary(file_content)
out_file=open("Temp_out", "w")
bin.encode(out_file)
out_file.close()
print "Binary data: "+bin.data
print "Encoded file: "+open("Temp_out", "r").read()
binary_content=open("Temp_out", "rb").read()
bin1=Binary(binary_content)
ret_val = s.Client_SaveFile(file_name, bin1)
print "Got return "+str(ret_val)
file.close()
out_file.close()

