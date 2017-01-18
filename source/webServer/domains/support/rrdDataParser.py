import collectd
import random
import re
from datetime import datetime, timedelta
import time
import os
import gzip

LASTREADDATETIME = None
LINKS = {}

def getDateTimeFromLogLine(line):
    return datetime.fromtimestamp(float(line.split('\t')[1]))

def getLinesFromFile(theFile, start, stop):

    #collectd.notice("# Getting lines from {}".format(theFile))
    usePreviousLog = False

    linesUsed = []
    if os.path.exists(theFile):
        if re.search("gz", theFile):
            fileLines = [x.rstrip() for x in gzip.open(theFile, "rt").readlines()]
        else:
            fileLines = [x.rstrip() for x in open(theFile, "r").readlines()]

        lines = [l for l in fileLines if not re.match("^#", l)][::-1]
        if getDateTimeFromLogLine(lines[-1]) > start:
            usePreviousLog = True

        linesUsed = [l for l in lines if getDateTimeFromLogLine(l) > start and getDateTimeFromLogLine(l) <= stop]
    else:
        collectd.notice("# File {} does not exist".format(theFile))
        usePreviousLog = True

    return usePreviousLog, linesUsed

def getDateTimeStartStopFromFileName(fullFilePath):
    arr = fullFilePath.split('/')
    dateStr = arr[4]
    timeStr = arr[5].split('.')[1]

    dtStart = datetime.strptime("{} {}".format(dateStr, timeStr.split('-')[0]), "%Y-%m-%d %H:%M:%S")
    dtStop = datetime.strptime("{} {}".format(dateStr, timeStr.split('-')[1]), "%Y-%m-%d %H:%M:%S")

    return dtStart, dtStop

def includeFile(start, stop, fStart, fStop):

    if fStop < start or fStart > stop:
        return False

    if fStart <= start and (fStop > start and fStop <= stop):
        return True
    if (fStart >= start and fStart <= stop) and (fStop >= start and fStop <= stop):
        return True
    if (fStart >= start and fStart <= stop) and fStop > stop:
        return True

def getPreviousLogFiles(start, stop):

    #theDir = "/var/log/bro/{:04d}-{:02d}-{:02d}/".format(start.year, start.month, start.day)
    theDir = "/var/log/bro/"

    retFiles = []
    for root, dirs, files in os.walk(theDir):
        for f in files:
            fullFileName = os.path.join(root, f)
            if re.search("stats_count", fullFileName):
                fStart, fStop = getDateTimeStartStopFromFileName(fullFileName)
                if includeFile(start, stop, fStart, fStop):
                    retFiles.append(fullFileName)

    return retFiles

def getLinesToParse():
    global LASTREADDATETIME

    currentLogFile = "/var/log/bro/current/stats_count.log"

    stop = datetime.now()
    start = LASTREADDATETIME
    LASTREADDATETIME = stop
    if start is None:
        start = stop - timedelta(seconds=900)
        LASTREADDATETIME = start

    collectd.notice("# Start: {}".format(start))
    collectd.notice("# Stop: {}".format(stop))
    collectd.notice("# Curr Log File: {}".format(currentLogFile))

    usePreviousLog, linesUsed = getLinesFromFile(currentLogFile, start, stop)

    if usePreviousLog or True:
        previousLogFiles = getPreviousLogFiles(start, stop)
        for f in previousLogFiles:
            collectd.notice("# Curr Log File: {}".format(f))
            _, extraLines = getLinesFromFile(f, start, stop)
            linesUsed += extraLines

    return linesUsed

def parseData():
    global LINKS
    
    for line in getLinesToParse():
        periodId, ts, period, sndr, rcvr, proto, uid, func, tar, num, theBytes, isRequest, numResponse, avgDelay = line.split('\t')

        if not "-" in avgDelay:
            linkLocal = (re.sub('\.', '_', sndr), re.sub('\.', '_', rcvr))
            if linkLocal in LINKS:
                if proto in LINKS[linkLocal]:
                    if func in LINKS[linkLocal]:
                        LINKS[linkLocal][proto][func].append(float(avgDelay))
                    else:
                        LINKS[linkLocal][proto].update({func: [float(avgDelay)]})
                else:
                    LINKS[linkLocal].update({proto: {func: [float(avgDelay)]}})
            else:
                LINKS.update({linkLocal: {proto: {func: [float(avgDelay)]}}})

def configer(ObjConfiguration):
    collectd.notice('##### Python RRD: Configuring')

def initer():
    collectd.notice('##### Python RRD: Initializing')

def reader(input_data=None):
    global LINKS
    collectd.notice('##### Python RRD: Reading')

    parseData()

    dsType = "func_latency"
    for l in LINKS:
        plugin = "armore-" + "__".join(l)
        for p in LINKS[l]:
            for f in LINKS[l][p]:
                typeInstance = "{}_{}".format(p, f)
                collectd.notice("{} {} {}".format(dsType, typeInstance, plugin))
                v = collectd.Values(type=dsType, type_instance=typeInstance, plugin=plugin)
                v.values = [round(sum(LINKS[l][p][f])/len(LINKS[l][p][f]), 6)]
                v.interval = 900
                v.dispatch()

def writer(v1, data=None):
    for i in v1.values:
        print("{0} ({1}): {2}".format(v1.plugin, v1.type, i))

def shutdowner():
    collectd.notice('##### Python RRD: Shutting Down')

collectd.register_config(configer)
collectd.register_init(initer)
collectd.register_read(reader)
collectd.register_write(writer)
collectd.register_shutdown(shutdowner)
