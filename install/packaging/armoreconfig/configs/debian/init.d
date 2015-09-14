#!/bin/bash
#This Script manages the services needed for an ARMOREnode
### BEGIN INIT INFO
# Provides:          armoreconfig
# Required-Start:    $network $local_fs
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: ARMOREnode service
# Description:       Enables ARMOREnode services
### END INIT INFO

# Author: Steve Granda <sgrand2@illinois.edu>

PATH=/sbin:/usr/sbin:/bin:/usr/bin
DESC=armoreconfig #Introduce a short description here
NAME=ARMOREProxy # Introduce the short server's name here
DAEMON=/usr/share/armoreproxy/ARMOREProxy #Introduce the server's location here
DAEMON_ARGS=""             # Arguments to run the daemon with
PROXYPIDFILE=/var/run/$NAME.pid
SCRIPTNAME=/etc/init.d/$NAME

# Read configuration variable file if it is present
[ -r /etc/default/$NAME ] && . /etc/default/$NAME

# Load the VERBOSE setting and other rcS variables
. /lib/init/vars.sh

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.0-6) to ensure that this file is present.
. /lib/lsb/init-functions
. /etc/armore/armore-utils.sh

#
#Check Kernel Version. Without correct version, software will not work
#
KERNPACK="$(uname -ra | grep -ci 3.12.0)"

if [ $KERNPACK -ne "1" ];
then
    log_failure_msg "Wrong Kernel Version ArmoreNode not running. If you just installed ARMORENode please restart" 
    exit 0
fi

#
# Function that starts the daemon/service
#
do_start()
{
  #Make system do ip_forwarding
  log_success_msg "Enabling IPv4 forwarding"
  sysctl -w net.ipv4.ip_forward=1


  #Check to see if netmap modules are loaded 
  if checkmodules -ne "1" ;
  then
    log_failure_msg "NetMap module could not be loaded. ARMORENode not running"
    exit 
  fi

  #Read in proxy configuration
  APROXYCFG="/etc/armore/proxycfg"

  if [ -e "$APROXYCFG" ]
  then

    PORT="$(cat $APROXYCFG | grep Port\ = | sed 's/Port\ =//' | sed 's/ //')"
    Roletype="$(cat $APROXYCFG | grep Roletype\ = | sed 's/Roletype\ =//' | sed 's/ //')"
    Interface="$(cat $APROXYCFG | grep Interface\ = | sed 's/Interface\ =//' | sed 's/ //')"
    Bind="$(cat $APROXYCFG | grep Bind\ = | sed 's/Bind\ =//' | sed 's/ //')"
    Connect="$(cat $APROXYCFG | grep Connect\ = | sed 's/Connect\ =//' | sed 's/ //')"
    Operation="$(cat $APROXYCFG | grep Operation\ = | sed 's/Operation\ =//' | sed 's/ //')"
    Capture="$(cat $APROXYCFG | grep Capture\ = | sed 's/Capture\ =//' | sed 's/ //')"
    Log="$(cat $APROXYCFG | grep Log\ = | sed 's/Log\ =//' | sed 's/ //')"
    PROMSET="$(ifconfig $Interface promisc)"
    DAEMON_ARGS=""


    if [ "$Roletype" == "Server" ];
    then
      DAEMON_ARGS="-s "
    fi
    
    if [ "$Capture" == "Pcap" ];
    then
      DAEMON_ARGS="$DAEMON_ARGS -c pcap $Interface "
    else
      DAEMON_ARGS="$DAEMON_ARGS $Interface "
    fi

    if [ "$BIND" == "" ] && [ "$Roletype" == "Server" ];
    then 
      DAEMON_ARGS="$DAEMON_ARGS *:$PORT"
    else
      DAEMON_ARGS="$DAEMON_ARGS $BIND;$Connect:$PORT"
    fi
    
    #DAEMON_ARGS="-s -i 0 -c pcap -m zeromq $LIFACE *:$PORT"
    #echo $PORT $Roletype $Interface $Bind $Connect $Operation $Capture $Log
    #echo "---------------------cut here-----------------"
    #echo "$DAEMON_ARGS"



    NODE_ARGS="/usr/share/statsd/stats.js /etc/statsd/localConfig.js"
    NODENAME="node"
    NODEDAEMON="/usr/bin/node"

    #Start ARMOREProxy with settings from APROXYCFG in /etc/armore/proxycfg
    log_success_msg "Starting ARMORE Proxy on $LIFACE $PORT"
    start-stop-daemon --start --background --quiet --name $NAME --exec $DAEMON -- $DAEMON_ARGS > $Log


  else
    log_failure_msg "ARMOREProxy configuration not found. ARMORENode not running"

  fi

  #Start Carbon Service to begin collecting statsd metrics
  log_success_msg "Starting Carbon"
  carbon-cache --config=/etc/carbon/carbon.conf start

  #Start NodeJS to begin collecting StatsD via Bro-StatsD-Plugin
  log_success_msg "Starting Node"
  start-stop-daemon --start --background --quiet --name $NODENAME --exec /usr/bin/node -- $NODE_ARGS > $Log 

  log_success_msg "Starting Bro"
  broctl start

}

#
# Function that stops the daemon/service
#
do_stop()
{
    
  log_success_msg "Stopping ARMORE Proxy"
  start-stop-daemon --stop --quiet --signal QUIT --name $NAME
  
  log_success_msg "Stopping Bro"
  broctl stop
  
  log_success_msg "Stopping Node Daemon"
  start-stop-daemon --stop --quiet -x /usr/bin/node
  
  log_success_msg "Stopping Carbon"
  carbon-cache --config=/etc/carbon/carbon.conf stop 
  
  log_success_msg "Disabling IPv4 forwarding"
  sysctl -w net.ipv4.ip_forward=0
}

#
# Function that sends a SIGHUP to the daemon/service
#
do_reload() {

  log_success_msg "Reloading Bro"
  broctl restart
}

do_check() {

  #Check if ARMOREProxy is running
  status_of_proc "$DAEMON" "$NAME" #&& exit 0 || exit $?

  CARBONSTAT="$(carbon-cache --config=/etc/carbon/carbon.conf status | grep -ci pid)"
  STATSD="$(netstat -tuplv | grep -ci localConfig)"
  BROSTAT="$(ps awux | grep -ci local.bro)"

  
  #Check if Carbon and StatsD are running. We also need Apache2

  if [ $CARBONSTAT -ge "1" ] && [ $STATSD -ge "1" ];
  then
    log_success_msg "Visual Stats are being collected" 
  else
    log_failure_msg "Visual Stats are NOT being collected" 
  fi

  #Check that an instance of Bro is running

  if [ $BROSTAT -ge "1" ];
  then
    log_success_msg "Bro is running"
  else  
    log_failure_msg "Bro is not running"
  fi

}

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
