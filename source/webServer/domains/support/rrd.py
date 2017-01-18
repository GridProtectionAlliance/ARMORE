# # # # #
# rrd.py
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

import sys
from domains.support.lib.common import *
import re
from pprint import pprint

def getRrdFiles():
    return getRrdFilesInfo(getFileList("/var/webServer/static/rrd", recursive=True, match='^armore-.*'))

def parseInfo(paramList):
    ds = {} 
    cf = {}
    for p in paramList.split("\n"):
        dsSearch = re.search('^ds\[(\w+)\]', p)
        cfSearch = re.search('.*\.cf = "(\w+)"', p)
        if dsSearch:
            ds.update({dsSearch.group(1): 1})
        if cfSearch:
            cf.update({cfSearch.group(1): 1})
    
    return list(ds.keys()), list(cf.keys())

def getRrdFilesInfo(rrdFiles):
    paramList = {}
    newDict = {}
    for d in rrdFiles:
        connFrom, connTo = [re.sub('_', '.', x) for x in (re.sub('armore-', '', d.split('/')[-1])).split('__')]

        if connFrom in newDict:
            if not connTo in newDict[connFrom]:
                newDict[connFrom][connTo] = []
        else:
            newDict[connFrom] = {connTo: []}

        for f in rrdFiles[d]:
            paramList = cmd(["rrdtool", "info", "{0}/{1}".format(d, f)])
            ds, cf = parseInfo(paramList)
            rrdFiles[d][f]['ds'] = ds
            rrdFiles[d][f]['cf'] = cf
            newDict[connFrom][connTo].append(f)

    return newDict

