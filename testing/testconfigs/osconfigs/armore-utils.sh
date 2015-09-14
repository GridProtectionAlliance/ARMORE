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
#Attempt to load the hardware modules
#This is called to load the hardware modules for netmap
#We'll blindly reload them all for now
###############################################################################
checkhwmodules ()
{
    HWKMODSRC="/lib/modules/3.12.0-armore/kernel/net/armorenetmap"
    #INTELKMODREG="$(lsmod | grep -ci e1000)"
    #INTELKMODPRO="$(lsmod | grep -ci e1000e)"
    #INTELKMODREG1="$(lsmod | grep -ci igb)"
    #INTELKMODPRO1="$(lsmod | grep -ci ixgbe)"

    #if [ "$INTELKMODREG" \> "0" ];
    #then
        rmmod e1000
        insmod $HWKMODSRC/e1000.ko
    #fi

    #if [ "$INTELKMODPRO" \> "0" ];
    #then
        rmmod e1000e
        insmod $HWKMODSRC/e1000e.ko
    #fi

    #if [ "$INTELKMODREG1" \> "0" ];
    #then
        rmmod igb
        insmod $HWKMODSRC/igb.ko
    #fi

    #if [ "$INTELKMODPRO1" \> "0" ];
    #then
        rmmod igxbe
        insmod $HWKMODSRC/ixgbe.ko
    #fi


}
###############################################################################
#Check if netmap is actually loaded
###############################################################################
checkmodules()
{
    NETMAPLOADED="$(lsmod | grep -ci netmap)"

    if [ "$NETMAPLOADED" -eq "1" ];
    then
        return 1
    else
        NETMAPSRC="$(insmod /lib/modules/3.12.0-armore/kernel/net/armorenetmap/netmap.ko | grep -ci Error)"
        #echo "Reloading netmap.ko"
        if [ $NETMAPSRC -ne "1" ];
        then
            checkhwmodules
            service networking restart 
            return 1 
        else
            return 0
        fi
    fi

}

