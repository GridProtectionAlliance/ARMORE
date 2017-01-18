#!/usr/bin/python
"""

University of Illinois/NCSA Open Source License Copyright (c) 2015 Information
Trust Institute All rights reserved.

Developed by:

Information Trust Institute University of Illinois http://www.iti.illinois.edu

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal with
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimers. Redistributions in binary
form must reproduce the above copyright notice, this list of conditions and the
following disclaimers in the documentation and/or other materials provided with
the distribution.

Neither the names of Information Trust Institute, University of Illinois, nor
the names of its contributors may be used to endorse or promote products
derived from this Software without specific prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
SOFTWARE.

"""

import subprocess
import random
import time
import argparse
import os
import netifaces
from netaddr import *
from collections import defaultdict
import threading
import signal

#Server
from pymodbus.server.sync import StartTcpServer
from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

#Client
from pymodbus.client.sync import ModbusTcpClient

"""
modbusnetwork:
    This class takes in the amount of ip addresses needed locally as well as
    any manually entered ip addresses and creates methods to allocate,
    deallocate, and otherwise track the objects
    NetSegment: IPv4 Address Space/CIDR
    LocalAddrsAlloc: Number of Local Address to Alloc
    RemoteAddrDef: Remote Server Addresses Manually Defined



"""
class modbusnetwork():
    node = []
    def __init__(self):
        self.ipaddr = None
        self.port = None
        self.interface = None
        self.role = None
        self.location = None #Local & Remote. If remote we avoid allocating
        self.status = None #Taken or Not

    #Populate Object Info
    def allocate(self, ipv4addr, port, interface, role, location):
            self.ipaddr = ipv4addr
            self.port = port
            self.interface = interface
            self.role = role
            self.location = location
            if location is "Remote": 
                self.status = 1
            else:
                self.status = 0 
            modbusnetwork.node.append(self) #Keep track of each instance

    #Return an instance 
    def assign(self):
        hostlist = self.node
        #First find an unused address, then allocate it, then run
        for x in range(len(hostlist)):
            if hostlist[x].location is "Local" and hostlist[x].status is 0:
                #Flag the address as used
                #Might be an issue with already used interface i.e eth0:0
                #Can do a check later with:
                #if VirtNic not in netifaces.interfaces():
                vnic = hostlist[x].interface + ":" + str(x + 1)
                hostlist[x].interface = vnic

                ipaddress = hostlist[x].ipaddr
                hostlist[x].status = 1

                #Check ipaddr is not allocated by system
                #if address not in netifaces.ifaddresses("eth0")[2]
                #run ifconfig and allocate address then set status to 1

                alist = [vnic, str(ipaddress)]
                subprocess.call(["ifconfig"] + alist)

                return hostlist[x]



    #Delete an instance and return nothing
    def deallocate(self):
        hostlist = self.node
        print "Deallocate IP Address"
        for modbusobjects in range(len(hostlist)):
            interface = str(hostlist[modbusobjects].interface)
            if ":" in interface:
                subprocess.call(["ifconfig"] + [interface] + ["down"])
                print interface
        self.status = 0


"""
client:
    This class takes in the amount of ip addresses needed locally as well as

"""

class client(threading.Thread):
    def __init__(self, ipaddr):
        threading.Thread.__init__(self)
        print "Configuring Network"
        self.configure = modbusnetwork()
        self.parameter = self.configure.assign()
        self.parameter.role = "Client"
        self.parameter.location= "Local"
        self.serveraddr = ipaddr
        self.clientstop = threading.Event()

    def run(self):
        print "and we're off!"
        clientaddr = str(self.parameter.ipaddr)
        clientport = str(self.parameter.port)
        serveraddr = str(self.serveraddr)

        print "Client at %s on port %s is connected to %s" % (clientaddr,
                clientport, serveraddr)

        client = ModbusTcpClient(serveraddr, clientport,
                source_address=(clientaddr,0), retries=1, retry_on_empty=True)
        while(not self.clientstop.is_set()):
            fnselect = random.randint(0,8)
            output = ""
            if fnselect is 0: 
                try:
                    output = client.write_coil(1, True)
                except:
                    print "Can't write coil"


            if fnselect is 1: 
                try:
                    output = client.write_register(1, 3)
                except:
                    print "Cannot Write Register"


            if fnselect is 2: 
                try:
                    output = client.write_coils(1, [True]*10)
                except:
                    print "Cannot Write Coils"

            if fnselect is 3: 
                try:
                     output = client.write_registers(1, [3]*10)
                except:
                    print "Cannot Write Registers"

            if fnselect is 4: 
                try:
                     output = client.write_registers(1, [4]*10)
                except:
                    print "Cannot Write Registers"

            if fnselect is 5: 
                try:
                     output = client.read_coils(1, 10)
                except:
                    print "Cannot read coils"

            if fnselect is 6: 
                try:
                     output = client.read_discrete_inputs(1, 10)
                except:
                    print "Cannot read discrete inputs"
            if fnselect is 7: 
                try:
                     output = client.read_holding_registers(1, 10)
                except:
                    print "Cannot read holding registers"

            if fnselect is 8: 
                try:
                      output = client.read_input_registers(1, 10)
                except:
                    print "Cannot read holding registers"

            time.sleep(random.randint(0,3))
            print output



            #print "coil write from: %s " % self.parameter.ipaddr
            time.sleep(random.randint(0,3))
        print "stopping"
        client.socket.shutdown(1)
        client.close()
        return

    def stop(self):
        self.clientstop.set()
        return self.clientstop.is_set()

    
    def assignserver(self):
        print "print here we go!"

"""
server:
    This class takes in the amount of ip addresses needed locally as well as

"""


class server(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        #Data Store for the server instance. Only set by connecting clients
        self.store = ModbusSlaveContext(
                di = ModbusSequentialDataBlock(0, [17]*100),
                co = ModbusSequentialDataBlock(0, [17]*100),
                hr = ModbusSequentialDataBlock(0, [17]*100),
                ir = ModbusSequentialDataBlock(0, [17]*100))
        self.context = ModbusServerContext(slaves=self.store, single=True)
        
        print self.store

        self.identity = ModbusDeviceIdentification()
        self.identity.VendorName  = 'ITI'
        self.identity.ProductCode = 'PM'
        self.identity.VendorUrl   = 'code.iti.illinois.edu'
        self.identity.ProductName = 'Server Instance'
        self.identity.ModelName   = 'ITI Test'
        self.identity.MajorMinorRevision = '1.0'
        self.configure = modbusnetwork()
        self.parameter = self.configure.assign()
        self.parameter.role = "Server"

    def run(self):
        ipaddress = str(self.parameter.ipaddr)
        port = int(self.parameter.port)
        interface = self.parameter.interface
        location = self.parameter.location
        print "Starting %s ModBus Server: %s:%s %s" % (location, ipaddress, port, interface)
        StartTcpServer(self.context, identity=self.identity, address=(ipaddress,port))
        return



def signal_handler(signal, frame):
    print('Cleaning up...')
    cleanup = modbusnetwork()
    cleanup.deallocate()

    print ('Waiting for threads to finish')
    time.sleep(7)
    os._exit(0) #Hack because serve_forever callback is broken

"""
Input Parsing

"""


def main():
    parser = argparse.ArgumentParser(description='Generate ModBus Traffic.')
    parser.add_argument('-c', '--clients', type=int, default=0,
                        help='Total Local Modbus Clients')
    parser.add_argument('-s', '--servers', type=int, default=0,
                        help='Total Local Modbus Servers')
    parser.add_argument('-r', '--remote', default=None,
                        help='Comma Seperated Remote Server IP Addresses ')
    parser.add_argument('-f', '--config', help='Configuration File')
    parser.add_argument('-i', '--interface',
                        help='Network Interface (i.e. eth0)')
    parser.add_argument('-p', '--port', default=502,
                        help='Modbus Port (Default 502)')
    parser.add_argument('-n', '--net', default="192.168.0.0/24",
                        help='IPv4/CIDR')
    parser.add_argument('-d', '--delay', default=1, type=int,
                        help='Communication Delay in Seconds')
    args = parser.parse_args()


# Validate Input. Configuration file overrides everything

    if args.config is not None:
        if not os.path.isfile(args.config):
            print "Invalid Configuration File"
            exit()
        else:
            print "parsing file"
            # Set Values from Configuration File
            # NumClients = args.clients
            # NumServers = args.servers
            # NumRemote = args.remote
            # VirtNic = args.interface
            # Port = args.port
            # Delay = args.delay
            # Net = IPNetwork(args.net)
            # Going to need ipaddr list for NumRemote
    else:
        # Set Values from ClI
        NumClients = args.clients
        NumServers = args.servers
        NumRemote = args.remote
        VirtNic = args.interface
        Port = args.port
        Delay = args.delay
        Net = IPNetwork(args.net)

# Validation Checks

    if NumRemote is not None:
        NumRemote = NumRemote.split(",")
        for x in range(len(NumRemote)):
            NumRemote[x] = IPAddress(NumRemote[x])
    else:
        NumRemote = []

    if VirtNic not in netifaces.interfaces():
        print "Network interface not specified or invalid"
        exit()

    if NumClients+NumServers > len(Net):
        print "Number of Clients and Servers bigger then Address Space"
        exit()

# Fill in modbusnetwork class with ip addresses to allocate
    for ipv4address in Net.iter_hosts():
        modbusinstance = modbusnetwork() #Fill in Valid addrs
        NICIface = netifaces.ifaddresses(VirtNic)[2]
        NICIface = NICIface[0]
        #System will likely have an addr in that range so skip it
        if str(ipv4address) != str(NICIface['addr']):
            if ipv4address in NumRemote:
                modbusinstance.allocate(ipv4address, Port, None, "Server", "Remote")
            else:
                modbusinstance.allocate(ipv4address, Port, VirtNic, None , "Local")


# Allocate servers
    for servers in range(NumServers):
        t = server()
        t.start()

# Enumerate all the server addresses
    serverlist  = []
    hostlist = modbusnetwork()
    hostlist = hostlist.node

    for x in range(len(hostlist)):
        if hostlist[x].role is "Server" and hostlist[x].ipaddr not in serverlist:
                serverlist.append(hostlist[x].ipaddr)

# Allocate Clients 
    index = 0
    #Balance servers and clients
    for clients in range(NumClients):
        t = client(serverlist[index])
        t.start()
        if index == len(serverlist)-1:
            index = 0
        else:
            index = index + 1


    signal.signal(signal.SIGINT, signal_handler)
    signal.pause()

if __name__ == '__main__':
    main()
