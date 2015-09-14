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
#Installation Script for ARMORE Node
###############################################################################



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
    if  [ "`ping -c 2 code.iti.illinois.edu`" ];
    then
        echo "Network is up"
    else
        echo "Please Check Network"
        exit 
    fi


}

###############################################################################
#Check the Kernel Version
###############################################################################
kernel_check ()
{

    KERNEL_VERSION=`uname -r|cut -d\- -f1`
    MINIMAL_KERNEL="3.12.0"

    printf "It looks like you have Kernel $KERNEL_VERSION installed or currently running. \nThis installation requires kernel
    =>$MINIMAL_KERNEL \n\n\n"

    read -p "Do you want to install our kernel? [y/n]" yn
    case $yn in
        [Yy]* ) echo "Installing Kernel" && kernel_install;;
    [Nn]* ) echo "Dont Install" && exit;;
esac

}

###############################################################################
#Install new Kernel

#If KERNEL < 3.12 wget source from cosmos so we can build a few packages later
#http://cosmos.cites.illinois.edu/pub/linux/kernel/v3.x/linux-3.12.tar.xz
#The Kernel Headers will be installed from the debian package included
###############################################################################
kernel_install ()
{

    #Get the Kernel sources for later installation
    #printf "Downloading Kernel Sources"
    #wget -c http://cosmos.cites.illinois.edu/pub/linux/kernel/v3.x/linux-3.12.tar.xz -P /usr/src/

    #untarxz the sources and delete the tarball
    # printf "Unpacking Kernel Sources"
    #tar xf /usr/src/linux-3.12.tar.xz -C /usr/src/ && rm /usr/src/linux-3.12.tar.xz

    #Install ITI Debian Kernel 3.12
    printf "Installing ARMORE Linux Kernel"
    dpkg -i ./kernel/linux-image-3.12.0-armore_1_i386.deb \
        ./kernel/linux-headers-3.12.0-armore_1_i386.deb \
        ./kernel/linux-source-3.12.0-armore_1_all.deb

}

###############################################################################
#Patching LIBPCAP to work with NetMap

#We need to patch the libpcap.so.0.8. This will be useful for TCPDUMP or other
#libraries that may need to communicate with NETMAP
###############################################################################
lib_patch()
{
    LIBDIR=/usr/lib/i386-linux-gnu
    LIBFILE=/usr/lib/i386-linux-gnu/libpcap.so.0.8
    PCAPN=libpcap.so.1.6.0-PRE-GIT

    if [ -h $LIBFILE ];
    then
        #Patch Libcap with a shared library for tcpdump to work
        echo "Existing libpcap.so.0.8 found"
        cp -rf ./libs/libpcap.so.1.6.0-PRE-GIT $LIBDIR
        rm -rf $LIBFILE
        ln -s "$LIBDIR/$PCAPN"  "$LIBFILE"
        echo "Patched $LIBFILE"


    else
        echo "libpcap.so.0.8 not found"
        cp -rf ./libs/libpcap.so.1.6.0-PRE-GIT $LIBDIR
        ln -s "$LIBDIR/$PCAPN"  "$LIBFILE"
        echo "Patched $LIBFILE"
    fi


}
###############################################################################
#Installing Dependencies required by AMORE
###############################################################################
dep_install()
{
    
    TCPDUMP=4.3.0-1+deb7u2
    LUA=5.1.5-4+deb7u1
    LIBLUA=5.1.5-4+deb7u1
    JEMALOC=3.0.0-3
    BISON=1\:2.5.dfsg\-2.1
    LIBPCAP=1.3.0-1
    NETMAP=https://code.google.com/p/netmap
    NETMAPP=https://code.google.com/p/netmap-libpcap

    apt-get -qq -y update
    apt-get -qq -y install tcpdump=$TCPDUMP
    apt-get -qq -y install liblua5.1\-0-dev=$LIBLUA
    apt-get -qq -y install lua5.1=$LUA
    apt-get -qq -y install libjemalloc\-dev=$JEMALOC
    apt-get -qq -y install bison=$BISON
    apt-get -qq -y install libpcap0.8\-dev=$LIBPCAP
    apt-get -qq -y install build-essential
    
    git clone $NETMAP
    git clone $NETMAPP
}

###############################################################################
#Installing Dependencies required by AMORE
###############################################################################
pbricks_install()
{

    BUILD=$PWD
    PBRICKS=https://github.com/bro/packet-bricks
    PBDIR=./packet-bricks
    git clone $PBRICKS
    cd $PBDIR && \
        ./configure --with-jemalloc-include=/usr/include/jemalloc \
        --with-lua-include=/usr/include/lua5.1 \
        && make
    cd $BUILD


}
###############################################################################
#Installing Dependencies required by AMORE
###############################################################################
bro_install()
{
    BUILD=$PWD
    BRORPO=git://git.bro.org/bro
    BRODIR=./bro

    printf "Downloading Bro Dependencies"
    apt-get -qq -y install cmake make gcc g++ flex libssl-dev python-dev swig zlib1g-dev
    
    
    printf "Fetching Bro"

    git clone --recursive $BRORPO
    cd $BRODIR && \
        ./configure \
        && make \
        && make install
    cd $BUILD


}

###############################################################################
#Installing Dependencies required by AMORE
###############################################################################
bro_pinstall()
{
    BUILD=$PWD
    BROPRPO=https://github.com/bro/bro-plugins
    BROPDIR=./bro-plugins/netmap
    BRODIR=$BUILD/bro
    NETMAPDIR=$BUILD/netmap

    printf "Fetching NetMap Bro-Plugin Repository"

    git clone $BROPRPO
    cd $BROPDIR  && \
        ./configure --bro-dist=$BRODIR --with-netmap=$NETMAPDIR  \
        && make \
        && make install
    cd $BUILD


}

###############################################################################
#Installing Dependencies required by AMORE
###############################################################################
patchconfigs()
{
    BUILD=$PWD
    PBRICKSDIR=./packet-bricks
    BRODIR=/usr/local/bro


    printf "Modifying Default Configuration Files for Packet-Bricks \n"
    
    rm -rf $PBRICKSDIR/scripts
    cp -rf ../testconfigs/packet-bricks/scripts $BUILD/packet-bricks/

    printf "Modifying Default Configuration Files for Bro \n"

    rm -rf $BRODIR/etc
    cp -rf ../testconfigs/bro/etc $BRODIR/

    cd $BUILD


}
###############################################################################
#Installing Dependencies required by AMORE
###############################################################################
endinstall()
{
    printf "Installing of the ARMORE environment is complete \n"
    printf "You'll have to restart the machine to use the ARMORE 3.12 Kernel. \n"
    printf "Included is also a system start script which will initialize the environment \n"
    
    
}




###############################################################################
#Installation Script for ARMORE NODE on Debian Wheezy
###############################################################################

environment_check
kernel_check
dep_install
pbricks_install
bro_install
bro_pinstall
patchconfigs
lib_patch
endinstall
#PatchOS





#! /bin/bash
###############################################################################
#Initialization script for ARMORE NODE

#Please note, this is for a generic environment
#you will have to build and insmod the correct module for your card
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

env_init()
{

    PBRICKS=$PWD/packet-bricks/bin
    PBRICKCFG=$PWD/packet-bricks/scripts
    BUILD=$PWD

    #Lets get all the bricks binaries into our path
    export PATH=$PATH:$PBRICKS

    #start up vmwares kmods 
    vmware-user  

    #This will stop the OS from doing an autoconf or dhcp on the interfaces 
    service network-manager stop 

    #We will want to run our card in this mode to enable sniffing
    ifconfig eth0 promisc

    #Make sure to comment out the right module for your card as well. netmap.ko is needed bare minimum
    insmod $BUILD/kmods/netmap.ko

    printf "Packet-Bricks and Bro should now be able to be started \n"
}


environment_check
env_init





