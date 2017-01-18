Installing AppArmor
------------------------

Most of the instructions are from here: https://wiki.debian.org/AppArmor/HowToUse  

To manually install the profile for ARMOREProxy in ARMORE you will need to do the following:  

1) Install ARMORE First  

2) ```sudo apt-get install apparmor apparmor-profiles apparmor-utils```  

3) Copy the usr.bin.ARMOREProxy profile to /etc/apparmor.d/   

4) ```sudo perl -pi -e 's,GRUB_CMDLINE_LINUX="(.*)"$,GRUB_CMDLINE_LINUX="$1 apparmor=1 security=apparmor",' /etc/default/grub```  

5) ```sudo update-grub```  

6) ```sudo reboot```  

7) Check the status by doing: ```sudo aa-status```  

8) By default the profile is in Audit mode, you can change the flag in the file, or do: ```sudo aa-enforce /etc/apparmor.d/usr.bin.ARMOREProxy```  

9) ```sudo dmesg``` should also show issues regarding apparmor  
