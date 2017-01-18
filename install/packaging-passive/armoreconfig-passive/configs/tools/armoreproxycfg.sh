#!/bin/bash

set -eu

###############################################################################
#Check if we are root.
#This script is run at postinst, but can also be run manually after
###############################################################################
environment_check ()
{
    #Check if user is root
    if [ "$EUID" -ne 0 ];
    then 
        echo "Please run this script as root"
        exit 0 
    fi
}

###############################################################################
#Prompt User
###############################################################################
runprompt ()
{
    existconfig=/etc/armore/armoreconfig
    if [ -e "$existconfig" ];
    then
        overwriteconfig=$(whiptail --title "ARMORE Proxy Configuration Tool" --radiolist "Existing ARMORE Configuration Found" 20 80 6 \
            "Overwrite" "Overwrite existing ARMORE configuration" on \
            "Keep" "Keep existing ARMORE configuration" off  3>&1 1>&2 2>&3)
        
        if [ "$overwriteconfig" == "Keep" ];
        then
            exit 0
        fi

    else
    
        whiptail --title "ARMOREnode Proxy Configuration Tool" --yesno\
        "This utility will configure an ARMORE node"\
        --yes-button "Continue" --no-button "Cancel"  10 70

    if [ "$?" -ne 0 ];
    then
        exit 0
    fi
fi
}
###############################################################################
#Select Operation Type 
#1. Proxy:(Default) ARMOREProxy will proxy data between nodes. Bro will monitor
#   Traffic.
#2. Passive: Monitor data
#3. Transparent: Network Bridge monitoring
###############################################################################
optype ()
{
    optype=$(whiptail --title "Operation Type" --radiolist "Choose a mode of operation" 20 65 8 \
        "Proxy" "Encapsulate and monitor network data." on \
        "Passive" "Monitor network data" off \
        "Transparent" "Forward network data" off  3>&1 1>&2 2>&3)

    if [ "$?" -ne 0 ];
    then
        exit 0 
    fi
}
###############################################################################
#Select Nodes Role in the Network
#Server ARMORE nodes will connect here (Default)
#Client
###############################################################################
roletype ()
{
    roletype=$(whiptail --title "Role" --radiolist "Role of this node" 20 80 6 \
         "Server" "ArmoreNode will act as a server for other nodes." on \
         "Client" "ArmoreNode will connect to a Server to Relay traffic." off  3>&1 1>&2 2>&3)
    
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
    
}
###############################################################################
#Prompt user to enable or disable IPFW Firewall
###############################################################################
firewallen()
{
    firewall=$(whiptail --title "IPFW Setting" --radiolist "Firewall Setting" 20 80 6 \
         "Disabled" "IPFW will be entirely disabled" on \
         "Enabled" "IPFW will be started with ARMORE." off  3>&1 1>&2 2>&3)
    
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
    
}
###############################################################################
#Select interface connected to internet or outer facing network
#Data comming from here will typically be encrypted/encapsulated by ARMORE
###############################################################################
mgmtiface ()
{
    mgmtiface1=$(whiptail --inputbox "Enter an interface to use for management" 8 78 \
        eth0 --title "Management Network Interface" 3>&1 1>&2 2>&3)
    
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi


        mgmtaddr=$(whiptail --inputbox "Enter IP address for management interface" 8 78 \
            --title "Management IP" 3>&1 1>&2 2>&3)
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi


        mgmtmask=$(whiptail --inputbox "Enter netmask for management interface" 8 78 \
        255.255.255.0 --title "Management Netmask" 3>&1 1>&2 2>&3)
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
}
###############################################################################
#Select interface connected to internet or outer facing network
#Data comming from here will typically be encrypted/encapsulated by ARMORE
###############################################################################
proxyiface ()
{
    #Specify internal interface to proxy data from 
    proxyiface1=$(whiptail --inputbox "Enter internal network interface to proxy data from." 8 78 \
        eth2 --title "Network interface" 3>&1 1>&2 2>&3)
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
    
    
    #Specify internal interface address
    pifaceaddr1=$(whiptail --inputbox "Enter internal interface IP address" 8 78 \
        192.168.1.2 --title "Internal Network Interface IP" 3>&1 1>&2 2>&3)
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
    
    #Specify internal interface netmask
    pifacemask1=$(whiptail --inputbox "Enter internal network interface netmask" 8 78 \
        255.255.255.0 --title "Internal Network Interface Netmask" 3>&1 1>&2 2>&3)
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
}
###############################################################################
#Select interface connected to internal interface to monitor
###############################################################################
spaniface ()
{ 
    #Specify interface we will passively monitor
    spaniface1=$(whiptail --inputbox "Enter network interface to monitor." 8 78 \
        eth2 --title "Network interface" 3>&1 1>&2 2>&3)
    
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
    
    
    #Specify interface address we will monitor 
    spanaddr1=$(whiptail --inputbox "Enter internal interface IP address" 8 78 \
        192.168.1.1 --title "Internal Network Interface IP" 3>&1 1>&2 2>&3)
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
    
    
    #Specify interface netmask we will monitor
    spanmask1=$(whiptail --inputbox "Enter internal network interface netmask" 8 78 \
        255.255.255.0 --title "Internal Network Interface Netmask" 3>&1 1>&2 2>&3)
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
}
###############################################################################
#Set Encryption
#Enable encryption of encapsulated data between nodes. 
#Encryption Keys will be saved in /etc/armore/.curve
###############################################################################
encrypt()
{
    encryptdata=$(whiptail --title "Encryption Mode" --radiolist "Set Encryption" 20 80 6 \
         "Enabled" "Encapsulated data will be encrypted between nodes" on \
         "Disabled" "Data is encapsulated but not encrypted between nodes" off  3>&1 1>&2 2>&3)
    
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
    
}
###############################################################################
#Set interface address which server or client will connect to
#This address sits on the external facing interface
#There's a binding address as well in the tool, but not implemented
###############################################################################
connaddr()
{
    #Specify the interface we will use to connect to the external network
    extiface=$(whiptail --inputbox "Enter external network facing interface" 8 78 \
        eth1 --title "External Network Interface" 3>&1 1>&2 2>&3)

    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi



    #Specify the IPv4 address will use for the external network
    if [ "$roletype" == "Server" ];
    then
        extip=$(whiptail --inputbox "Enter external interface address" 8 78 \
            --title "External Interface IP address" 3>&1 1>&2 2>&3)

    else
        extip=$(whiptail --inputbox "Enter server address" 8 78 \
            --title "External Interface IP address" 3>&1 1>&2 2>&3)
    fi
    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi


    #Specify the netmask address for the external network
    extmask=$(whiptail --inputbox "Enter netmask for this interface" 8 78 \
        255.255.255.0 --title "External Interface Netmask" 3>&1 1>&2 2>&3)

    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
}
###############################################################################
#Set port where client or server will communicate to outer interface address
###############################################################################
connport ()
{
    port=$(whiptail --inputbox "Please Enter Port Number" 8 78 \
        5560 --title "Port Number" 3>&1 1>&2 2>&3)

    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
}

###############################################################################
#Set capture type.
#NetMap - Requires netmap kmods and vale configuration
#Pcap - Requires pfring and additional kmods to speed operation
#Dpdk - Not Yet Implemented
###############################################################################
pcapt ()
{
    captype=$(whiptail --title "Packet Capture Type" --radiolist "Select Capture Mode" 20 75 6 \
         "NetMap" "Use NetMap for packet capture" on \
         "Pcap" "Use Pcap for packet capture" off  3>&1 1>&2 2>&3)

    if [ "$?" -ne 0 ];
    then
        echo "User cancelled operation"
        exit 0
    fi
}

###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
writeproxconfig ()
{
    configdir=/etc/armore
    ifile=/etc/armore/armoreconfig
    armoreproxy=/usr/bin/ARMOREProxy
    existkeys=/etc/armore/.curve


    ###Check to see if configuration directory exists 
    if [ ! -d "$configdir" ];        
    then
        whiptail --title "Warning" --msgbox\
            "Could not find $configdir directory to write configuration" 20 80
        exit 0
    fi

    ###Check to see if ARMOREProxy is installed to write keys.
    ###Then check for existing keys
    if [ ! -f "$armoreproxy" ];
    then
        whiptail --title "Warning" --msgbox\
            "ARMOREProxy not found. Encryption keys will not be created" 20 80
        encryptdata='Disabled'

    elif [ -d "$existkeys" ];
    then
        writekey=$(whiptail --title "Warning" --radiolist "Existing Key Found" 20 80 6 \
            "Overwrite" "Overwrite existing key in /etc/armore/" on \
            "Keep" "Keep existing key in /etc/armore" off  3>&1 1>&2 2>&3)

        if [ "$?" -ne 0 ];
        then
            exit 0
        fi


    fi


    printf "###############################################################################\n" > $ifile
    printf "#This file automatically generated by /etc/armore/armoreproxycfg.sh \n" >> $ifile
    printf "###############################################################################\n\n" >> $ifile

    printf "Operation = $optype\n" >> $ifile
    printf "Roletype = $roletype\n" >> $ifile
    printf "Interface = $proxyiface1\n" >> $ifile
    printf "Encryption = $encryptdata\n" >> $ifile
    printf "Bind = $extip\n" >> $ifile
    printf "Port = $port\n" >> $ifile
    printf "Capture = $captype\n" >> $ifile
    printf "Firewall = $firewall\n" >> $ifile
    printf "ManagementIp = $mgmtaddr\n" >> $ifile
    printf "ManagementInt = $mgmtiface1\n" >> $ifile
    printf "ManagementMask = $mgmtmask\n" >> $ifile


    #Begin generating keys in /etc/armore using ARMOREProxy.

    #On first install for a fresh system, the netmap kmod is not loaded.
    #We temporarily run ARMOREProxy in pcap mode to generate the keys until
    #the next start up when depmod and init script take over using keys
    #Keys need to be copied over between server and client by sysadmin

    #We also make sure the armoreproxy is actuall installed


    if [[ "$encryptdata" == "Enabled" && ! "writekey" == "Keep" ]];
    then
        cd $configdir
        if [ "$roletype" == "Server" ];
        then
            #preserve status or callback fails from return
            timeout --preserve-status 3 /usr/bin/ARMOREProxy -s -e -c pcap lo *:5560&
        else
            #preserve status or callback fails from return
            timeout --preserve-status 3 /usr/bin/ARMOREProxy -e -c pcap lo *:5560&
        fi
    fi


}
###############################################################################
# Key Notice
###############################################################################
keynotice ()
{

    if [ "$encryptdata" == "Enabled" ];
    then

    whiptail --title "Encryption Notice" --msgbox\
        "With Encryption enabled, you will need to copy generated keys between server and client nodes. \n
    The server's key in: \n<server machine>/etc/armore/.curve/local/server.pub should be copied to <client machine>/etc/armore/.curve/remote. \n
    The client's key in: \n<client machine>/etc/armore/.curve/local/<clienthostname.pub> should be copied to <server machine>/etc/armore/.curve/remote. " 30 80

    fi
}
###############################################################################
# Bridge Configuration
###############################################################################
bridgeiface ()
{
    whiptail --title "ARMOREnode Network Configuration tool" --msgbox\
        "We need to create a network bridge using 2 interfaces to route traffic" 8 70

    bridgeiface1=$(whiptail --inputbox "Please enter first interface" 8 78 eth0 --title "Select an Interface" 3>&1 1>&2 2>&3)
    bridgeiface2=$(whiptail --inputbox "$bridgeiface1 was selected as the first interface.\n\
Please enter second interface" 8 78 eth1 --title "Select an Interface" 3>&1 1>&2 2>&3)
    
    #make sure the interfaces are not the same else we'll restart the process again
    if [ "$bridgeiface1" == "$bridgeiface2" ];
    then 
    
        whiptail --title "ARMOREnode Network Configuration tool" --msgbox\
        "The interfaces can not be the same" 8 70

        bridgeiface
    fi

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
writebridgeconfig ()
{
    configdir=/etc/armore
    ifile=/etc/armore/armoreconfig



    ###Check to see if configuration directory exists 
    if [ ! -d "$configdir" ];        
    then
        whiptail --title "Warning" --msgbox\
            "Could not find $configdir directory to write configuration" 20 80
        exit 0
    fi

    printf "###############################################################################\n" > $ifile
    printf "#This file automatically generated by /etc/armore/armoreproxycfg.sh \n" >> $ifile
    printf "###############################################################################\n\n" >> $ifile

    printf "Operation = $optype\n" >> $ifile
    printf "Interface = br0\n" >> $ifile
    printf "Bind = $bridgeip\n" >> $ifile
    printf "ManagementIp = $mgmtaddr\n" >> $ifile
    printf "ManagementInt = $mgmtiface1\n" >> $ifile
    printf "ManagementMask = $mgmtmask\n" >> $ifile
}
###############################################################################
#Prompt User to see if they actually want to have this program configure
###############################################################################
writespanconfig ()
{
    configdir=/etc/armore
    ifile=/etc/armore/armoreconfig


    ###Check to see if configuration directory exists 
    if [ ! -d "$configdir" ];        
    then
        whiptail --title "Warning" --msgbox\
            "Could not find $configdir directory to write configuration" 20 80
        exit 0
    fi

    printf "###############################################################################\n" > $ifile
    printf "#This file automatically generated by /etc/armore/armoreproxycfg.sh \n" >> $ifile
    printf "###############################################################################\n\n" >> $ifile

    printf "Operation = $optype\n" >> $ifile
    printf "Interface = $spaniface1\n" >> $ifile
    printf "ManagementIp = $mgmtaddr\n" >> $ifile
    printf "ManagementInt = $mgmtiface1\n" >> $ifile
    printf "ManagementMask = $mgmtmask\n" >> $ifile
}
###############################################################################
#Write out bro configuration
###############################################################################
writeinterfaces ()
{
    ifacefile=/etc/network/interfaces

    printf '#Autogenerated Management interface /etc/armore/armoreproxyconfig.sh\n' > $ifacefile
    printf "auto lo $mgmtiface1\n" >> $ifacefile
    printf 'iface lo inet loopback\n' >> $ifacefile

    printf '#Management interface\n' >> $ifacefile
    printf "allow-hotplug $mgmtiface1\n" >> $ifacefile
    printf "iface $mgmtiface1 inet static\n" >> $ifacefile
    printf "address $mgmtaddr \n" >> $ifacefile
    printf "netmask $mgmtmask\n" >> $ifacefile
    printf 'up sleep 5;\n\n' >> $ifacefile

    if [ "$optype" == "Proxy" ];
    then
        printf '#Proxy Mode Configuration\n' >> $ifacefile
        printf "auto $extiface $proxyiface1\n\n" >> $ifacefile


        printf '#External Interface\n' >> $ifacefile
        printf "allow-hotplug $extiface\n" >> $ifacefile
        printf "iface $extiface inet static\n" >> $ifacefile
        printf "address $extip\n" >> $ifacefile
        printf "netmask $extmask\n" >> $ifacefile
        printf 'up sleep 5;\n\n' >> $ifacefile

        printf '#Internal Interface\n' >> $ifacefile
        printf "allow-hotplug $proxyiface1\n" >> $ifacefile
        printf "iface $proxyiface1 inet static\n" >> $ifacefile
        printf "address $pifaceaddr1\n" >> $ifacefile
        printf "netmask $pifacemask1\n" >> $ifacefile
        printf "up sleep 5; $proxyiface1 promisc\n\n" >> $ifacefile



    elif [ "$optype" == "Passive" ];
    then
        printf '#Passive Mode Configuration\n' >> $ifacefile
        printf "auto $spaniface1 \n\n" >> $ifacefile


        printf '#Span interface\n' >> $ifacefile
        printf "allow-hotplug $spaniface1\n" >> $ifacefile
        printf "iface $spaniface1 inet static\n" >> $ifacefile
        printf "address $spanaddr1 \n" >> $ifacefile
        printf "netmask $spanmask1 \n" >> $ifacefile
        printf "up sleep 5; $spaniface1 promisc\n\n" >> $ifacefile


    elif [ "$optype" == "Transparent" ];
    then
        printf '#Transparent Mode Bridge\n' >> $ifacefile
        printf 'auto br0\n\n' >> $ifacefile

        printf "iface $bridgeiface1 inet manual\n" >> $ifacefile
        printf "iface $bridgeiface2 inet manual\n\n" >> $ifacefile


        printf "iface br0 inet static\n" >> $ifacefile
        printf "    bridge_ports $bridgeiface1 $bridgeiface2\n" >> $ifacefile
        printf "    address $bridgeip\n" >> $ifacefile
        printf "    broadcast $broadcast\n" >> $ifacefile
        printf "    netmask $netmask\n" >> $ifacefile
        printf "    gateway $gateway\n" >> $ifacefile
        printf "    up ip route add $route via $gateway\n" >> $ifacefile
    else
        echo "Undefined mode"
    fi


}
###############################################################################
#Write out bro configuration
###############################################################################
broifaceset ()
{
    if [ "$optype" == "Proxy" ];
    then
        sed -i s/interface\=.*/interface\=$proxyiface1/ /etc/bro/node.cfg
    elif [ "$optype" == "Passive" ];
    then
        sed -i s/interface\=.*/interface\=$spaniface1/ /etc/bro/node.cfg
    elif [ "$optype" == "Transparent" ];
    then
        sed -i s/interface\=.*/interface\=br0/ /etc/bro/node.cfg
    else
        echo "Undefined Operation Type"
    fi

    broctl install
}

###############################################################################
#Operation Define
##Begin setting environment variables based on optype selection
###############################################################################
opdefine ()
{
    if [ "$optype" == "Proxy" ];
    then
        roletype           #define client or server mode 
        firewallen         #Enable or Disable Firewall
        proxyiface         #define internal network interface to proxy data from
        encrypt            #define if user wants to enable encryption on proxy
        connaddr           #define address for clients to connect to
        connport           #define proxy port
        pcapt              #define traffic capturing method
        writeproxconfig    #write out armore configuration files
        backupconfig       #backup interfaces file
        writeinterfaces    #write out the interface file
        broifaceset        #set the interfaces for bro
        keynotice          #inform user of key exchange

    elif [ "$optype" == "Passive" ];
    then
        spaniface          #define interface to monitor
        writespanconfig    #write out armore configuration files
        backupconfig       #backup interfaces file
        writeinterfaces    #write out the interfaces file 
        broifaceset        #set the interfaces for bro
    else [ "$optype" == "Transparent" ];
        bridgeiface        #define bridge configurations
        addrconfirm        #confirm bridge settings with user
        writebridgeconfig  #Write out armore configuration files
        backupconfig       #backup interfaces file
        writeinterfaces    #write out the interfaces file 
        broifaceset        #set the interfaces for bro
    fi

    ##Here we're going to define the bypass mode for failure
}


###############################################################################
#Installation Script for ARMORE NODE on Debian Wheezy
###############################################################################

environment_check
#Default Write Values
    mgmtiface1='eth0'
    mgmtaddr='192.168.1.5'
    mgmtmask='255.255.255.0'
#Proxy Mode Default Variables 
    optype='Proxy'
    roletype='Server'
   
#Proxy Mode internal network values
    proxyiface1='eth2'
    pifaceaddr1='192.168.1.3'
    pifacemask1='255.255.255.0'

#Proxy Mode external network values
    extiface='eth1'
    extip='192.168.1.2'
    extmask='255.255.255.0'

    encryptdata='Enabled'
    port='5560'
    captype='NetMap'
#Proxy Mode Default Variables 
    spaniface1='eth1'
    spanaddr1='192.168.1.1'
    spanmask1='255.255.255.0'
#FireWall Settings
    firewall='Disabled'
#Bridge Write Values
    bridgeip='192.168.1.5'
    broadcast='192.168.1.255'
    netmask='255.255.255.0'
    gateway='192.168.1.1'
    route='192.168.2.0/24'
    bridgeiface1='eth0'
    bridgeiface2='eth1'
runprompt
mgmtiface
optype
opdefine
