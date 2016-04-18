#!/bin/bash
#This Script manages the services needed for an ARMOREnode
### BEGIN INIT INFO
# Provides:          armoreconfig
# Required-Start:    $network $local_fs $syslog
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: ARMORE service
# Description:       ARMORE service manager
### END INIT INFO

# Author: Steve Granda <sgrand2@illinois.edu>

###############################################################################
#Initialization Vars
###############################################################################

PATH=/sbin:/usr/sbin:/bin:/usr/bin
DESC=ARMORE 				#Introduce a short description here
NAME=ARMOREProxy 			#Introduce the short server's name here
DAEMON=/usr/bin/ARMOREProxy 		#Introduce the server's location here
DAEMON_ARGS=""             		#Arguments to run the daemon with
#PROXYPIDFILE=/var/run/$NAME.pid #REMOVE_TEST
SCRIPTNAME=/etc/init.d/$NAME

# Load the VERBOSE setting and other rcS variables
. /lib/init/vars.sh

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.0-6) to ensure that this file is present.
. /lib/lsb/init-functions
##. /etc/armore/armore-utils.sh

#Check Kernel Version.
KERNPACK="$(uname -ra | grep -ci 3.18.21-armore)"

#Check Kernel is loaded
if [ $KERNPACK -ne "1" ];
then
    log_failure_msg "ArmoreKernel not running. If you just installed ARMORENode please restart" 
    exit 0 
fi

#Read configuration and set variables
  APROXYCFG="/etc/armore/armoreconfig"

    if [ -e "$APROXYCFG" ]
      then

	Operation="$(cat $APROXYCFG | grep Operation\ = | sed 's/Operation\ =//' | sed 's/ //')"
	Roletype="$(cat $APROXYCFG | grep Roletype\ = | sed 's/Roletype\ =//' | sed 's/ //')"
	Interface="$(cat $APROXYCFG | grep Interface\ = | sed 's/Interface\ =//' | sed 's/ //')"
	Encryption="$(cat $APROXYCFG | grep Encryption\ = | sed 's/Encryption\ =//' | sed 's/ //')"
	Bind="$(cat $APROXYCFG | grep Bind\ = | sed 's/Bind\ =//' | sed 's/ //')"
	Port="$(cat $APROXYCFG | grep Port\ = | sed 's/Port\ =//' | sed 's/ //')"
	Capture="$(cat $APROXYCFG | grep Capture\ = | sed 's/Capture\ =//' | sed 's/ //')"

#Flag variables for daemon to run with
	Roleflag=""
	Encryptionflag=""
	Captureflag=""
	fi


###############################################################################
# Latch
##############################################################################
do_latch()
{
  log_success_msg "Enabling Hardware Bypass Mode"
}
###############################################################################
# Start Monitoring
##############################################################################
do_monitor()
{
#Start/Check for Bro instance
  BROSTAT="$(pgrep -c bro)"
    if [ $BROSTAT -eq "0" ];
  then
    log_success_msg "Starting Bro"
    broctl start
    else
      log_warning_msg "Bro already running"
	fi
}
###############################################################################
# Start Proxy
##############################################################################
do_proxy()
{
  if [ "$Capture" == "NetMap" ];
  then

#Check NetMap kmod is loaded

    NETMAPCHECK="$(lsmod | grep -ci netmap)"
    if [ "$NETMAPCHECK" -lt "1" ];
  then 
    log_warning_msg "Loading NetMap Module"
    insmod /lib/modules/3.18.21-armore/kernel/net/armorenetmap/netmap.ko
    fi

    Captureflag="-c netmap"
    fi

#Specify Pcap as capture method

    if [ "$Capture" == "Pcap" ];
  then
    Captureflag="-c pcap"
    fi

#Now we check to see if we're running a server
    if [ "$Roletype" == "Server" ];
  then
    Roleflag="-s"
    else
      Roleflag=""
	fi


#Check if encryption is enabled
	if [ "$Encryption" == "Enabled" ];
  then
    if [ -d "/etc/armore/.curve" ];
  then
    Encryptionflag="-e"
    else
      log_failure_msg "Certificates could not be found in /etc/armore. ARMORE running unencrypted!"
	Encryptionflag=""
	fi

	else
	  Encryptionflag=""
	    fi

}
###############################################################################
# Start Services
##############################################################################
do_start()
{


	if [ "$Operation" == "Proxy" ];
  then
    do_proxy

    log_success_msg "Starting ARMORE Proxy on $Interface $Bind:$Port"
    DAEMON_ARGS="$Roleflag $Encryptionflag $Interface $Bind:$Port"
    start-stop-daemon --start --chdir /etc/armore/ --background --quiet --name $NAME --exec $DAEMON -- $DAEMON_ARGS

#Begin Monitoring Services
    do_monitor

    fi

    if [ "$Operation" == "Passive" ];
  then
    do_monitor
    fi

    if [ "$Operation" == "Transparent" ];
  then
    do_monitor
    fi

}
###############################################################################
# Stop Service
###############################################################################
do_stop()
{
  if [ "$Operation" == "Proxy" ];
  then
    APROXYCFG="/etc/armore/armoreconfig"
    log_success_msg "Stopping ARMORE Proxy"
    start-stop-daemon --stop --quiet --signal QUIT --name $NAME
    fi

    BROSTAT="$(pgrep -c bro)"
    if [ $BROSTAT -ne "0" ];
  then
    log_success_msg "Stopping Bro"
    broctl stop
    else
      log_warning_msg "Bro already stopped"
	fi
}

###############################################################################
# Function that sends a SIGHUP to the daemon/service
###############################################################################
do_reload() {

  log_success_msg "Reloading Bro"
    broctl install
    broctl restart

    do_stop
    do_start

}

###############################################################################
#Check status
###############################################################################
do_check() 
{
  if [ "$Operation" == "Proxy" ];
  then
    status_of_proc "$DAEMON" "$NAME" #$$ exit 0 || exit $?
    fi

    BROSTAT="$(pgrep -c bro)"

    if [ $BROSTAT -ne "0" ];
  then
    log_success_msg "Bro is running"
    else
      log_failure_msg "Bro is not running"
	fi

}

###############################################################################
#Service Definitions
###############################################################################

case "$1" in
  start)
    [ "$VERBOSE" != no ] && log_daemon_msg "Starting $DESC " "$NAME"
    do_start
    case "$?" in
		0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
		2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
	esac
  ;;
  stop)
	[ "$VERBOSE" != no ] && log_daemon_msg "Stopping $DESC" "$NAME"
	do_stop
	case "$?" in
		0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
		2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
	esac
	;;
  status)
  do_check

       ;;
  #reload|force-reload)
	#
	# If do_reload() is not implemented then leave this commented out
	# and leave 'force-reload' as an alias for 'restart'.
	#
	#log_daemon_msg "Reloading $DESC" "$NAME"
	#do_reload
	#log_end_msg $?
	#;;
  restart|force-reload)
	#
	# If the "reload" option is implemented then remove the
	# 'force-reload' alias
	#
	log_daemon_msg "Restarting $DESC" "$NAME"
	do_stop
	case "$?" in
	  0|1)
		do_start
		case "$?" in
			0) log_end_msg 0 ;;
			1) log_end_msg 1 ;; # Old process is still running
			*) log_end_msg 1 ;; # Failed to start
		esac
		;;
	  *)
	  	# Failed to stop
		log_end_msg 1
		;;
	esac
	;;
  *)
	#echo "Usage: $SCRIPTNAME {start|stop|restart|reload|force-reload}" >&2
	echo "Usage: $SCRIPTNAME {start|stop|status|restart|force-reload}" >&2
	exit 3
	;;
esac

:
