 	 Silicom Linux Bypass-SD Control Utility
		  
1. Compiling, installing and loading the software.
              
   Compiling and installing the software in the system directory:
	# make install
	
2. Software loading:
	# bpctl_start 
	      
3. Using the software.

3.1 Utility.

Usage: bpctl_util <if_index|bus:slot.function> <command> [parameters]
       bpctl_util <info|help>
<if_index>   - interface name, for example, eth0, or all for all Bypass-SD/TAP-SD Control devices
<command>    - bypass control command (see Commands List).
[parameters] - set_bypass_wd command:
                   WDT timeout interval, msec (0 for disabling WDT).
               set_bypass/set_bypass_pwoff/set_bypass_pwup/set_dis_bypass commands:
                   on/off for enable/disable Bypass
               set_std_nic command:
                   on/off for enable/disable Standard NIC mode
               set_tx command:
                   on/off for enable/disable transmit
               set_tpl command:
                   on/off for enable/disable TPL
               set_hw_reset command:
                   on/off for enable/disable hw_reset
               set_tap/set_tap_pwup/set_dis_tap commands:
                   on/off for enable/disable TAP
               set_disc/set_disc_pwup/set_dis_disc commands:
                   on/off for enable/disable Disc
               set_wd_exp_mode command:
                   bypass/tap/disc for bypass/tap/disc mode
               set_wd_autoreset command:
                   WDT autoreset interval, msec (0 for disabling WDT autoreset).
if_scan      - refresh the list of network interfaces.
info         - print Program Information.
help         - print this message.
   Commands List:
is_bypass        - check if device is a Bypass/TAP controlling device
get_bypass_slave - get the second port participate in the Bypass/TAP pair
get_bypass_caps  - obtain Bypass/TAP capabilities information
get_wd_set_caps  - obtain watchdog timer settings capabilities
get_bypass_info  - get bypass/TAP info
set_bypass       - set Bypass mode state
get_bypass       - get Bypass mode state
get_bypass_change - get change of Bypass mode state from last status check
set_dis_bypass   - set Disable Bypass mode
get_dis_bypass   - get Disable Bypass mode state
set_bypass_pwoff - set Bypass mode at power-off state
get_bypass_pwoff - get Bypass mode at power-off state
set_bypass_pwup  - set Bypass mode at power-up state
get_bypass_pwup  - get Bypass mode at power-up state
set_std_nic      - set Standard NIC mode of operation
get_std_nic      - get Standard NIC mode settings
set_bypass_wd    - set watchdog state
get_bypass_wd    - get watchdog state
get_wd_time_expire - get watchdog expired time
reset_bypass_wd - reset watchdog timer
set_tx      - set transmit enable / disable
get_tx      - get transmitter state (enabled / disabled)
set_tpl      - set TPL enable / disable
get_tpl      - get TPL state (enabled / disabled)
set_hw_reset          - set hw_reset enable / disable
get_hw_reset          - get hw_reset (enabled / disabled)
set_tap       - set TAP mode state
get_tap       - get TAP mode state
get_tap_change - get change of TAP mode state from last status check
set_dis_tap   - set Disable TAP mode
get_dis_tap   - get Disable TAP mode state
set_tap_pwup  - set TAP mode at power-up state
get_tap_pwup  - get TAP mode at power-up state
set_disc       - set Disc mode state
get_disc       - get Disc mode state
get_disc_change - get change of Disc mode state from last status check
set_dis_disc   - set Disable Disc mode
get_dis_disc   - get Disable Disc mode state
set_disc_pwup  - set Disc mode at power-up state
get_disc_pwup  - get Disc mode at power-up state
set_wd_exp_mode - set adapter state when WDT expired
get_wd_exp_mode - get adapter state when WDT expired
set_wd_autoreset - set WDT autoreset mode
get_wd_autoreset - get WDT autoreset mode

Example: bpctl_util eth0 set_bypass_wd 5000
         bpctl_util all set_bypass on
         bpctl_util eth0 set_wd_exp_mode tap
         bpctl_util 0b:00.0 get_bypass_info
         
         
3.2 PROC interface.

Interaction with bypass functionality is done also through Linux proc interface.
After Ethernet driver installation, bypass control commands can be found at folder:

	/proc/net/bypass/bypass_eth# 
	
	
  All the commands are listed below.
  
bypass_slave
    Get the second port participate of the Bypass pair:
	cat bypass_slave
    
bypass_caps
    Obtain Bypass capabilities information:
	cat bypass_caps

wd_set_caps
    Obtain watchdog timer setting capabilities:
	cat wd_set_caps
 
bypass
    Set or get Bypass state.
    Disable Bypass:
	echo "off" > bypass
    Enable Bypass:		
	echo "on" > bypass
    Get Bypass state:
	cat bypass    
	
bypass_change
    Get change of Bypass state from last status check:
	cat bypass_change
    
bypass_wd
    Set watchdog state.
    For enable & activate WDT feature please retype:
        echo "<wdt_timeout(msec)>" > bypass_wd
    Get watchdog state:
	cat bypass_wd    
    
reset_bypass_wd
    Reset watchdog timer:
	cat reset_bypass_wd
	
wd_expire_time
    Get watchdog expired time:
	cat wd_expire_time
         
dis_bypass
    Set or get Disable Bypass mode.
    Set Disable Bypass mode off:
	echo "off" > dis_bypass
    Set Disable Bypass mode on:
	echo "on" > dis_bypass
    Get Disable Bypass mode state:
	cat dis_bypass

bypass_pwup
     Set or get Bypass state on power up.
     Set Enable Bypass on power up:
	echo "on" > bypass_pwup
     Set Disable Bypass on power up:
	echo "off" > bypass_pwup
     Get Bypass state on power up:
	cat bypass_pwup 
	  
bypass_pwoff
     Set or get Bypass state on power off.
     Set Enable Bypass on power off:
	echo "on" > bypass_pwoff
     Set Disable Bypass on power off:
	echo "off" > bypass_pwoff
     Get Bypass state on power off:
	cat bypass_pwoff 
	
std_nic
     Set or get Standard NIC mode.
     Set Standard NIC mode on:
	echo "on" > std_nic
     Set Standard NIC mode off:
	echo "off" > std_nic
     Get Standard NIC mode:
	cat std_nic   

tap
    Set or get TAP state.
    Disable TAP:
	echo "off" > tap
    Enable TAP:		
	echo "on" > tap
    Get TAP state:
	cat tap    
	
tap_change
    Get change of TAP state from last tap_change status check:
	cat tap_change
    
dis_tap
    Set or get Disable TAP mode.
    Set Disable TAP mode off:
	echo "off" > dis_tap
    Set Disable TAP mode on:
	echo "on" > dis_tap
    Get Disable TAP mode state:
	cat dis_tap

tap_pwup
     Set or get TAP state on power up.
     Set Enable TAP on power up:
	echo "on" > tap_pwup
     Set Disable TAP on power up:
	echo "off" > tap_pwup
     Get TAP state on power up:
	cat tap_pwup   
	
wd_exp_mode
     Set or get adapter state when WDT expired.
     Set TAP when WDT expired:
	echo "tap" > wd_exp_mode
     Set Bypass when WDT expired:
	echo "bypass" > wd_exp_mode
     Set Disconnect when WDT expired:
	echo "disc" > wd_exp_mode

     Get adapter state when WDT expired:
	cat wd_exp_mode

wd_autoreset
     Set or get WDT autoreset mode.   
     For setting WDT autoreset mode please retype:
        echo "<rst_period(msec)>" > wd_autoreset
    Get WDT autoreset mode state:
	cat wd_autoreset 
	
tpl
    Set or get TPL state.
    Disable TPL:
	echo "off" > tpl
    Enable TPL:		
	echo "on" > tpl
    Get TPL state:
	cat tpl 
	
disc
    Set or get Disconnect state.
    Disable Disconnect:
	echo "off" > disc
    Enable Disconnect:		
	echo "on" > disc
    Get Disconnect state:
	cat disc 
	
dis_disc
    Set or get Disable Disconnect mode.
    Set Disable Disconnect mode off:
	echo "off" > dis_disc
    Set Disable Disconnect mode on:
	echo "on" > dis_disc
    Get Disable Disconnect mode state:
	cat dis_disc

	
disc_pwup
     Set or get Disconnect state on power up.
     Set Enable Disconnect on power up:
	echo "on" > disc_pwup
     Set Disable Disconnect on power up:
	echo "off" > disc_pwup
     Get Disconnect state on power up:
	cat disc_pwup 
	
disc_change
    Get change of Disconnect state from last disc_change status check:
	cat disc_change   
	
bypass_info
     Get detailed info about Bypass device.
	cat bypass_info   


4. Software unloading.

    # bpctl_stop
 
5. Uninstall.

    # make uninstall
   
