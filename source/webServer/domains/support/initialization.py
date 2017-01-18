# # # #

# initialization.py
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

import os
import sys
import re
from string import Template
import domains.support.lib.common as comLib
import domains.support.network as netLib
import domains.support.system as sysLib
import domains.support.config as confLib

def updateTypesDb():
    tPath = comLib.getSupportFilePath("CollectdTypes")

    appendFile = False
    with open(tPath, 'r') as typesFile:
        typesLines = [x.rstrip() for x in typesFile.readlines()]
        if not re.search("func_latency", typesLines[-1]):
            appendFile = True

    if appendFile:
        with open(tPath, 'a') as typesFile:
            typesFile.write("func_latency\t\tlatency:GAUGE:0:U")

def enablePythonRrd():
    cPath = comLib.getSupportFilePath("CollectdConf")
    
    startIndex = -1
    stopIndex = -1
    lastBlankSpace = -1
    cLines = [x.rstrip() for x in open(cPath, 'r').readlines()]
    for x in range(len(cLines)):
        if re.match('^# ARMORE', cLines[x]) and startIndex == -1:
            startIndex = x
        elif re.match('^# ARMORE', cLines[x]) and startIndex != -1:
            stopIndex = x
        elif re.match('^$', cLines[x]) and startIndex == -1:
            lastBlankSpace = x

    newTxt = '''# ARMORE
<LoadPlugin python>
    Globals true
    Interval 900
</LoadPlugin>

<Plugin python>
    ModulePath "/var/webServer/domains/support"
    LogTraces true
    Interactive false
    import rrdDataParser
    <Module rrdDataParser>
    </Module>
</Plugin>
# ARMORE'''.split('\n')

    if startIndex == -1:
        cLines.extend(newTxt)
    else:
        cLines[startIndex:stopIndex+1] = newTxt

    with open(cPath, 'w') as cFile:
        cFile.write("\n".join(cLines))

    comLib.cmd("/etc/init.d/collectd restart")

def createArmoreDb():
    try:
        comLib.cmd("sh {}".format(comLib.getSupportFilePath("ArmoreDBInit")))
    except Exception as e:
        print("Error Creating armore.db: {}".format(e))

def createRrdSymLink():
    try:
        if not os.path.exists(comLib.getSupportFilePath("RrdServ")):
            comLib.cmd("ln -s {}/{}/ {}".format(comLib.getSupportFilePath("RrdLib"), sysLib.getHostname(), comLib.getSupportFilePath("RrdServ")))
    except Exception as e:
        print("Error Creating rrd symlink: {}".format(e))

def initAll(supportFilesPath=None):
    createArmoreDb()
    confLib.createArmoreModeConfigFiles(supportFilesPath)
    updateTypesDb()
    enablePythonRrd()
    createRrdSymLink()

if __name__ == "__main__":

    import lib.common as comLib
    import network as netLib

    initAll(supportFilesPath="/var/webServer/supportFiles.txt")



