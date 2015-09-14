#!/bin/bash
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
#Check the users environment before we try and install things
#First we'll check if we're root
#Then we'll check if there is internet
###############################################################################
environment_check ()
{

    #Check if user is root
    if [ "$EUID" -ne 0 ];
    then 
        echo "Please run this script as root"
        exit
    fi
}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
runprompt ()
{

    whiptail --title "ARMOREnode Proxy Configuration tool" --yesno\
        "This utility will configure the ARMOREnode Proxy."\
        --yes-button "Continue" --no-button "Cancel"  10 70

    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi


}
###############################################################################
#Select Operation Type to write out to /etc/armore/proxycfg.txt
#Proxy (Default setting for Alpha)
#Passive
#Transparent
###############################################################################
optype ()
{
    optype=$(whiptail --title "Operation Type" --radiolist "Choose a mode of operation" 20 90 6 \
         "Proxy" "Default Operation of ArmoreNode to encapsulate and Monitor Data." on \
         "Passive" "Monitors Data only." off \
         "Transparent" "Device is latched with no operation." off  3>&1 1>&2 2>&3)
    
    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi

}
###############################################################################
#Select Role to write out to /etc/armore/proxycfg.txt
#Server (Default setting)
#Client
###############################################################################
roletype ()
{
    roletype=$(whiptail --title "Role" --radiolist "Role of this node" 20 80 6 \
         "Server" "ArmoreNode will act as a server for other nodes." on \
         "Client" "ArmoreNode will connect to a Server to Relay traffic." off  3>&1 1>&2 2>&3)


    if [ "$roletype" == "Server" ];
    then
        echo "Server"
    else
        echo "Client"
    fi
    
    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi

}
###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
selectiface ()
{

    iface1=$(whiptail --inputbox "Please enter interface to bind to" 8 78 eth0 --title "Select an Interface" 3>&1 1>&2 2>&3)
    
    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi

}
###############################################################################
#Select Role to write out to /etc/armore/proxycfg.txt
#Server (Default setting)
#Client
###############################################################################
bindaddr()
{
    if [ "$roletype" == "Server" ];
    then
        bindad=$(whiptail --inputbox "Please enter IP address to bind to." 8 78 \
            --title "Address for Server to Bind to" 3>&1 1>&2 2>&3)
    else
        bindad=$(whiptail --inputbox "Please enter IP address to bind to. " 8 78 \
            127.0.0.1 --title "Address for Client to Bind to" 3>&1 1>&2 2>&3)
    fi
    
    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi


}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
selectaddr ()
{
    
    if [ "$roletype" == "Client" ];
    then
        connad=$(whiptail --inputbox "Enter Server IP address client should connect to." 8 78 \
            --title "Address for Server to Bind to" 3>&1 1>&2 2>&3)
    fi
    
    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi
    


}
###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
connport ()
{

    port=$(whiptail --inputbox "Please Enter Port address" 8 78 \
        5560 --title "Address for Server to Bind to" 3>&1 1>&2 2>&3)

    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi
}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
pcapt ()
{
    captype=$(whiptail --title "Packet Capture Type" --radiolist "Select Capture Mode" 20 75 6 \
         "NetMap" "Use NetMap for packet capture" on \
         "Pcap" "Use Pcap for packet capture" off  3>&1 1>&2 2>&3)



    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        exit
    fi
}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
writeconfig ()
{
    ifile=/etc/armore/proxycfg
    
printf "###############################################################################\n" > $ifile
printf "#This file Automatically Generated by /etc/armore/armoreproxycfg.sh \n" >> $ifile
printf "###############################################################################\n\n" >> $ifile

    printf "Operation = $optype\n" >> $ifile
    printf "Roletype = $roletype\n" >> $ifile
    printf "Interface = $iface1\n" >> $ifile
    printf "Bind = $bindad\n" >> $ifile
    printf "Connect = $connad\n" >> $ifile
    printf "Port = $port\n" >> $ifile
    printf "Capture = $captype\n" >> $ifile
    printf "Log = $logfile\n" >> $ifile

}


###############################################################################
#Installation Script for ARMORE NODE on Debian Wheezy
###############################################################################
#environment_check
#Let's put some default values
    logfile='/dev/null'
    optype='proxy'
    roletype='Server'
    iface1='eth0'
    bindad=''
    connad=''
    port='5560'
    captype='NetMap'
runprompt
optype
roletype
selectiface
bindaddr
selectaddr
connport
pcapt
writeconfig



