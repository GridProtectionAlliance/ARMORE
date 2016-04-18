import argparse
import netifaces
import subprocess
import os
import signal
import time
import random
import threading

from netaddr import *

# Modbus Server Libraries
from pymodbus.server.sync import StartTcpServer
from pymodbus.server.sync import ModbusTcpServer
from pymodbus.transaction import *
from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

# Modbus Client Libraries
from pymodbus.client.sync import ModbusTcpClient

###############################################################################
# Globals
###############################################################################

config = {}
tmpserv = []  #remove me later
tmpclient = []  #remove me later
tmpclientw = []  #remove me later

###############################################################################
# Modbus Datastore Configuration
###############################################################################

store = ModbusSlaveContext(
        di = ModbusSequentialDataBlock(0, [17]*100),
        co = ModbusSequentialDataBlock(0, [17]*100),
        hr = ModbusSequentialDataBlock(0, [17]*100),
        ir = ModbusSequentialDataBlock(0, [17]*100))
context = ModbusServerContext(slaves=store, single=True)

identity = ModbusDeviceIdentification()
identity.VendorName = 'ITI'
identity.ProductCode = 'PM'
identity.VendorUrl = 'code.iti.illinois.edu'
identity.ProductName = 'Server Instance'
identity.ModelName = 'ITI Test'
identity.MajorMinorRevision = '1.0'

###############################################################################
# Functions
###############################################################################



def validateIface(iface):
    if not (iface in netifaces.interfaces()):
        print iface + " is not a valid interface"
        exit()

"""validateIP
Check ip address to make sure it isn't broadcast or zero or being used
"""


def validateIP(ipaddr, virtNIC):
    nicAddr = netifaces.ifaddresses(virtNIC)
    curAddr = nicAddr[netifaces.AF_INET]
    ipcheck = ipaddr.split(".", 3)
    if(ipcheck[0] == "0" or ipcheck[3] == '0' or '255' in ipcheck):
        return 0  # Invalid. Address was either broadcast or zero
    elif any(d['addr'] == ipaddr for d in curAddr):
        return 0  # Invalid. Address used by OS
    else:
        return 1  # Valid we can use this

"""serverthreads
class of ipobjects
"""


class serverthreads(threading.Thread):
    ipalloc = []   # class list of ipaddresses in use

    def __init__(self, vnic, ipaddr, port):
        threading.Thread.__init__(self)
        self.ipaddr = ipaddr  # ip address
        self.port = port  # port address
        self.vnic = vnic  # virtual nic
        self.mode = ""  # server or client
        self.state = ""  # up or down
        self.serverstop = threading.Event()
        self.server = ""
        self.framer = ""

    def run(self):
            self.serverint()

    def serverint(self):  # instantiate server stuff
        #print "Server mode"
        # StartTcpServer(context, identity=identity, address=(self.ipaddr, self.port))
        self.framer = ModbusSocketFramer
        self.server = ModbusTcpServer(context, self.framer, identiy=identity, address=(self.ipaddr, self.port))
        self.server.timeout = 1
        while(not self.serverstop.is_set()):
            self.server.handle_request()  # The original function overrides socketserver->serve_forever()
        # self.server.serve_forever()
        # self.server.shutdown()


    def alloc(self):  # Allocate ip address
        if(self.ipaddr in self.ipalloc):
            print "Address in-use"
            return 0
        else:
            self.ipalloc.append(self.ipaddr)
            cmdargs = [self.vnic, self.ipaddr]
            subprocess.call(["ifconfig"] + cmdargs)
            self.state = "up"
            return 1

    def dealloc(self):  # De-allocate ip address
        self.ipalloc.remove(self.ipaddr)
        cmdargs = [self.vnic]
        subprocess.call(["ifconfig"] + cmdargs + ["down"])

    def stop(self):
        self.serverstop.set()
        return

"""clientthreads
class of ipobjects
"""


class clientthreads(threading.Thread):

    def __init__(self, vnic, ipaddr, port):
        threading.Thread.__init__(self)
        self.ipaddr = ipaddr  # ip address
        self.port = port  # port address
        self.vnic = vnic  # virtual nic
        self.mode = ""  # server or client
        self.state = ""  # up or down
        self.dest = ""  # destination address for client
        self.clientstop = threading.Event()
        self.server = ""
        self.client = ""
        self.framer = ""
        self.vnicm = ""
        self.runtime= 0
        self.delayr = random.uniform(0,5)
        self.delayw = random.uniform(0,60)
        self.firstdelay = 0
        self.pstart= ""


    def run(self):
        self.client = ModbusTcpClient(self.dest, self.port, source_address=(self.ipaddr, 0), retries=1, retry_on_empty=True)
        if(self.mode=="read"):
            self.clientintr()
        elif(self.mode=="write"):
            self.clientintw()
        else:
            print "wrong mode specified"

    def clientintr(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() - self.pstart > self.runtime):
                print "stopping"
                break
            if(self.firstdelay < 1):
                print "Start RDelay is: " + str(self.delayr)
                time.sleep(self.delayr)
                self.firstdelay = 1
                print "Starting Reads"

            self.clientreads()
            print "\n\r-----read-----\n\r"
            print self.dest 
            print time.time() - self.pstart
            print "------------------\n\r"
    
    def clientintw(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() - self.pstart > self.runtime):
                print "stopping"
                break
            if(self.firstdelay < 1):
                print "Start WDelay is: " + str(self.delayw)
                time.sleep(self.delayw)
                self.firstdelay = 1
                print "Starting Writes"

            self.clientwrites()
            print "\n\r-----write----\n\r"
            print self.dest 
            print time.time() - self.pstart
            print "------------------\n\r"


    def clientreads(self):
        self.client.read_coils(1, 10)
        self.client.read_discrete_inputs(1, 10)
        self.client.read_holding_registers(1, 10)
        self.client.read_input_registers(1, 10)
        time.sleep(5)

    def clientwrites(self):
        self.client.write_coil(1, True)
        self.client.write_register(1, 3)
        self.client.write_coils(1, [True]*10)
        self.client.write_registers(1, [3]*10)
        time.sleep(60)
    
    def alloc(self):  # Allocate ip address
        if (validateIP(self.ipaddr, self.vnicm)):
            cmdargs = [self.vnic, self.ipaddr]
            subprocess.call(["ifconfig"] + cmdargs)
        else:
            return 0

    def dealloc(self):  # De-allocate ip address
        cmdargs = [self.vnic]
        subprocess.call(["ifconfig"] + cmdargs + ["down"])

    def stop(self):
        self.clientstop.set()
        return


"""signalEvent
Handle ctrl-c and call specific routines
"""


def signalEvent(signal, frame):
    print "Exiting"
    for servinst in range(len(tmpserv)):
        tmpserv[servinst].dealloc()
    tmpclient[0].dealloc()
    os._exit(0)  # Hack because serve_forever callback is broken


###############################################################################
# Prog
###############################################################################
ipRange = "192.168.5.0/24"
virtNIC = "eth0"
modbusPort = "502"
runtime = 10800 #time in seconds

def main():


    enumIP4 = (IPNetwork(ipRange))
    #for ip in range(len(enumIP4)):
    #for ip in range(8,109):
    for ip in range(8,18):
        tmpserv.append(serverthreads("eth0:"+str(ip), str(enumIP4[ip]), 502))

    for servinst in range(len(tmpserv)):
        tmpserv[servinst].mode = "server"
        tmpserv[servinst].alloc()
        tmpserv[servinst].start()
        print "server at " + tmpserv[servinst].ipaddr

    for clientinstr in range(0,len(tmpserv)):  #range(0, numClients)
        tmpclient.append(clientthreads("eth0:190", "192.168.5.190", 502))
        tmpclient[clientinstr].mode = "read"
        tmpclient[clientinstr].pstart= time.time()
        tmpclient[clientinstr].vnicm= virtNIC
        tmpclient[clientinstr].runtime= runtime
        tmpclient[clientinstr].dest = tmpserv[clientinstr].ipaddr
        tmpclient[clientinstr].alloc()
        tmpclient[clientinstr].start()
    
    for clientinstr in range(0,len(tmpserv)):  #range(0, numClients)
        tmpclientw.append(clientthreads("eth0:190", "192.168.5.190", 502))
        tmpclientw[clientinstr].mode = "write"
        tmpclientw[clientinstr].pstart= time.time()
        tmpclientw[clientinstr].vnicm= virtNIC
        tmpclientw[clientinstr].runtime= runtime
        tmpclientw[clientinstr].dest = tmpserv[clientinstr].ipaddr
        tmpclientw[clientinstr].alloc()
        tmpclientw[clientinstr].start()
    


    #print "------cut here-------"
    # alloc here
    signal.signal(signal.SIGINT, signalEvent)
    signal.pause()

if __name__ == '__main__':
    main()
