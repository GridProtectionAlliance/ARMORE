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
from pymodbus.other_message import * 
from pymodbus.file_message import * 

###############################################################################
# Globals
###############################################################################

config = {}
tmpserv = []  #remove me later
tmpclient = []  #remove me later
tmpclientw = []  #remove me later

tmpclientrw = []  #remove me later
tmpclientdiag = []  #remove me later

tmpclientanom = []  #remove me later

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
        self.delayw = random.uniform(0,30)
        self.delayrw = random.uniform(0,300)
        self.delaydiag = random.uniform(0,3600)
        self.firstdelay = 0
        self.pstart= 0
        self.anom4 = 0
        self.anom4a = 0
        self.anom6 = 0
        self.anom7 = 0


    def run(self):
        self.client = ModbusTcpClient(self.dest, self.port, source_address=(self.ipaddr, 0), Retries=9000, Reconnects=9000, Timeout=20, retry_on_empty=False)
        #self.client = ModbusTcpClient(self.dest, self.port, source_address=(self.ipaddr, 0), Retries=9000, Reconnects=9000, retry_on_empty=True)
        if(self.mode=="read"):
            self.clientintr()
        elif(self.mode=="write"):
            self.clientintw()
        elif(self.mode=="rw"):
            self.clientintrw()
        elif(self.mode=="diag"):
            self.clientintdiag()
        elif(self.mode=="anom"):
            self.clientintanom()
        elif(self.mode=="anom2"):
            self.clientintanom2()
        elif(self.mode=="anom3"):
            self.clientintanom3()
        elif(self.mode=="anom5"):
            self.clientintanom5()
        elif(self.mode=="anom8"):
            self.clientintanom8()
        elif(self.mode=="anom9"):
            self.clientintanom9()
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

            #introduce anomaly4 to 192.168.6.8
            if((self.dest== '192.168.6.8') and (self.pstart + 44700 < time.time()) and (self.anom4 < 1)):
                #print "Anomaly 4 Injected: "
                #print self.anom4
                #print time.time()
                if((self.dest== '192.168.6.8') and (self.pstart + 44700  + 1200 < time.time()) and (self.anom4 < 1)):
                    self.anom4 = 1 #Stop Anomaly 4
                    print "Anom 4 Ended"
                #print "Anomaly 4 End"
            else:
                if((self.dest== '192.168.6.8') and (self.pstart + 46860 < time.time()) and (self.anom6 < 1)):
                    self.clientwritesanom6()
                    print time.time()
                    print "Anomaly 6 Start"
                else:
                    self.clientwrites()
                print "\n\r-----write----\n\r"
                print self.dest 
                print time.time() - self.pstart
                print "------------------\n\r"
                if((self.dest== '192.168.6.8') and (self.pstart + 46860 + 480 < time.time()) and (self.anom6 < 1)):
                    self.anom6 = 1 #Stop Anomaly 6 after 200 seconds
                    print "Anomaly 6 Ended"
    
    def clientintrw(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() - self.pstart > self.runtime):
                print "stopping"
                break
            if(self.firstdelay < 1):
                print "Start RWDelay is: " + str(self.delayrw)
                time.sleep(self.delayrw)
                self.firstdelay = 1
                print "Starting Read Writes"
            if((self.dest== '192.168.6.8') and (self.pstart + 44700 < time.time()) and (self.anom4a < 1)):
                #print "Anomaly 4a Injected: " 
                #print self.anom4a
                #print time.time()
                if((self.dest== '192.168.6.8') and (self.pstart + 44700+ 1200< time.time()) and (self.anom4a < 1)):
                    self.anom4a = 1
                    print "Anomaly 4a Stop"
            else:
                self.clientrw()
                print "\n\r-----Read write----\n\r"
                print self.dest 
                print time.time() - self.pstart
                print "------------------\n\r"
    
    def clientintdiag(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() - self.pstart > self.runtime):
                print "stopping"
                break
            if(self.firstdelay < 1):
                print "Start Diag is: " + str(self.delaydiag)
                time.sleep(self.delaydiag)
                self.firstdelay = 1
                print "Starting Diag"

            self.clientdiag()
            print "\n\r-----Diag----\n\r"
            print self.dest 
            print time.time() - self.pstart
            print "------------------\n\r"
    
    def clientintanom(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() < self.pstart):
                time.sleep(1)
            else:
                print "Anomaly 1 Start"
                try:
                    self.client.read_coils(1, 10)
                except:
                    print "Could Not Read Coils AN1"

                try:
                    self.client.read_discrete_inputs(1, 10)
                except:
                    print "Could Not Read Discrete Inputs AN1"

                try:
                    self.client.read_holding_registers(1, 10)
                except:
                    print "Could not read holding registers AN1"

                try:
                    self.client.read_input_registers(1, 10)
                except:
                    print "Could not read input registers AN1"

                print "Anomaly 1 End"
                break
    
    def clientintanom2(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() < self.pstart):
                time.sleep(1)
            else:
                print "Anomaly 2 Start"
                subprocess.call(["iptables", "-I","INPUT", "-d", "192.168.6.8", "-j", "DROP"])
                time.sleep(1200)
                subprocess.call(["iptables", "-F"])
                print "Anomaly 2 End"
                break
    
    def clientintanom3(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() < self.pstart):
                time.sleep(1)
            else:
                print "Anomaly 3 Start"
                presult = subprocess.call(["ping", "-f", "-I", "192.168.6.8", "192.168.6.190", "-c", "100", "-q"])
                time.sleep(10)
                print presult
                print "Anomaly 3 End"
                break
    
    def clientintanom5(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() < self.pstart):
                time.sleep(1)
            else:
                print "Anomaly 5 Start"
                for i in range(0,10):
                    rq = ReadExceptionStatusRequest()
                    try:
                        self.client.execute(rq)
                    except:
                        print "Could not Read Exception AN5"
                    time.sleep(4)
                print "Anomaly 5 End"
                break
    
    def clientintanom8(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() < self.pstart):
                time.sleep(1)
            else:
                print "Anomaly 8 Start"
                #subprocess.call(["ipfw", "add", "pipe", "2", "src-ip", "192.168.6.8", "in"])
                #subprocess.call(["ipfw", "add", "pipe", "2", "src-ip", "192.168.6.8", "out"])
                #subprocess.call(["ipfw", "pipe", "2", "config", "bw", "60kbit/s", "queue", "20", "delay", "500ms"])
                subprocess.call(["ipfw", "add", "pipe", "2", "src-ip", "192.168.6.8"])
                subprocess.call(["ipfw", "pipe", "2", "config", "delay", "200ms"])
                presult = subprocess.call(["ipfw", "list"])
                print presult
                time.sleep(1200)
                #presult = subprocess.call(["ipfw", "-q", "flush"])
                subprocess.call(["ipfw", "-q", "delete", "00200"])
                #subprocess.call(["ipfw", "-q", "delete", "00300"])
                presult = subprocess.call(["ipfw", "list"])
                print presult
                print "Anomaly 8 End"
                break
    
    def clientintanom9(self):  # instantiate server stuff
        while(not self.clientstop.is_set()):
            if(time.time() < self.pstart):
                time.sleep(1)
            else:
                print "Anomaly 9 Start"
                #subprocess.call(["ipfw", "add", "prob", ".050", "drop", "ip", "from",  "192.168.6.8", "to" "any"])
                subprocess.call(["ipfw", "add", "pipe", "2", "src-ip", "192.168.6.8", "out"])
                #subprocess.call(["ipfw", "00200", "add", "prob", ".080", "drop", "ip", "from",  "192.168.6.8", "to", "any"])
                subprocess.call(["ipfw", "pipe", "2", "config", "plr", ".3"])
                presult = subprocess.call(["ipfw", "list"])
                print presult
                time.sleep(1200)
                #presult = subprocess.call(["ipfw", "-q", "flush"])
                subprocess.call(["ipfw", "-q", "delete", "00200"])
                presult = subprocess.call(["ipfw", "list"])
                print presult
                print "Anomaly 9 End"
                break


    def clientreads(self):
        if((self.dest== '192.168.6.8') and (self.pstart + 47460 < time.time()) and (self.anom7 < 1)):
            try:
                self.client.read_coils(11, 10) #Bits must be changed not 11,20 but 11, 10 bits
            except:
                print "Could not Read Coils 11-20 AN7"
            print time.clock() 
            print "Anomaly 7 \n"
        else:
            try:
                self.client.read_coils(1, 10)
            except:
                print "Could not Read Coils"

        try:
            self.client.read_discrete_inputs(1, 10)
        except:
            print "Could not read discrete inputs"

        try:
            self.client.read_holding_registers(1, 10)
        except:
            print "Could not read holding registers"
        
        try:
            self.client.read_input_registers(1, 10)
        except:
            print "Could not read input registers"

        time.sleep(5)
        if((self.dest== '192.168.6.8') and (self.pstart + 47460 + 300 < time.time()) and (self.anom7 < 1)):
            self.anom7 = 1 #Deactivate anomaly 7
            print " Anomaly 7 End \n"
            print time.clock() 
    def clientwrites(self):
        try:
            self.client.write_coil(1, True)
        except:
            print "Could not write coil"
        try:
            self.client.write_register(1, 3)
        except:
            print "Could not write register"
        #self.client.write_coils(1, [True]*10)
        #self.client.write_registers(1, [3]*10)
        time.sleep(30)
    
    def clientwritesanom6(self):
        try:
            self.client.write_coil(1, True)
        except:
            print "Could not write coil AN6"

        try:
            self.client.write_register(1, 3)
        except:
            print "Could not write register"

        #self.client.write_coils(1, [True]*10)
        #self.client.write_registers(1, [3]*10)
        time.sleep(5)
    
    def clientrw(self):
        try:
            self.client.write_coils(1, [True]*10)
        except:
            print "Could not write coils"
        #self.client.read_input_registers(1, 10)
        #self.client.write_registers(1, [4]*10)
        time.sleep(300)
    
    def clientdiag(self):
        try:
            self.client.write_registers(1, [4]*10)
        except:
            print "Could not write registers"
        #rq = ReadExceptionStatusRequest()
        #self.client.execute(rq)
        time.sleep(3600)
    
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
ipRange = "192.168.6.0/24"
virtNIC = "eth1"
modbusPort = "502"
#runtime = 18000#time in seconds
runtime = 53400#time in seconds for whole simulation
anom1time= 43260 #time in seconds for anomoly
anom3time= 43860#time in seconds for anomoly
#anom4 goes here @ 44700
anom5time= 46260 #time in seconds for anomoly
#anom6 goes here @ 46860
#anom7 goes here @ 47460 
anom8time= 48300 #time in seconds for anomoly
anom9time= 50100 #time in seconds for anomoly
anom2time= 51900 #time in seconds for anomoly




def main():


    enumIP4 = (IPNetwork(ipRange))
    #for ip in range(len(enumIP4)):
    for ip in range(8,18):
        tmpserv.append(serverthreads("eth1:"+str(ip), str(enumIP4[ip]), 502))

    for servinst in range(len(tmpserv)):
        tmpserv[servinst].mode = "server"
        tmpserv[servinst].alloc()
        tmpserv[servinst].start()
        print "server at " + tmpserv[servinst].ipaddr

    for clientinstr in range(0,len(tmpserv)):  #range(0, numClients)
        tmpclient.append(clientthreads("eth1:190", "192.168.6.190", 502))
        tmpclient[clientinstr].mode = "read"
        tmpclient[clientinstr].pstart= time.time()
        tmpclient[clientinstr].vnicm= virtNIC
        tmpclient[clientinstr].runtime= runtime
        tmpclient[clientinstr].dest = tmpserv[clientinstr].ipaddr
        tmpclient[clientinstr].alloc()
        tmpclient[clientinstr].start()
    
    for clientinstr in range(0,len(tmpserv)):  #range(0, numClients)
        tmpclientw.append(clientthreads("eth1:190", "192.168.6.190", 502))
        tmpclientw[clientinstr].mode = "write"
        tmpclientw[clientinstr].pstart= time.time()
        tmpclientw[clientinstr].vnicm= virtNIC
        tmpclientw[clientinstr].runtime= runtime
        tmpclientw[clientinstr].dest = tmpserv[clientinstr].ipaddr
        tmpclientw[clientinstr].alloc()
        tmpclientw[clientinstr].start()
    
    for clientinstr in range(0,len(tmpserv)):  #range(0, numClients)
        tmpclientrw.append(clientthreads("eth1:190", "192.168.6.190", 502))
        tmpclientrw[clientinstr].mode = "rw"
        tmpclientrw[clientinstr].pstart= time.time()
        tmpclientrw[clientinstr].vnicm= virtNIC
        tmpclientrw[clientinstr].runtime= runtime
        tmpclientrw[clientinstr].dest = tmpserv[clientinstr].ipaddr
        tmpclientrw[clientinstr].alloc()
        tmpclientrw[clientinstr].start()
    
    for clientinstr in range(0,len(tmpserv)):  #range(0, numClients)
        tmpclientdiag.append(clientthreads("eth1:190", "192.168.6.190", 502))
        tmpclientdiag[clientinstr].mode = "diag"
        tmpclientdiag[clientinstr].pstart= time.time()
        tmpclientdiag[clientinstr].vnicm= virtNIC
        tmpclientdiag[clientinstr].runtime= runtime
        tmpclientdiag[clientinstr].dest = tmpserv[clientinstr].ipaddr
        tmpclientdiag[clientinstr].alloc()
        tmpclientdiag[clientinstr].start()
   
    #Anomaly Threads
    tmpclientanom.append(clientthreads("eth1:191", "192.168.6.191", 502))
    tmpclientanom[0].mode = "anom"
    tmpclientanom[0].pstart= time.time() + anom1time+ 1
    tmpclientanom[0].vnicm= virtNIC
    tmpclientanom[0].runtime= 300
    tmpclientanom[0].dest = '192.168.6.8'
    tmpclientanom[0].alloc()
    tmpclientanom[0].start()

    tmpclientanom.append(clientthreads("eth1:190", "192.168.6.190", 502))
    tmpclientanom[1].mode = "anom2"
    tmpclientanom[1].pstart= time.time() + anom2time+ 1
    tmpclientanom[1].vnicm= virtNIC
    tmpclientanom[1].runtime= 300
    tmpclientanom[1].dest = '192.168.6.8'
    tmpclientanom[1].alloc()
    tmpclientanom[1].start()
    
    tmpclientanom.append(clientthreads("eth1:190", "192.168.6.190", 502))
    tmpclientanom[2].mode = "anom3"
    tmpclientanom[2].pstart= time.time() + anom3time+ 1
    tmpclientanom[2].vnicm= virtNIC
    tmpclientanom[2].runtime= 300
    tmpclientanom[2].dest = '192.168.6.8'
    tmpclientanom[2].alloc()
    tmpclientanom[2].start()
    
    tmpclientanom.append(clientthreads("eth1:190", "192.168.6.190", 502))
    tmpclientanom[3].mode = "anom5"
    tmpclientanom[3].pstart= time.time() + anom5time+ 1
    tmpclientanom[3].vnicm= virtNIC
    tmpclientanom[3].runtime= 300
    tmpclientanom[3].dest = '192.168.6.8'
    tmpclientanom[3].alloc()
    tmpclientanom[3].start()
    
    tmpclientanom.append(clientthreads("eth1:190", "192.168.6.190", 502))
    tmpclientanom[4].mode = "anom8"
    tmpclientanom[4].pstart= time.time() + anom8time+ 1
    tmpclientanom[4].vnicm= virtNIC
    tmpclientanom[4].runtime= 300
    tmpclientanom[4].dest = '192.168.6.8'
    tmpclientanom[4].alloc()
    tmpclientanom[4].start()
    
    tmpclientanom.append(clientthreads("eth1:190", "192.168.6.190", 502))
    tmpclientanom[5].mode = "anom9"
    tmpclientanom[5].pstart= time.time() + anom9time+ 1
    tmpclientanom[5].vnicm= virtNIC
    tmpclientanom[5].runtime= 300
    tmpclientanom[5].dest = '192.168.6.8'
    tmpclientanom[5].alloc()
    tmpclientanom[5].start()



    #print "------cut here-------"
    # alloc here
    signal.signal(signal.SIGINT, signalEvent)
    signal.pause()

if __name__ == '__main__':
    main()
