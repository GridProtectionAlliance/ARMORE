# # # # #
# networking.py
#
# Contains methods used for generic
# networking functions
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

from os import popen
import netifaces
import iptc

# Returns a dictionary containing
# each interface on a machine and
# the IP address to which it is
# mapped

def getInterfaceIps():
    interfaceDict = {}
    interfaces = netifaces.interfaces()
    for i in interfaces:
        interfaceDict[i] = netifaces.ifaddresses(i)[2][0]['addr']
    return interfaceDict

def deleteChain(name="ARMORE"):
    table = iptc.Table(iptc.Table.FILTER)
    chain = None
    chain = iptc.Chain(table, name)
    if chain:
        print(chain.name)
        chain.delete()

def createChain(name):
    table = iptc.Table(iptc.Table.FILTER)
    chain = iptc.Chain(table, name)
    if not chain:
        chain = table.create_chain(name)

def blockTraffic(ipSrc="0.0.0.0/0", ipDst="0.0.0.0/0", portSrc=None, portDst=None, proto="tcp", chainName="INPUT"):
    chain = iptc.Chain(iptc.Table(iptc.Table.FILTER), chainName)
    rule = iptc.Rule()
    rule.protocol = proto
    rule.src = ipSrc
    rule.dst = ipDst

    match1 = rule.create_match(proto)
    if portDst:
        match1.dport = portDst
    if portSrc:
        match1.sport = portDst

    match2 = rule.create_match("comment")
    match2.comment = "Drop all packets from " + ipSrc + " going to " + ipDst

    rule.target = iptc.Target(rule, "DROP")

    chain.insert_rule(rule)

def flushTables(chainName="INPUT"):
    table = iptc.Table(iptc.Table.FILTER)
    chain = iptc.Chain(table, chainName)
    chain.flush()

#flushTables()
#flushTables(chainName="OUTPUT")
#blockAddress("172.16.213.1", proto="icmp")
