#!/bin/bash
# Run this script to start the ARMORE web server under supervisor.  Should be run as root for now.

# -n: Run in foreground of terminal. Defaults to background
# -c <configFile>: Specify config file.  Defaults to /etc/supervisord.conf

supervisord -n -c supervisorConfig/supervisord.conf
