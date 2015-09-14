# # # # #
# log.py
#
# Contains methods used for reporting
# or writing logs
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

import re # Regex

def getLogsArmore():
    results = []
    lines = open("/var/log/syslog", 'r').readlines()
    for line in lines:
        if re.search(r'.*armore.*', line, re.I):
            results.append(line)
    return results

def getLogsBro():
    results = []
    lines = open("/var/log/syslog", 'r').readlines()
    for line in lines:
        if re.search(r'.*Bro.*', line, re.I):
            results.append(line)
    return results

def getLogsZmq():
    results = []
    lines = open("/var/log/syslog", 'r').readlines()
    for line in lines:
        if re.search(r'.*zmq.*', line, re.I):
            results.append(line)
    return results

def getLogsNetmap():
    results = []
    lines = open("/var/log/syslog", 'r').readlines()
    for line in lines:
        if re.search(r'.*netmap.*', line, re.I):
            results.append(line)
    return results

