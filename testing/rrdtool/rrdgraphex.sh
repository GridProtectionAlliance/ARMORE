#!/bin/bash
#apt-get install collectd
#apt-get install collectd-dev
#apt-get install rrdtool
#apt-get install python-rrdtool
#watch rrdtool graph blah.png --start 1458832790 DEF:load=/var//lib/collectd/rrd/ARMORETEST/load/load.rrd:shortterm:AVERAGE LINE2:load#FF0000
watch rrdtool graph blah.png --start 1458832790  \
--title "ARMORETEST" \
--slope-mode \
DEF:load=/var/lib/collectd/rrd/ARMORETEST/load/load.rrd:shortterm:AVERAGE \
DEF:pkt=/var/lib/collectd/rrd/ARMORETEST/interface-eth0/if_packets.rrd:rx:AVERAGE \
LINE2:load#FF0000 \
LINE3:pkt#000000
