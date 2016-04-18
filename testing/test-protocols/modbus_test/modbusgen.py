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
from netaddr import * 

import threading
import time
import random
import string
import socket
import signal
import sys
import os

#Server
from pymodbus.server.sync import StartTcpServer
from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

#Client
from pymodbus.client.sync import ModbusTcpClient


################################
##Modbus Identification
################################

store = ModbusSlaveContext(
        di = ModbusSequentialDataBlock(0, [17]*100),
        co = ModbusSequentialDataBlock(0, [17]*100),
        hr = ModbusSequentialDataBlock(0, [17]*100),
        ir = ModbusSequentialDataBlock(0, [17]*100))
context = ModbusServerContext(slaves=store, single=True)

identity = ModbusDeviceIdentification()
identity.VendorName  = 'ITI'
identity.ProductCode = 'PM'
identity.VendorUrl   = 'code.iti.illinois.edu'
identity.ProductName = 'Server Instance'
identity.ModelName   = 'ITI Test'
identity.MajorMinorRevision = '1.0'

################################
## Host Lists
################################


hosts       = {}  #dict of ipv4addrs available index->ipv4
ip_alloc    = {}  #ipv4addrs allocated ethx:x->ipv4
client_addr = {}  #dict of client addrs iface:ipv4
server_addr = {}  #dict of server addrs iface:ipv4

clientlist = []   #list of client threads
serverlist = []   #list of server threads
#-----------------------------------------------------------------------------#
# Helper Functions
#-----------------------------------------------------------------------------#

class serverthreads(threading.Thread):
    def __init__(self, ipaddr, port):
        threading.Thread.__init__(self)
        self.ipaddr = ipaddr
        self.port = port

    def run(self):
        StartTcpServer(context, identity=identity, address=(self.ipaddr,self.port))
        return

class clientthreads(threading.Thread):
    def __init__(self, ipaddr, port):
        threading.Thread.__init__(self)
        self.ipaddr = ipaddr
        self.port   = port
        self.clientstop = threading.Event()

    def run(self):
        comm=server_addr[random.randint(0,len(serverlist)-1)]
        client = ModbusTcpClient(comm, self.port, source_address=(self.ipaddr,0), retries=1, retry_on_empty=True)
        while(not self.clientstop.is_set()):
            client.write_coil(1, True)
            print "coil write from:" + self.ipaddr + " --->" + comm 
            time.sleep(random.randint(0,3))
        print "stopping"
        client.socket.shutdown(1)
        client.close()
        return

    def stop(self):
        self.clientstop.set()
        return self.clientstop.is_set()

def ipv4rngcidr(string):
    ipv4Range=IPNetwork(string)
    return ipv4Range

def validate_nic(nic):
    if(nic in netifaces.interfaces()):
        return 1
    else:
        return 0


def ip_exist(ipaddr,nic):
    #make sure nic doesnt already have that address in list
    nicAddr = netifaces.ifaddresses(nic)
    curAddr = nicAddr[netifaces.AF_INET]
    #list of dictionaries search for ip addresses
    if not any(d['addr'] == ipaddr for d in curAddr):
        return 0 #We could not find any allocated for interface
    else:
        return 1 #We found an allocated one so we gotta skip

def validate_ip(ipaddr,nic):
    ipcheck = ipaddr.split(".", 3)
    if(ipcheck[0] == '0' or ipcheck[3] == '0'):
        return 0
    elif '255' in ipcheck:
        return 0
    elif ip_exist(ipaddr,nic) == 1:
        return 0
    else:
        return 1

def allocate_address(ipaddr, nic, index):
    vnic = nic +":"+`index`
    alist = [vnic, str(ipaddr)]
    #add allocated ip address to a list to split
    ip_alloc[vnic] = ipaddr
    subprocess.call(["ifconfig"] + alist)
    return 1

def deallocate_address(nic, index):
    vnic = nic +":"+`index`
    print "Stopping: " + vnic
    alist = [vnic]
    subprocess.call(["ifconfig"] + alist + ["down"])
    return 1

def cleanup():
    for addressesallocated in range (len(hosts)):
        deallocate_address(curIface, addressesallocated)
    
    for clients in range (len(clientlist)):
        graceful = clientlist[clients].stop()
        while(not graceful):
            clientlist[clients].stop()
            time.sleep(3)
    return 1



def signal_handler(signal, frame):
    print('Cleaning up...')
    cleanup()
    print ('Waiting for threads to finish')
    time.sleep(7)
    os._exit(0) #Hack because serve_forever callback is broken


################################
##Input Parsing
################################

def main():
    parser = argparse.ArgumentParser(description='Generate ModBus Traffic.')
    parser.add_argument('clientinput', metavar='-c', type=int, help='Number of clients to simulate')
    parser.add_argument('serverinput', metavar='-s', type=int, help='Number of servers to simulate')
    parser.add_argument('interfaceinput', metavar='-i', help='Enter Interface i.e. eth0')
    parser.add_argument('iprangeinput', metavar='-r', help='IP address CIDR')
    parser.add_argument('portinput', metavar='-p',  type=int, default='502', help='Port (default 502)')
    parser.add_argument('delayinput', metavar='-d',  type=int, help='Delay in seconds')
#parser.add_argument('configfile', metavar='-f',   nargs='+', help='Specify file with options')
#parser.add_argument('manclientinput', metavar='-c', type=int, nargs='+', help='Comma Seperarted Client IP addresses')
#parser.add_argument('manserverinput', metavar='-s', type=int, nargs='+', help='Comma Seperated Server IP addresses')
    args = parser.parse_args()


    global curIface #stupid hack FIX ME
    numClients = (args.clientinput)
    numServers = (args.serverinput)
    curIface   = (args.interfaceinput)
    ipv4Range  = IPNetwork(args.iprangeinput)
    port       = (args.portinput)
    delay      = (args.delayinput)

#Does interface exist
    if validate_nic(curIface) == 0:
        print "Invalid Interface"
        exit()

################################
##IP allocation
################################
    ip_index = 0
    for virtnic in range (len(ipv4Range)):
        if validate_ip(str(ipv4Range[virtnic]),curIface) == 1:
            hosts[ip_index] = ipv4Range[virtnic]
            ip_index=ip_index+1

    if(numClients+numServers > len(hosts)):
        print "Specified more clients and hosts then available IP's"
        exit()
    else:
        for ipalloc in range (len(hosts)):
            #Allocate addresses to virtual interfaces
            allocate_address(hosts[ipalloc],curIface,ipalloc)

##Divide ip addresses among users
    serverAlloc = numServers
    clientAlloc = numClients
    serverIndex = 0
    clientIndex = 0

    for divideip in range (len(hosts)):
        #allocate servers first
        if(serverAlloc > 0):
            server_addr[serverIndex] = str(hosts[divideip])
            serverAlloc = serverAlloc - 1
            serverIndex = serverIndex + 1
        elif(clientAlloc > 0):
            client_addr[clientIndex] = str(hosts[divideip])
            clientAlloc = clientAlloc - 1
            clientIndex = clientIndex + 1
        else:
            break


################################
##Starting Servers/Clients
################################

##ip addreses are now assigned time to start servers
#give them server_addr[xxx]
    
    ##Check num servers isn't 0, then start allocating threads
    for servers in range(0,len(server_addr)):  #range(0, numServers)
        t = serverthreads(server_addr[servers], port)
        serverlist.append(t)
        print "Server Spawned: " + server_addr[servers]
        t.start()

    ##Check num clients isn't 0, then start allocating threads
    for clients in range(0,len(client_addr)):  #range(0, numClients)
        t = clientthreads(client_addr[clients], port)
        clientlist.append(t)
        print "Client Spawned: " + client_addr[clients]
        t.start()

    signal.signal(signal.SIGINT, signal_handler)
    signal.pause()

if __name__ == '__main__':
    main()



