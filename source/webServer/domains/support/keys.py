# # # # #
# keys.py
#
# This file is used to serve up
# RESTful links that can be
# consumed by a frontend system
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

import domains.support.lib.common as comLib 
import os
import re

def getKeyFileNames(thePath):
    return [z[0] for x, y in comLib.getFileList(thePath).items() for z in y.items() if re.match(".*\.pub$", z[0])]

def getKeys(sort="name", order="asc"):

    remotePath = comLib.getSupportFilePath("RemoteKeys")
    localPath = comLib.getSupportFilePath("LocalKeys")

    remoteKeys = getKeyFileNames(remotePath)
    localKeys = getKeyFileNames(localPath)

    keys = []
    for r in remoteKeys:
        keys.append({"name": r, 'type': "remote", "mtime": comLib.timestampToPrettyDate(os.stat("{}/{}".format(remotePath, r))[8])})

    for l in localKeys:
        keys.append({"name": l, 'type': "local", "mtime": comLib.timestampToPrettyDate(os.stat("{}/{}".format(localPath, l))[8])})

    return comLib.sortOrder(keys, sort, order)

def getKeyContent(filename, keyType):

    filePath = "{}/{}".format(comLib.getSupportFilePath("RemoteKeys"), filename)
    if keyType.lower() == "local":
        filePath = "{}/{}".format(comLib.getSupportFilePath("LocalKeys"), filename)

    if os.path.exists(filePath):
        f = open(filePath, 'r')
        try:
            return f.read()
        except IOError as e:
            return ''
        except UnicodeDecodeError as e:
            return ''

    return ''
 
def verifyFileName(name, keyType):

    fileOK = False
    errs = []

    if keyType is None or keyType not in ["local", "remote"]:
        errs.append("Invalid key type: {}".format(keyType))
    
    if not re.match(".*\.pub$", name):
        errs.append("Invalid filename: {}.  File must end in .pub".format(name))

    msg = ""
    if len(errs) > 0:
        msg = "<br/>".join(errs)
    else:
        fileOK = True

    return fileOK, errs

def verifySaveOk(keyType, filename):
    thePath = comLib.getSupportFilePath("{}Keys".format(keyType.capitalize()))
    theFiles = getKeyFileNames(thePath)

    ret = []
    print(keyType, theFiles, filename)
    if keyType == "local" and len(theFiles) == 1:
        ret.append("ERROR: Local key already exists.  Please delete existing local key before adding a new one")
    
    if filename in theFiles:
        ret.append("ERROR: Key '{}' already exists in {} keys".format(filename, keyType))

    return ret

def saveFile(keyType, theFile):
    msg = verifySaveOk(keyType, theFile.filename)
    if len(msg) == 0:
        theFile.save("{}/{}".format(comLib.getSupportFilePath("{}Keys".format(keyType.capitalize())), theFile.filename))

    return "<br/>".join(msg)

def verifyDeleteOk(keyType, filename):
    thePath = comLib.getSupportFilePath("{}Keys".format(keyType.capitalize()))
    theFiles = getKeyFileNames(thePath)

    ret = ""
    if len(theFiles) == 1:
        ret = "Warning: Deleting the last {0} key prevents encrypted communication from working correctly.  Please upload a {0} public key to enable encrypted communiation".format(keyType)
    
    return ret

def deleteFile(keyType, filename):
    ret = verifyDeleteOk(keyType, filename)
    os.remove("{}/{}".format(comLib.getSupportFilePath("{}Keys".format(keyType.capitalize())), filename))
    
    return ret

