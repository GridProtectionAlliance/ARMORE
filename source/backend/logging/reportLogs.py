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
# Add backend lib to path
import sys
sys.path.append("../lib")

# Import backend library functions
from log import *
from common import *
import datetime
import re

def getDateFromLine(line):
    m = re.search(r'^(\w{3})\s+(\d\d?)\s+(\d\d):(\d\d):(\d\d)', line)

    mon = int(monToNum(m.group(1)))
    day = int(m.group(2))
    hh = int(m.group(3))
    mm = int(m.group(4))
    ss = int(m.group(5))

    timestamp = toTimestamp(mon, day, hh, mm, ss)
    return timestamp

def entryAllowed(line, year, mon, day, hh, mm, ss):
    earliestTimestamp = datetime.datetime(year, mon, day, hh, mm, ss).timestamp()
    lineTimestamp = getDateFromLine(line)
    if (lineTimestamp >= earliestTimestamp):
        return True
    else:
        return False

def getLogs(logType=None):
    fullDict = {}

    if logType == None or logType == "armore":
        logsArmore = getLogsArmore()
        fullDict['armore'] = logsArmore
    if logType == None or logType == "bro":
        logsBro = getLogsBro()
        fullDict['bro'] = logsBro
    if logType == None or logType == "zmq":
        logsZmq = getLogsZmq()
        fullDict['zmq'] = logsZmq
    if logType == None or logType == "netmap":
        logsNetmap = getLogsNetmap()
        fullDict['netmap'] = logsNetmap

    return fullDict

def reportLogs(logType=None):
    fullDict = getLogs(logType)

    fullJsonMsg = getJsonStrFormatted(fullDict)
    return fullJsonMsg

def reportLogsSince(logType=None, year=None, mon=None, day=None, hh=None, mm=None, ss=None):
    fullDict = getLogs(logType)
    filteredDict = {}

    if year:
        for key in fullDict:
            filteredDict[key] = []
            for line in fullDict[key]:
                if entryAllowed(line, int(year), int(monToNum(mon)), int(day), int(hh), int(mm), int(ss)):
                    filteredDict[key].append(line)
    else:
        filteredDict = fullDict

    fullJsonMsg = getJsonStrFormatted(filteredDict)
    return fullJsonMsg

