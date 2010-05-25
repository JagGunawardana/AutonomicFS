#!/usr/bin/python
import sys
import string

from xmlrpclib import ServerProxy
import datetime
from xmlrpclib import Binary

file_name = sys.argv[1]
force = sys.argv[2]
if (len(sys.argv)==2):
    force = False
elif (sys.argv[2]=="Force"):
    force = True
else:
    force = False

s = ServerProxy('http://localhost:8085')

file=open(file_name, "r")
file_content=file.read()
bin=Binary(file_content)
out_file=open("Temp_out", "w")
bin.encode(out_file)
out_file.close()
binary_content=open("Temp_out", "rb").read()
new_binary_content=""
for line in binary_content.split('\n'):
    if not (line.startswith('<value>') or line.startswith('</base64>')):
        new_binary_content += line

print "Content: "+new_binary_content
bin1=Binary(new_binary_content)
ret_val = s.Service_SaveFile(file_name, bin1, force)
print "Got return "+str(ret_val)
file.close()
out_file.close()

