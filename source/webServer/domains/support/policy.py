# # # #
# policy.py
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

import domains.support.lib.common as comLib
import domains.support.lib.ipfw as ipfw
from domains.support.models import *
import re
import os
import os.path
import datetime as dt
from string import Template
from uuid import uuid4

LINETEMPLATE = "$ruleNum\t$name\tsrc=$src,dest=$dest\t$proto\t$func\t$time\t$severity\t$filtering\n"
POLICYCONFIGFILE = comLib.getSupportFilePath("PolicyConfig")
POLICYLOGFILE = comLib.getSupportFilePath("PolicyLog")

def verifyPolicyFileExists():
    writeFile = False
    if not os.path.isfile(POLICYCONFIGFILE):
        writeFile = True
    elif len(open(POLICYCONFIGFILE, 'r').readlines()) == 0:
        writeFile = True
    
    if writeFile:
        with open(POLICYCONFIGFILE, 'w') as f:
            f.write("#fields\truleNum\tname\tip\tproto\tfunc\ttimePer\tseverity\tfiltering\n")

def getNextPolicyNumber():

    f = open(POLICYCONFIGFILE, 'r+')
    lastLine = f.readlines()[-1]
    f.close()

    if re.match('^#', lastLine):
        nextNum = 1
    else:
        nextNum = int(lastLine.rstrip('\n').split('\t')[0]) + 1

    return nextNum

def parseIPFW():

    ipfw.flush()

    lines = [x.rstrip('\n') for x in open(os.path.realpath(POLICYCONFIGFILE), 'r').readlines()]

    ruleNum = 0
    if len(lines) <= 1:
        return
    else:
        cron = ipfw.createCron()
        for l in lines[1:]:
            ruleId, name, ip, proto, func, timePer, severity, filtering = l.split('\t')
            srcIp, tarIp = [x.split('=')[1] for x in ip.split(',')]
            if filtering == 'active':
                ruleNum, cron = ipfw.createCronJob(cron, ruleNum, timePer, srcIp=srcIp, tarIp=tarIp, proto=proto)
                ruleNum += 1

def updatePolicyConfig(formData, num=None):

    verifyPolicyFileExists()

    times = {}
    lt = Template(LINETEMPLATE)

    if num is None or "submitAsNew" in formData:
        #nextNum = getNextPolicyNumber()
        nextNum = str(uuid4())
    else:
        nextNum = num

    for fd in formData:

        t = re.search("time(Start|Stop)(\d+)", fd)

        if t:
            startStop = t.group(1).lower()

            num = t.group(2)

            theTimeStr = formData[fd]
            if theTimeStr == "" and startStop == "start":
                theTimeStr = "00:00:00"
            if theTimeStr == "" and startStop == "stop":
                theTimeStr = "23:59:59"

            if num in times:
                times[num][startStop] = theTimeStr
            else:
                times.update({num: {startStop: theTimeStr}})

    timeStr = ""
    for t in times:
        if 'Daily' in formData:
            timeStr += "Daily:{}:{},".format(times[t]["start"], times[t]["stop"])
        else:
            if 'Weekday' in formData:
                timeStr += "Weekday:{}:{},".format(times[t]["start"], times[t]["stop"])
            else:
                if 'Mon' in formData:
                    timeStr += "Mon:{}:{},".format(times[t]["start"], times[t]["stop"])
                if 'Tue' in formData:
                    timeStr += "Tue:{}:{},".format(times[t]["start"], times[t]["stop"])
                if 'Wed' in formData:
                    timeStr += "Wed:{}:{},".format(times[t]["start"], times[t]["stop"])
                if 'Thu' in formData:
                    timeStr += "Thu:{}:{},".format(times[t]["start"], times[t]["stop"])
                if 'Fri' in formData:
                    timeStr += "Fri:{}:{},".format(times[t]["start"], times[t]["stop"])

            if 'Weekend' in formData:
                timeStr += "Weekend:{}:{},".format(times[t]["start"], times[t]["stop"])
            else:
                if 'Sat' in formData:
                    timeStr += "Sat:{}:{},".format(times[t]["start"], times[t]["stop"])
                if 'Sun' in formData:
                    timeStr += "Sun:{}:{},".format(times[t]["start"], times[t]["stop"])

    src = formData["srcIp"]
    if src == "":
        src = "0.0.0.0/0"
    dest = formData["destIp"]
    if dest == "":
        dest = "0.0.0.0/0"
    func = formData["func"]
    regexType = formData["funcRegex"]
    if func == "":
        func = "*".format(regexType)
    func = "{}:{}".format(regexType, func)

    inputDict = {
        "ruleNum": nextNum,
        "name": formData["policyName"],
        "src": src,
        "dest": dest,
        "proto": re.sub('Any', '*', formData["proto"]),
        "func": func,
        "time": timeStr[:-1],
        "severity": formData["severity"],
        "filtering": formData["filtering"]
    }

    output = lt.substitute(inputDict)
    lines = open(POLICYCONFIGFILE, 'r').readlines()

    append = True
    for x in range(len(lines)):
        if lines[x].split('\t')[0] == nextNum:
            append = False
            lines[x] = output

    if append:
        lines.append(output)

    f = open(POLICYCONFIGFILE, 'w')
    f.write("".join(lines))
    f.close()

    parseIPFW()

def parseTimes(ts):
    ret = []
    d = {}
    p = {}
    for t in ts:
        day, startH, startM, startS, stopH, stopM, stopS = t.split(':')
        start = ":".join([startH, startM, startS])
        stop = ":".join([stopH, stopM, stopS])
        startStop = " to ".join([start, stop])

        if day in d:
            d[day].append(startStop)
        else:
            d[day] = [startStop]
        
        if startStop in p:
            p[startStop].append(day)
        else:
            p[startStop] = [day]

    if len(d) >= len(p):
        for x in sorted(p):
            dayStr = ",".join(p[x])
            ret.append("{} from {}".format(dayStr, x))
    else:
        for x in d:
            for y in d[x]:
                ret.append("{} from {}".format(x, y))

    return ret

def getPolicies():

    verifyPolicyFileExists()

    policies = []
    lines = [x.rstrip() for x in open(POLICYCONFIGFILE, 'r').readlines()[1:]]

    for l in lines:
        pol = {}
        num, name, ip, proto, func, time, severity, filtering = l.split('\t')
        pol["num"] = num
        pol["name"] = name
        pol["ip"] = [re.sub('0.0.0.0/0', "Any", " = ".join(x.split('='))) for x in ip.split(',')]
        pol["proto"] = proto.split(',')
        regex, funcText = func.split(':')
        pol["funcRegex"] = regex
        pol["func"] = funcText.split(',')
        ts = time.split(',')
        pol["time"] = parseTimes(ts)
        
        pol["severity"] = severity.capitalize()
        pol["filtering"] = filtering.capitalize()
        policies.append(pol)

    return policies

    return ""

def getPolicy(theId=None):
    verifyPolicyFileExists()
    lines = [x.rstrip() for x in open(POLICYCONFIGFILE, 'r').readlines()[1:]]

    policy = {}
    if theId is None:
        theId = -1

    for l in lines:
        num, name, ip, proto, func, time, severity, filtering = l.split('\t')

        if num == theId:
            policy["num"] = num
            policy["name"] = name
            policy["src"], policy["dest"] = [x.split('=')[1] for x in ip.split(',')]
            policy["proto"] = proto
            funcRegex, funcText = func.split(':')
            policy["funcRegex"] = funcRegex
            policy["func"] = funcText
            starts = []
            stops = []
            for t in time.split(','):
                day, startH, startM, startS, stopH, stopM, stopS = t.split(":")
                policy[day.lower()] = "on"
                currStart = ":".join([startH, startM, startS])
                currStop = ":".join([stopH, stopM, stopS])
                if 'timePer' in policy:
                    if not (currStart in starts and currStop in stops):
                        policy['timePer'].append({
                            "start": currStart,
                            "stop": currStop
                        })
                else:
                    policy['timePer'] = [{
                        "start": currStart,
                        "stop": currStop
                    }]
                starts.append(currStart)
                stops.append(currStop)
            policy["severity"] = severity
            policy["filtering"] = filtering
            break

    return policy

def delPolicy(num):

    lines = [x for x in open(POLICYCONFIGFILE, 'r').readlines() if x.split('\t')[0] != num]
    newLines = lines[0]
    i = 0
    for l in lines[1:]:
        i += 1
        num, name, ip, proto, func, timePer, severity, filtering = l.rstrip('\n').split('\t')
        #num = i
        newLines += "{}\n".format("\t".join([str(num), name, ip, proto, func, timePer, severity, filtering]))
    f = open(POLICYCONFIGFILE, 'w')
    f.write(newLines)
    f.close()

    parseIPFW()

def parseEntry(entry):
    parsedEntries = []
    reasons = entry.split(',')
    for r in reasons:
        if re.match('Time', r):
            label, val = r.split('=')
            valDt = dt.datetime.fromtimestamp(float(val))
            valFormatted = valDt.strftime("%A @ %H:%M:%S.%f")
            parsedEntries.append("{}: {}".format(label, valFormatted))
        else:
            newR = re.sub('=', ': ', r)
            if re.match('[Protocol|Function]', newR):
                newR = re.sub('\*', 'unknown', newR)
                newR = re.sub(';', ',', newR)
            parsedEntries.append(newR)

    return parsedEntries

def toTuple(t, a):
    if type(a) is list:
        for x in a:
            t = toTuple(t, x)
    else:
        t += (a,)

    return t

def getPolName(polId):

    with open(POLICYCONFIGFILE, 'r') as f:
        for l in [x.strip('\n') for x in f.readlines()]:
            if polId == l.split('\t')[0]:
                return l.split('\t')[1]
    
    return ""

def getLogEvents(orderBy="timestamp", sort="desc", theFilter="all", severity="all", amt=100):
    #try:
    #if theFilter == "acked":
    #    logs = getAckedPolicyEvents(amt, sort, order)
    #elif theFilter == "unacked":
    #    logs = getUnAckedPolicyEvents(amt, sort, order)
    #else:
    #    logs = getAllPolicyEvents(amt, sort, order)
    #except:
    #    logs = []

    logs = getPolicyEvents(amt, orderBy=orderBy, sort=sort, theFilter=theFilter, severity=severity)
    ret = []
    if logs is None:
        logs = []

    for l in logs:
        log = {}
        log["timestamp"] = dt.datetime.fromtimestamp(float(l.ts))
        log["severity"] = l.severity
        log["entry"] = parseEntry(l.reason)
        log["polNum"] = l.polNum
        log["polName"] = getPolName(l.polNum)
        log["ackBy"] = uidToUsername(l.acknowledgedBy)
        log["uid"] = l.uid  

        ret.append(log)

    return ret

def runTestPcap():
    
    os.system("rm armore_policy.log; rm database/policy.sqlite; sudo bro -Cr /home/armore/armore/source/webServer/policy/mthread_10800.pcap /home/armore/armore/source/webServer/policy/policyLogging.bro")
    #comLib.cmd("sudo bro -Cr /home/armore/armore/source/webServer/BroStuff/mthread_10800.pcap /home/armore/armore/source/webServer/BroStuff/test.bro", False)

def getLogCounts():
    counts = {}
    counts["all"] = getAllPolicyEventsCount()
    counts["acked"] = getAckedPolicyEventsCount()
    counts["unacked"] = getUnAckedPolicyEventsCount()

    return counts

