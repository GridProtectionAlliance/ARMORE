#!/bin/bash
#Needs iperf on source and destination (apt-get install iperf)
#On server run iperf -s -u -B 192.168.10.90
#normal ping output
REMOTEADDR="192.168.1.91"
echo "-------------------------------------------------------------------------"
echo "Begin Normal Ping test"
echo "-------------------------------------------------------------------------"
ping -q $REMOTEADDR -c 5000
echo "TEST 1"
ping -q $REMOTEADDR -c 5000
echo "TEST 2"
ping -q $REMOTEADDR -c 5000
echo "TEST 3"
ping -q $REMOTEADDR -c 5000
echo "TEST 4"
ping -q $REMOTEADDR -c 5000
echo "TEST 5"
ping -q $REMOTEADDR -c 5000
echo "TEST 6"
ping -q $REMOTEADDR -c 5000
echo "TEST 7"
ping -q $REMOTEADDR -c 5000
echo "TEST 8"
ping -q $REMOTEADDR -c 5000
echo "TEST 9"
ping -q $REMOTEADDR -c 5000
echo "TEST 10"

#test out ping flooding to see packet loss
echo "-------------------------------------------------------------------------"
echo "Begin Ping Flooding"
echo "-------------------------------------------------------------------------"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 1"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 2"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 3"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 4"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 5"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 6"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 7"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 8"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 9"
ping -f -q $REMOTEADDR -c 100000
echo "TEST 10"

echo "-------------------------------------------------------------------------"
echo "Begin IPERF"
echo "-------------------------------------------------------------------------"

#test out ping flooding to see packet loss
#make sure to start iperf server on destination: iperf -s -u
iperf -u -c $REMOTEADDR -i 1 -r -b 1000m 
echo "TEST 1"
iperf -u -c $REMOTEADDR -i 1 -r -b 1000m 
echo "TEST 2"
iperf -u -c $REMOTEADDR -i 1 -r -b 1000m 
echo "TEST 3" 
iperf -u -c $REMOTEADDR -i 1 -r -b 1000m 
echo "TEST 4"
iperf -u -c $REMOTEADDR -i 1 -r -b 1000m 
echo "TEST 5"
