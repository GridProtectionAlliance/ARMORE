# # # # #
# memory.py
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

from domains.support.lib.common import *
import re
import struct
import psutil
import datetime

# Returns a dictionary containing
# each interface on a machine and
# the IP address to which it is
# mapped

def sortOrderBy(array, sortBy="pid", orderBy="asc"):
    ret = []
    tmpArray = []
    for a in array:
        if type(a[sortBy]) == type(""):
            app = ""
            for l in a[sortBy]:
                try:
                    app += l.lower()
                except:
                    app += l
            tmpArray.append(app)
        else:
            tmpArray.append(a[sortBy])

    if orderBy == "asc":
        tmpArray.sort()
    else:
        tmpArray.sort(reverse=True)

    pidsUsed = []
    for x in range(len(tmpArray)):
        for a in array:
            if a[sortBy] == tmpArray[x] and (a['pid'] not in pidsUsed):
                ret.append(a)
                pidsUsed.append(a['pid'])
                continue
    return ret

def filt(array, theFilt=None, thePid=None):
    ret = []
    for a in array:
        filtAppend = True
        pidAppend = True
        if theFilt:
            if theFilt == 'user':
                if a['user'] == 'root':
                    filtAppend = False
            elif theFilt == 'all':
                if a['user'] != 'root':
                    filtAppend = False
            else:
                if theFilt != a['name']:
                    filtAppend = False
        if thePid:
            if str(a['pid']) != str(thePid):
                pidAppend = False
        ret.append(a) if filtAppend and pidAppend else None

    return ret
