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


    #Check we actually have internet
    #if  [ "`ping -c 2 code.iti.illinois.edu`" ];
    #then
    #    echo "Network is up"
    #else
    #    echo "Please Check Network"
    #    exit 
    #fi


}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
runprompt ()
{

    whiptail --title "ARMOREnode Network Configuration tool" --yesno\
        "This utility will help you create a network configuration for an ARMOREnode. We will backup your existing configuration."\
        --yes-button "Continue" --no-button "Cancel"  10 70

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
    whiptail --title "ARMOREnode Network Configuration tool" --msgbox\
        "We need to create a network bridge using 2 interfaces to route traffic" 8 70

    iface1=$(whiptail --inputbox "Please enter first interface" 8 78 eth0 --title "Select an Interface" 3>&1 1>&2 2>&3)
    iface2=$(whiptail --inputbox "$iface1 was selected as the first interface.\n\
Please enter second interface" 8 78 eth1 --title "Select an Interface" 3>&1 1>&2 2>&3)
    
    #make sure the interfaces are not the same else we'll restart the process again
    if [ "$iface1" == "$iface2" ];
    then 
    
        whiptail --title "ARMOREnode Network Configuration tool" --msgbox\
        "The interfaces can not be the same" 8 70
        
        selectiface
    fi

}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
selectaddr ()
{

    bridgeip=$(whiptail --inputbox "Please enter an ipv4 address for this armorenode" 8 78 \
        192.168.1.5 --title "ARMOREnode Network Configuration tool" 3>&1 1>&2 2>&3)
    
    broadcast=$(whiptail --inputbox "Please enter an broadcast address for this armorenode" 8 78 \
        192.168.1.255 --title "ARMOREnode Network Configuration tool" 3>&1 1>&2 2>&3)
    
    netmask=$(whiptail --inputbox "Please enter a netmask address for this armorenode" 8 78 \
        255.255.255.0 --title "ARMOREnode Network Configuration tool" 3>&1 1>&2 2>&3)
    
    gateway=$(whiptail --inputbox "Please enter the gateway address for this armorenode" 8 78 \
        192.168.1.1 --title "ARMOREnode Network Configuration tool" 3>&1 1>&2 2>&3)
    
    route=$(whiptail --inputbox "Please enter routing address with cidr for other networks" 8 78 \
        192.168.2.0/24 --title "ARMOREnode Network Configuration tool" 3>&1 1>&2 2>&3)


}
###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
addrconfirm ()
{

    whiptail --title "ARMOREnode Network Configuration tool" --yesno\
        "BridgeIP Address: $bridgeip\nBroadcast Address: $broadcast\nNetmask: $netmask\nGateway: $gateway\nRoute:$route\n"\
        --yes-button "Continue" --no-button "Cancel"  12 70

    proceed=$?

    if [ "$proceed" -ne 0 ];
    then
        runprompt
    fi
}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
backupconfig ()
{
    if [ -f /etc/network/interfaces ]; then
        cp /etc/network/interfaces /etc/network/interfaces.backup
        whiptail --title "ARMOREnode Network Configuration tool" --msgbox\
            "Configuration file has been written to /etc/networking/interfaces and the orignal backed up" 8 70
    else
        whiptail --title "ARMOREnode Network Configuration tool" --msgbox\
            "An original network interface file was not found so it was not backed up" 8 70
    fi


}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
writeconfig ()
{
    ifile=/etc/network/interfaces

    printf '# This was automatically generated by ARMOREnode Network Configuration tool\n' > $ifile
    printf 'auto lo br0\n' >> $ifile
    printf 'iface lo inet loopback\n\n' >> $ifile
    
    printf "iface $iface1 inet manual\n" >> $ifile
    printf "iface $iface2 inet manual\n\n" >> $ifile
    
    
    printf "iface br0 inet static\n" >> $ifile
    printf "    bridge_ports $iface1 $iface2\n" >> $ifile
    printf "    address $bridgeip\n" >> $ifile
    printf "    broadcast $broadcast\n" >> $ifile
    printf "    netmask $netmask\n" >> $ifile
    printf "    gateway $gateway\n" >> $ifile
    printf "    up ip route add $route via $gateway\n" >> $ifile





}


###############################################################################
#Installation Script for ARMORE NODE on Debian Wheezy
###############################################################################
environment_check
#Let's put some default values
    bridgeip='192.168.1.5'
    broadcast='192.168.1.255'
    netmask='255.255.255.0'
    gateway='192.168.1.1'
    route='192.168.2.0/24'
    iface1='eth0'
    iface2='eth1'
runprompt
selectiface
selectaddr
addrconfirm
backupconfig
writeconfig
