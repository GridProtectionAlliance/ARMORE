# # # #
# ipfw.py
# # University of Illinois/NCSA Open Source License
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
from crontab import CronTab
import re

DOW = {"SUN": 0, "MON": 1, "TUE": 2, "WED": 3, "THU": 4, "FRI": 5, "SAT": 6}

def flush():
    cmd("ipfw -q flush")

def getProtoPort(proto):

    if proto == "modbus":
        theFile = getSupportFilePath("ModbusMain")
    elif proto == "dnp3":
        theFile = getSupportFilePath("Dnp3Main")
    else:
        return "any"

    for l in [x.rstrip("\n") for x in open(theFile, 'r').readlines()]:
        mObject = re.match(r'^const ports = {(.*)}', l)
        if mObject:
            return ",".join([x.split('/')[0].strip(' ') for x in mObject.groups(1)[0].strip(' ').split(',')])

    return "any"

def parseStartDays(days):
    newStartDays = []
    for x in range(len(days)):
        currDay = days[x]
        prevDay = (days[x]-1) % 7

        if prevDay not in days:
            newStartDays.append(currDay)

    return ",".join([str(x) for x in newStartDays])

def parseStopDays(days):
    newStopDays = []
    for x in range(len(days)):
        currDay = days[x]
        nextDay = (days[x]+1) % 7

        if nextDay not in days:
            newStopDays.append(nextDay)

    return ",".join([str(x) for x in newStopDays])

def parseTime(timePer):

    days = set()
    start = set()
    stop = set()

    print(timePer)
    cronRet = {"start": [], "stop": [], "all": False}
    entries = timePer.split(',')
    for e in entries:
        day, startHour, startMin, startSec, stopHour, stopMin, stopSec = e.split(':')
        result = {"day": set(), "start": {}, "stop": {}}

        if day.lower() == "daily" and int(startHour) == 00 and int(startMin) == 00 and int(startSec) == 00 and int(stopHour) == 23 and int(stopMin) == 59 and int(stopSec) == 59:
           cronRet["all"] = True
        else:
            if day.lower() == "daily":
                days = {0, 1, 2, 3, 4, 5, 6}
            elif day.lower() == "weekdays":
                days.update({DOW["MON"], DOW["TUE"], DOW["WED"], DOW["THU"], DOW["FRI"]})
            elif day.lower() == "weekend":
                days.update({DOW["SAT"], DOW["SUN"]})
            else:
                days.update({DOW[day.upper()]})

            start.add("{}:{}".format(startHour, startMin))
            stop.add("{}:{}".format(stopHour, stopMin))

    for st in start:
        cRet = {}
        if st == "00:00" and days != {0, 1, 2, 3, 4, 5, 6}:
            cRet["days"] = parseStartDays(list(days))
        else:
            cRet["days"] = ",".join([str(x) for x in days])
        cRet["hour"], cRet["min"] = st.split(':')
        cronRet["start"].append(cRet)

    for st in stop:
        cRet = {}
        newDays = []
        if st == "23:59":
            cRet["days"] = parseStopDays(list(days))
            st = "00:00"
        else:
            cRet["days"] = ",".join([str(x) for x in days])
        cRet["hour"], cRet["min"] = st.split(':')
        cronRet["stop"].append(cRet)

    print(cronRet)
    return cronRet

def createCron():
    ret = CronTab(user=False)
    ret.remove_all()
    return ret

def createCronJob(cron, ruleNum, timePer, srcIp="any", tarIp="any", ports="any", inOut="any", proto="all"):
    startCmd, stopCmd = ipfwRules(ruleNum, srcIp, tarIp, ports, proto=proto)

    startStop = parseTime(timePer) 
    if startStop["all"] == True:
        cmd(startCmd)
    else:
        jobStart = []
        jobStop = []

        print(startStop["start"])
        for x in range(len(startStop["start"])):
            jobStart.append(cron.new(command=startCmd, user="root"))
            jobStart[x].minute.on(startStop["start"][x]["min"])
            jobStart[x].hour.on(startStop["start"][x]["hour"])
            for i in [int(y) for y in startStop["start"][x]["days"].split(',')]:
                jobStart[x].dow.also.on(i)

        for x in range(len(startStop["stop"])):
            jobStop.append(cron.new(command=stopCmd, user="root"))
            jobStop[x].minute.on(startStop["stop"][x]["min"])
            jobStop[x].hour.on(startStop["stop"][x]["hour"])
            for i in [int(y) for y in startStop["stop"][x]["days"].split(',')]:
                jobStop[x].dow.also.on(i)

        cron.write(getSupportFilePath("PolicyCronFile"))

    return ruleNum, cron

def ipfwRules(ruleNum, srcIp="any", tarIp="any", ports="any", inOut="any", proto="all"):

    if proto == "modbus":
        ports = getProtoPort("modbus")
    elif proto == "dnp3":
        ports = getProtoPort("dnp3")

    proto = "all"

    startCmd = "/usr/bin/ipfw -q add {} deny {} from {} to {}".format(ruleNum, proto, srcIp, tarIp)
    if ports != "any":
        startCmd += " {}".format(ports)
    startCmd += " keep-state"

    stopCmd = "/usr/bin/ipfw -q delete {}".format(ruleNum)

    return startCmd, stopCmd


