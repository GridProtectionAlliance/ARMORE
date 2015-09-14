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
from pymodbus.client.sync import ModbusTcpClient
import time



#-----------------------------------------------------------------------------#
# Connect to the Server
#-----------------------------------------------------------------------------#

serv=raw_input('Enter Server Address: ')
client = ModbusTcpClient(serv, port=502)
mpackets = input('Enter Modbus Packets: ')



#-----------------------------------------------------------------------------#
# Send out Coil Writes to server and get time between. 
#-----------------------------------------------------------------------------#

testrun = mpackets
totaltime = 0;
avgtime = 0;

tstart = time.clock()

while testrun >0 :
    client.write_coil(1, True)
    pstart = time.clock()
    result = client.read_coils(1,1)
    pend = time.clock()
    roundtrip=(pend-pstart)
    print "Coil Write to", serv, " Time:", roundtrip*1000, "ms"
    #print result.bits[0]
    client.close()
    avgtime=roundtrip+avgtime
    testrun = testrun - 1


tend= time.clock()
totaltime  =(tend-tstart)*1000
averagetime=(avgtime/mpackets)*1000

#-----------------------------------------------------------------------------#
# Print out statistics. 
#-----------------------------------------------------------------------------#

print("--- ModbusServer Statistics ---")
print "Packets Sent:", mpackets
print "Total Time:", totaltime, "ms"
print "Average Time:", averagetime, "ms"
