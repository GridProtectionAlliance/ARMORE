###############################################################################
#Attempt to load the hardware modules
#This is called to load the hardware modules for netmap
#We'll blindly reload them all for now
###############################################################################
checkhwmodules ()
{
    HWKMODSRC="/lib/modules/4.4.19-armore/kernel/net/armorenetmap"
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
        NETMAPSRC="$(insmod /lib/modules/4.4.19-armore/kernel/net/armorenetmap/netmap.ko | grep -ci Error)"
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

