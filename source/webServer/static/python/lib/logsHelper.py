# # # # #
# logs.py
#
# Contains methods used for generic
# networking functions
#
# University of Illinois/NCSA Open Source License
# Copyright (c) 2015 Information Trust Institute
# All rights reserved.
#
# Developed by:
#
# Information Trust Institute
# University of Illinois
# http://www.iti.illinois.edu
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal with
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# Redistributions of source code must retain the above copyright notice, this list
# of conditions and the following disclaimers. Redistributions in binary form must
# reproduce the above copyright notice, this list of conditions and the following
# disclaimers in the documentation and/or other materials provided with the
# distribution.
#
# Neither the names of Information Trust Institute, University of Illinois, nor
# the names of its contributors may be used to endorse or promote products derived
# from this Software without specific prior written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
#
# # # # #

import gzip
import os
from common import *
import re
import html
import mimetypes

logLocations = [
    '/var/log/',
    '/var/log/armore/'
]

def testGzFile(filename):
    ret = False
    unzippedFile = 'testFile'
    uf = open(unzippedFile, 'wb')
    contents = gzip.open(filename, 'rb').read()
    uf.write(contents)
    fileType = cmd('file ' + unzippedFile).split(': ')[1]
    if re.match('.*text.*', fileType):
        ret = True
    os.remove(unzippedFile) 

    return ret

def includeFile(filename, exclude=None):
    if not os.access(filename, os.R_OK):
        return False

    if exclude and re.search(exclude, filename, re.I):
        return False

    theType = getFileType(filename)
    if re.match('.*gzip.*', theType, re.I):
        return testGzFile(filename)
    if re.match('.*data.*', theType, re.I):
        return False

    return True

