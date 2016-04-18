#!/usr/bin/python
###############################################################################
#University of Illinois/NCSA Open Source License
#Copyright (c) 2015 Information Trust Institute
#All rights reserved.
#
#Developed by: 		
#
#Information Trust Institute
#University of Illinois
#http://www.iti.illinois.edu
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of
#this software and associated documentation files (the "Software"), to deal with
#the Software without restriction, including without limitation the rights to 
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
#of the Software, and to permit persons to whom the Software is furnished to do 
#so, subject to the following conditions:
#
#Redistributions of source code must retain the above copyright notice, this list
#of conditions and the following disclaimers. Redistributions in binary form must 
#reproduce the above copyright notice, this list of conditions and the following
#disclaimers in the documentation and/or other materials provided with the 
#distribution.
#
#Neither the names of Information Trust Institute, University of Illinois, nor
#the names of its contributors may be used to endorse or promote products derived 
#from this Software without specific prior written permission.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
#FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS 
#OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
#WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
#CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
###############################################################################

import argparse
import netifaces
import subprocess
import os
import signal
import time

from netaddr import *

###############################################################################
#Globals
###############################################################################

config  = {} #Global Configuration Values Loaded From File
server  = [] #ServerIP:virtual interface
client  = [] #ClientIP:virtual interface

ipalloc = {} #list of allocated ipv4 addresses
iplist  = [] #list of available ipv4 addresses

###############################################################################
#Functions
###############################################################################
"""parseConfig  -- Read Configuration file for values into global config[]
ipRange        = input: ipRange/CIDR   (i.e. 192.168.5.0/24)
virtNIC        = input: nic            (i.e. eth0)
modbusPort     = input: port        (default 502)
avgComInt      = input: average communication in seconds
minComInt      = input: minimum communication in seconds
maxComInt
jitter
startPeriod
endPeriod
clientNum
clientAddr
serverNumLoc   = input: number of local servers (optional)
serverAddrLoc  = input: addresses of local servers (optional)
serverAddrRem  = input: addresses of remote servers (optional)
cMap
cMapRule  - NA
queryType - NA
"""
def parseConfig(configFile):
    for line in open(configFile, 'r'):
        line = line.replace(" ", "").rstrip("\n")
        if line.find("ipRange") > -1:
            config["ipRange"] = line.split("=", 1)[1]
        if line.find("virtNIC") > -1:
            config["virtNIC"] = line.split("=", 1)[1]
        if line.find("modbusPort") > -1:
            config['modbusPort'] = line.split("=", 1)[1]
        if line.find("avgComInt") > -1:
            config['avgComInt'] = line.split("=", 1)[1]
        if line.find("minComInt") > -1:
            config['minComInt'] = line.split("=", 1)[1]
        if line.find("maxComInt") > -1:
            config['maxComInt'] = line.split("=", 1)[1]
        if line.find("jitter") > -1:
            config['jitter'] = line.split("=", 1)[1]
        if line.find("startPeriod") > -1:
            config['startPeriod'] = line.split("=", 1)[1]
        if line.find("endPeriod") > -1:
            config['endPeriod'] = line.split("=", 1)[1]
        if line.find("clientNum") > -1:
            config['clientNum'] = int(line.split("=", 1)[1])
        if line.find("clientAddr") > -1:
            config['clientAddr'] = line.split("=", 1)[1]
        if line.find("serverNumLoc") > -1:
            config['serverNumLoc'] = int(line.split("=", 1)[1])
        if line.find("serverAddrLoc") > -1:
            config['serverAddrLoc'] = line.split("=", 1)[1]
        if line.find("serverAddrRem") > -1:
            config['serverAddrRem'] = line.split("=", 1)[1]
        if line.find("cMap") > -1:
            config['cMap'] = line.split("=", 1)[1]

"""validateNIC
Check interface to see it's valid since we're using virtual interfaces
"""
def validateNIC(virtNIC):
    if(virtNIC in netifaces.interfaces()):
        return 1 #This is a valid NIC
    else:
        return 0 #This is not a valid NIC

"""validateIP
Check ip address to make sure it isn't broadcast or zero or being used
"""
def validateIP(ipaddr, virtNIC):
    nicAddr = netifaces.ifaddresses(virtNIC)
    curAddr = nicAddr[netifaces.AF_INET]
    ipcheck = ipaddr.split(".", 3)
    if(ipcheck[0] == "0" or ipcheck[3] == '0' or '255' in ipcheck):
        return 0 #Invalid. Address was either broadcast or zero
    elif any(d['addr'] == ipaddr for d in curAddr):
        return 0 #Invalid. Address is being used
    elif any(ipaddr in s for s in config['serverAddrLoc'].split(",", 1)):
        return 0 #Invalid. Address is being used
    elif any(ipaddr in s for s in config['serverAddrRem'].split(",", 1)):
        return 0 #Invalid. Address is being used
    elif any(ipaddr in s for s in config['clientAddr'].split(",", 1)):
        return 0 #Invalid. Address is being used
    else:
        return 1 #This is a valid ipaddress we can use

"""initializeAddresses
Enumerate valid addresses
"""
def enumAddr():
    """Enumerate list of ip addresses available"""
    enumIP4= IPNetwork(config["ipRange"])
    for ip in range (len(enumIP4)):
        if(validateIP(str(enumIP4[ip]), config["virtNIC"])):
            iplist.append(str(enumIP4[ip]))

"""Allocate Addreses
Allocate addresses
"""
def allocateAddress(ipaddr, nic, vindex):
    vnic  = nic +":"+ vindex
    alist = [vnic, str(ipaddr)]
    ipalloc[vnic] = ipaddr
    subprocess.call(["ifconfig"] + alist)
    return 1

"""deallocate Addreses
deallocate addresses 
"""
def deallocateAddress(nic, vindex):
    vnic  = nic +":"+ vindex
    print "Stopping: " + vnic
    alist = [vnic]
    subprocess.call(["ifconfig"] + alist + ["down"])
    return 1

"""signalEvent
Handle ctrl-c and call specific routines
"""
def signalEvent(signal, frame):
    print "Exiting"
    exit()


###############################################################################
#Prog
###############################################################################
def main():
    parser = argparse.ArgumentParser(description='Generate ModBus Traffic')
    parser.add_argument('-c', '--config', help='Configuration file')
    args = parser.parse_args()
    configFile = args.config
    parseConfig(configFile)

    """Validate the interface, else exit"""
    if not(validateNIC(config["virtNIC"])):
        print "Invalid Interface"
        exit()

    """Create address space to autogenerate with removed manual/remote ips"""
    enumAddr()


    if(config['clientNum'] + config['serverNumLoc'] > len(iplist)):
     print "Specified more local servers and clients then available IP addresses"
     exit()

    if(config["serverNumLoc"] > 0):
     for servers in range (config["serverNumLoc"]):
      if len(ipalloc) == 0:
       vindex = `len(ipalloc) + servers`
      else:
       vindex = `servers`
       
      ipalloc[iplist[servers]] = config['virtNIC'] + ":" + vindex
      del iplist[servers]
      allocateAddress(iplist[servers], config['virtNIC'],vindex)

    if(config["clientNum"] > 0):
     print "blah"

    signal.signal(signal.SIGINT, signalEvent)
    signal.pause()

if __name__ == '__main__':
    main()



