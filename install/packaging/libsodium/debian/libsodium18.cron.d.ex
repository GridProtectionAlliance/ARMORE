#
# Regular cron jobs for the libsodium18 package
#
0 4	* * *	root	[ -x /usr/bin/libsodium18_maintenance ] && /usr/bin/libsodium18_maintenance
