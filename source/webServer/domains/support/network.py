# # # # #
# network.py
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

import domains.support.lib.networking as networkingLib
import domains.support.lib.common as comLib
import re
import time

def getInterfaceStats():
    ret = []
    f = open("/proc/net/dev", "r")
    data = f.readlines()[2:]
    ips = getInterfaceIps()
    for d in data:
        cols = re.split('[\t\s]+', d)
        cols[1] = (cols[1])[:-1]
        if not re.match(r'eth.*', cols[1]):
            continue
        info = {
            "name":         cols[1],
            "ip":           ips[cols[1]], 

            "rx_bytes":     cols[2],
            "rx_packets":   cols[3],
            "rx_errors":    cols[4],
            "rx_dropped":   cols[5],
            "rx_fifo":      cols[6],
            "rx_frame":     cols[7],
            "rx_compressed":cols[8],
            "rx_multicast": cols[9],

            "tx_bytes":     cols[10],
            "tx_packets":   cols[11],
            "tx_errors":    cols[12],
            "tx_dropped":   cols[13],
            "tx_fifo":      cols[14],
            "tx_frame":     cols[15],
            "tx_compressed":cols[16],
            "tx_multicast": cols[17],
        }
        ret.append(info)

    return ret

def getPrimaryIp():
    ret = []
    f = open("/proc/net/dev", "r")
    data = f.readlines()[2:]
    ips = getInterfaceIps()
    nameMatch = re.compile(".*eth.*")
    for d in data:
        cols = re.split('[\t\s]+', d)
        cols[1] = (cols[1])[:-1]
        name = cols[1]
        if name in ips:
            ip = ips[name]
        else: continue
        if nameMatch.match(name) and ip and ip != '127.0.0.1':
            return ip
    return '127.0.0.1'
    
def getConnections():
    conns = networkingLib.psutil.net_connections(kind="inet")
    ret = []
    families = {}
    types = {}
    states = {}
    for c in conns:
        res = {}
        res['pid'] = c.pid
        res['fd'] = c.fd
        res['family'] = c.family
        res['type'] = c.type
        if res['family'] == networkingLib.socket.AF_UNIX:
            res['local_addr_host'] = c.laddr
            res['remote_addr_host'] = c.raddr
        else:
            res['local_addr_host'] = c.laddr[0]
            res['local_addr_port'] = c.laddr[1]
            res['remote_addr_host'] = c.raddr[0] if c.raddr else ''
            res['remote_addr_port'] = c.raddr[1] if c.raddr else ''
        res['state'] = c.status
        ret.append(res)
        types[res['type']] = 1
        families[res['family']] = 1
        states[res['state']] = 1
    return ret, types.keys(), families.keys(), states.keys()

def getInterfaceIps():
    interfaceDict = {}
    interfaces = networkingLib.getInterfaces()
    for i in interfaces:
        interfaceDict[i] = networkingLib.getIpAddrFromInt(i)
    return interfaceDict

def clearIpsOfInterface(interface):
    theCmd = "sudo ip addr flush dev {0}".format(interface)
    comLib.cmd(theCmd)

def bringInterfaceDown(interface):
    theCmd = "sudo ip link set {0} down".format(interface)
    comLib.cmd(theCmd)
    clearIpsOfInterface(interface)

def bringInterfaceUp(interface):
    theCmd = "sudo ip link set {0} up".format(interface)
    comLib.cmd(theCmd)

def setInterfaceIp(interface, ip):
    theCmd = "sudo ip addr add {0} dev {1}".format(ip, interface)
    comLib.cmd(theCmd)

def setIps(ipAddrs):
    for interface in ipAddrs:
        bringInterfaceDown(interface)
    
    time.sleep(1)

    for interface in ipAddrs:
        setInterfaceIp(interface, ipAddrs[interface])

    time.sleep(1)

    for interface in ipAddrs:
        bringInterfaceUp(interface)
    
    time.sleep(1)
    comLib.cmd("sudo dhclient")

def getNetmaskFromInt(interface):
    try:
        return networkingLib.getNetmaskFromInt(interface)
    except:
        #print("# Unable to get netmask from interface '{}'".format(interface))
        return ""

def getIpAddrFromInt(interface):
    try:
        return networkingLib.getIpAddrFromInt(interface)
    except:
        #print("# Unable to get IP Address from interface '{}'".format(interface))
        return ""


def getInterfaces():
    return networkingLib.getInterfaces()
