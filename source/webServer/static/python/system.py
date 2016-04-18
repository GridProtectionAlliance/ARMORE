# # # # #
# system.py
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
sys.path.append('./lib')

from common import *
from memory import *
import config as confLib
import socket
import re
import psutil
import subprocess

def getUptime():
    last = cmd("uptime -p")
    uptime = " ".join(re.split('[\s|\t|\n]+', last)[1:])
    return uptime

def getHostname():
    return open('/etc/hostname', 'r').readlines()[0].rstrip()

def getUserStats():
    last = re.split('[\n]+', cmd("last")) 
    hostname = socket.gethostname()
    ret = []
    for l in last:
        l = list(filter(None, re.split('[\s|\t]', l)))
        if len(l) > 5 and l[1] == ":0" and l[7] + " " + l[8] + " " + l[9] == "still logged in":
            ret.append({
                "name": l[0],
                "started": l[3] + " " + l[4] + " " + l[5] + " " + l[6],
                "host": hostname
            })

    return ret

def getProcessCpuUsage(processName):
    usageFlt = 0.0
    try:
        usageArr = cmd("ps -C " + processName + " -o %cpu").split('\n')
        usageStr = usageArr[1]
        usageFlt = float(usageStr)
    except:
        None

    return usageFlt

def getProcessStatus(processName):
    status = "error"
    state = "stopped"
    try:
        statusCmdResp = cmd("pgrep -c -x " + processName).rstrip()
        if int(statusCmdResp) > 0:
            status = "success"
            state = "running"
    except subprocess.CalledProcessError as e:
        if e.returncode == 2:
            print("Invalid options specified for {0}: {1}".format(processName, e.cmd))
        if e.returncode == 3:
            print("Internal Error getting {0} status".format(processName))
    
    return status, state

def getProcessDict(processName):
    processInfo = getProcessStatus(processName)

    ret = {}
    ret["name"] = processName.capitalize() if processName.islower() else processName
    ret["status"] = processInfo[0]
    ret["state"] = processInfo[1]
    ret["cpuPercent"] = getProcessCpuUsage(processName)
    
    return ret

def getArmoreStatus():
    with open('/etc/network/interfaces') as f:
        modeLine = f.readlines()[0]

    ret = {}
    ret["name"] = re.match('(?:Proxy|Passive|Transparent)', modeLine, re.I)
    ret["status"] = "error"
    ret["state"] = "stopped"

    if ret["name"].lower() == 'proxy':
        ret =  systems.append(getProcessDict("ARMOREProxy"))
    elif ret["name"].lower() in ["transparent","passive"]:
        ret["status"] = "success"
        ret["state"] = "running"

    return ret
    
def getDashboardInfo():
    systems = []
    systems.append(getProcessDict("ARMOREProxy"))
    systems.append(getProcessDict("bro"))

    return systems

def getStatuses(inpSystems):
    output = []
    for inpSys in inpSystems:
        if inpSys.lower() in ["bro","armoreproxy"]:
            output.append(getProcessDict(inpSys))
        else:
            inp = {}
            inp["name"] = inpSys.capitalize() 
            currConf = confLib.getProxyConfig()
            inp["status"] = "warning" if currConf["Encryption"] == "Disabled" else "success"
            inp["state"] = "disabled" if currConf["Encryption"] == "Disabled" else "enabled"

            output.append(inp)
    return output

def getSwapStats():
    vmstat = re.split('[\s|\n|\t*]+', cmd("vmstat")) 
    free = re.split('[\s|\n|\t*]+', cmd("free -b")) 
    ret = {
        "total": free[19],
        "used": free[20],
        "percent": float(free[20])/float(free[19]),
        "free": free[21],
        "swapped_in": vmstat[29],
        "swapped_out": vmstat[30]
    }
    swapStats = psutil.swap_memory()
    total = swapStats[0]
    used = swapStats[1]
    free = swapStats[2]
    sin = swapStats[4]
    sout = swapStats[5]
    ret = {
        "total": total,
        "used": used,
        "percent": used/total * 100,
        "free": free,
        "swapped_in": sin,
        "swapped_out": sout
    }
    return ret

def getDiskStats():
    f = re.split('[\n]+', cmd("df"))[1:]
    ret = []
    for d in f:
        e = re.split('[\s|\t*|\n]+', d)
        if len(e) == 6:
            ret.append({
                "device": e[0],
                "mountpoint": e[5],
                "space_total": float(e[1]) * 1024,
                "space_used": float(e[2]) * 1024,
                "space_used_percent": e[4],
                "space_free": float(e[3]) * 1024
            })
    return ret

def getLoadAvg():
    f = open("/proc/loadavg", 'r')
    laa = f.readlines()[0]
    laa = [float(x) for x in re.split('[\s|\t|\n]+', laa)[:3]]
    return laa

def getCpuStats():
    cpuStats = psutil.cpu_times_percent()
    user = cpuStats[0]
    system = cpuStats[2]
    idle = cpuStats[3]
    iowait = cpuStats[4]

    cpu = {
        "user": user,
        "system": system,
        "idle": idle,
        "iowait": iowait
    }

    return cpu

def getMemoryStats():
    memStats = psutil.virtual_memory()

    total = memStats[0]
    buffered = memStats[7]
    cached = memStats[8]
    free = memStats[4]
    avail = memStats[1]

    ex_used = total - avail
    ex_used_percent = ex_used/total * 100
    in_used = total - free
    in_used_percent = in_used/total * 100

    memStats = {
        "total": total,
        "available": avail,
        "ex_used": ex_used,
        "ex_percent": ex_used_percent,
        "in_used": in_used,
        "in_percent": in_used_percent,
        "free": free
    }

    return memStats

def getNumOfCores():
    return psutil.cpu_count()

def sect(array, theSection):
    return array

def getProcessStats(sortStr=None, order=None, filter=None, pid=None, section=None):
    procs = []

    for p in psutil.process_iter():
        proc = {}
        proc["pid"] = p.pid
        proc["name"] = p.name()
        proc["user"] = p.username()
        proc["status"] = p.status()
        proc["created"] = timestampToPrettyDate(p.create_time())
        proc["mem_rss"] = p.memory_info()[0]
        proc["mem_vms"] = p.memory_info()[1]
        proc["mem_percent"] = p.memory_percent()
        proc["cpu_percent"] = p.cpu_percent()
        proc["cmdline"] = p.cmdline()
        procs.append(proc)
    if filter or pid:
        procs = filt(procs, filter, pid)
    if sortStr or order:
        procs = sortOrderBy(procs, sortStr, order)
    if section:
        procs = sect(procs, section)
            
    return procs

def getNumberOfProcs():
    procs = 0
    user_procs = 0
    for p in psutil.process_iter():
        procs += 1
        user_procs += 1 if p.username() != 'root' else 0

    return procs, user_procs

def getArmoreState(proxy=None, bro=None):
    if proxy is None:
        proxy = getProcessDict("ARMOREProxy")
    if bro is None:
        bro = getProcessDict("bro")
    
    status = "error"
    state = "stopped"
    if proxy["status"] == "success":
        if bro["status"] == "success":
            status = "success"
            state = 'running'
        else:
            status = "warning"
            state = "running with errors"
        
    ret = {}
    ret["name"] = 'ARMORE'
    ret["status"] = status
    ret["state"] = state
    
    return ret

def getCommonInfo(currUser, page):

    ret = {}
    ret["uptime"] = getUptime()
    ret["currUser"] = currUser
    ret["page"] = page
    ret["hostname"] = getHostname()

    return ret
