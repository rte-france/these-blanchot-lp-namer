
import os
import shutil
import sys
import itertools
import urllib
import subprocess
import zipfile
import tarfile
import gzip
import datetime

import sys

result = {}
root_path = '.'
if len(sys.argv)>1:
    root_path = sys.argv[1]
# print('#getcwd() : ', os.getcwd())
# print('#root_path : ', root_path)
sorted_root_dir = sorted(os.listdir(root_path))
# print('#sorted_root_dir : ', sorted_root_dir)

for instance in sorted_root_dir:
    if '.mps' in instance and not '-1.mps' in instance:
        buffer = instance.strip().split("-")
        year = int(buffer[1])
        week = int(buffer[2])
        if not (year, week) in result:
            result[year, week] = [instance, '', '']
# print('#result : ', result)
#for line in result.iteritems():
#    print line


for instance in sorted_root_dir:
    if 'variables' in instance:
        buffer = instance.strip().split("-")
        year = int(buffer[1])
        week = int(buffer[2])
        result[year, week][1]  = instance
        

for instance in sorted_root_dir:
    if 'constraints' in instance:
        buffer = instance.strip().split("-")
        year = int(buffer[1])
        week = int(buffer[2])
        result[year, week][2]  = instance
        
for line in result.items():
    print(line[1][0]+' '+line[1][1]+' '+line[1][2]+' ')
    