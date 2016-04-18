###############################################################################
#Created 10/16/2015
#Patch info for 3.18.21
###############################################################################


##Applying Patch
bp_mod.c has been patched to work with kernel 3.18.21-armore
    patch <bpmodpatch-3.18.21.patch
to remove patch:
patch -R < bpmodpatch-3.18.21.patch


##Configure for autoload
To autoload insert kernel module in /lib/modules/<kernel>/
depmod -a

echo "bpctl_mod" >> /etc/modules


To Disable WatchDog:
Once kernel module has been inserted (bpctl_mod.ko)
Go to /proc/net/bypass/bypass_<interface> 
echo "0" > bypass_wd
echo "off" > bypass

To Set Watch Dog Timer:
echo "10000" > bypass_wd



Please refer to Linux_BP_Prog_Guide.pdf in silicom driver pack
