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
from domains.support.lib.logsHelper import *
import domains.support.lib.common as comLib

def example_getLogsInfo():
    ret = []
    for root, dirs, files in os.walk('/var/log'):
        for f in files:
            res = {}
            theFile = os.path.join(root, f)
            if not includeFile(theFile):
                continue
            info = os.stat(theFile)
            res['path'] = theFile
            res['size'] = info[6]
            res['atime'] = timestampToPrettyDate(info[7])
            res['mtime'] = timestampToPrettyDate(info[8])
            res['type'] = getFileType(theFile)
            ret.append(res)

    return ret

def getLogsFrom(headerName, name, path, exclude=None, start=None, stop=None):
    ret = {}
    ret["name"] = name
    ret["headerName"] = headerName
    logList = []
    for root, dirs, files in os.walk(path):
        for f in files:
            res = {}
            theFile = os.path.join(root, f)
            if not includeFile(theFile, exclude):
                continue
            info = os.stat(theFile)
            res['path'] = theFile
            res['size'] = info[6]
            res['atime'] = timestampToPrettyDate(info[7])
            res['mtime'] = timestampToPrettyDate(info[8])
            res['type'] = getFileType(theFile)
            logList.append(res)
    ret["logList"] = logList
    return ret

def getBroLogs(headerName, name, path, exclude=None, start=None, stop=None, dirsOnly=True):
    ret = {}
    ret["name"] = name
    ret["headerName"] = headerName
    ret["dirsOnly"] = dirsOnly
    logList = []
    for root, dirs, files in os.walk(path):
        if dirsOnly:
            if len(dirs) > 0:
                logList += [os.path.join(root, x) for x in dirs]
        else:
            for f in files:
                res = {}
                theFile = os.path.join(root, f)
                if not includeFile(theFile, exclude):
                    continue
                info = os.stat(theFile)
                res['path'] = theFile
                res['size'] = info[6]
                res['atime'] = timestampToPrettyDate(info[7])
                res['mtime'] = timestampToPrettyDate(info[8])
                res['type'] = getFileType(theFile)
                logList.append(res)
    ret["logList"] = sorted(logList, reverse=True)

    return ret

def getPathsFrom(path, sortBy, orderBy):
    ret = []
    for root, dirs, files in os.walk(path):
        for f in files:
            res = {}
            theFile = os.path.join(root, f)
            info = os.stat(theFile)
            res["isFile"] = True
            res['path'] = theFile
            res['size'] = info[6]
            res['atime'] = timestampToPrettyDate(info[7])
            res['mtime'] = timestampToPrettyDate(info[8])
            #res['type'] = "File"
            res['type'] = getFileType(theFile)
            if re.search("[ASCII|gzip]", res['type']) and not re.search("dBase III DBT", res['type']):
                ret.append(res)

        for d in dirs:
            res = {}
            theDir = os.path.join(root, d)
            info = os.stat(theDir)
            res["isFile"] = False
            res['path'] = theDir
            res['size'] = info[6]
            res['atime'] = timestampToPrettyDate(info[7])
            res['mtime'] = timestampToPrettyDate(info[8])
            #res['type'] = "Dir"
            res['type'] = getFileType(theDir)
            ret.append(res)

        break

    return comLib.sortOrder(ret, sortBy, orderBy)

def getPathFromSubsystem(subsystem):

    if subsystem.lower() in ['proxy','system', 'all']:
        return '/var/log'
    if subsystem.lower() in ['bro']:
        return '/var/log/bro'
    if subsystem.lower() in ['mongodb']:
        return '/var/log/mongodb'

    return ""

def pathIsValid(path):
    return re.match('^\/var\/log.*', path) and os.path.exists(path)

#def getLogsInfo(subsystem='all', path=None, sort=None, order=None):
def getLogsInfo(path, sort=None, order=None):
    logs = []

    #if path is None:
    #    path = getPathFromSubsystem(subsystem)

    logs = getPathsFrom(path, sort, order)

    return logs

def getLogContent(filename):
    if filename:
        if os.path.exists(filename):
            theType = getFileType(filename)
            if re.match('.*gzip.*', theType, re.I):
                f = gzip.open(filename, 'rt')
            else:
                f = open(filename, 'r')
            try:
                return f.read()
            except IOError as e:
                return ''
            except UnicodeDecodeError as e:
                return ''

    return ''
    
def getLogContentFiltered(filename, filterBy, nextSrch):

    content = html.escape(getLogContent(filename))
    ret = {}
    x = 0

    matches = []
    for r in re.finditer(re.escape(filterBy), content):
        matches.append(r.start(0))

    if len(matches) == 0:
        matches.append(-1)
        nextSrch = 0
    else:
        nextSrch = nextSrch % len(matches)

    ret['position'] = matches[nextSrch]
    ret['content'] = content
    ret['buffer_pos'] = matches[nextSrch]
    ret['filesize'] = len(content)

    return getJsonStrFormatted(ret)

