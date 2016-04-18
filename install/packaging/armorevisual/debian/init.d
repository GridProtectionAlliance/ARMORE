#!/bin/sh
### BEGIN INIT INFO
# Provides:          armorevisual
# Required-Start:    $local_fs $network $remote_fs $syslog
# Required-Stop:     $local_fs $network $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: ARMORE Visual
# Description:       ARMORE Visual services and stats
### END INIT INFO

# Author: sgrand2 <sgrand2@illinois.edu>


PATH=/sbin:/usr/sbin:/bin:/usr/bin
DESC="ARMORE WEBSERVICE"
NAME=armorevisual
DAEMON=
DAEMON_ARGS=""
SCRIPTNAME=/etc/init.d/$NAME

# Exit if the package is not installed
#[ -x "$DAEMON" ] || exit 0

## Read configuration variable file if it is present
##[ -r /etc/default/$NAME ] && . /etc/default/$NAME

# Load the VERBOSE setting and other rcS variables
. /lib/init/vars.sh

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.2-14) to ensure that this file is present
# and status_of_proc is working.
. /lib/lsb/init-functions

#
# Function that starts the daemon/service
#
do_start()
{
	carbon-cache --config=/etc/carbon/carbon.conf start
	start-stop-daemon --start --background --quiet --exec /usr/bin/nodejs -- /usr/share/statsd/stats.js /etc/statsd/localConfig.js
	start-stop-daemon --start --background --chdir /var/www/webServer --exec /usr/bin/python3 -- /var/www/webServer/armoreServer.py


}

#
# Function that stops the daemon/service
#
do_stop()
{
	carbon-cache --config=/etc/carbon/carbon.conf stop
	start-stop-daemon --stop --exec /usr/bin/nodejs
	start-stop-daemon --stop --exec /usr/bin/python3
}

#
# Function that sends a SIGHUP to the daemon/service
#
do_reload() {
	carbon-cache --config=/etc/carbon/carbon.conf stop
	start-stop-daemon --stop --exec /usr/bin/nodejs
	
	carbon-cache --config=/etc/carbon/carbon.conf start
	start-stop-daemon --start --background --quiet --exec /usr/bin/nodejs -- /usr/share/statsd/stats.js /etc/statsd/localConfig.js
	
	
	start-stop-daemon --stop --exec /usr/bin/python3
	start-stop-daemon --start --background --chdir /var/www/webServer --exec /usr/bin/python3 -- /var/www/webServer/armoreServer.py
}

case "$1" in
  start)
	[ "$VERBOSE" != no ] && log_daemon_msg "Starting $DESC" "$NAME"
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

	CARBON="$(pgrep -c carbon-cache)"
	if [ $CARBON -eq "0" ];
	then
	  log_failure_msg "carbon-cache service not running"
	else
	  log_success_msg "Carbon-Cache service is running"
	fi

	NODESVC="$(netstat -tuplv | grep -ci localConfig.js)"
	if [ $NODESVC -eq "0" ];
	then
	  log_failure_msg "Nodejs service not running"
	else
	  log_success_msg "Nodejs service is running"
	fi
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
