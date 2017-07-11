#!/bin/bash

set -e

###############################################################################
#Note:
#This script runs every hour + 15min and collects all logs from the previous
#hour to put into visualization database
###############################################################################

####Folder Locations
ARMORELOG=/var/log/armore/stats
ARMOREBASE=/var/log/armore/base
ARMOREANOM=/var/log/armore/anomalies

BRODIR=/var/log/bro
CREATEJSON=/var/webServer/domains/lib


#Date-Hour 
CURRENTHR="$(date +%H)"

#Day changes at midnight so we should pull logs from yesterday from 23:00-00:00
  if [ "$CURRENTHR" == "00" ];
   then
    CURRENTDATE="$(date --date='1 day ago' +%Y-%m-%d)"
   else
    CURRENTDATE="$(date +%Y-%m-%d)"
  fi

PRIORHR="$(date --date='1 hour ago' +%H)"

echo $PRIORHR
echo $CURRENTDATE

#Delete stats directory at each run if it exists
  if [ -d "$ARMORELOG" ];
   then
    rm -rf $ARMORELOG
  fi

#Delete baseline directory at each run if it exists
  if [ -d "$ARMOREBASE" ];
   then
    rm -rf $ARMOREBASE
  fi

#Delete anomaly directory at each run if it exists
  if [ -d "$ARMOREANOM" ];
   then
    rm -rf $ARMOREANOM
  fi

#Note: Bro will stop sometimes so there must be a 1:1 match for stats_col/count
#Script will ignore non matching sets
for statlogs in `ls -1 $BRODIR/$CURRENTDATE \
  | grep stats_col.$PRIORHR:* \
  | sed 's/stats_col.//g' \
  | sed 's/.log.gz//g'`; do

  if [ -e  "$BRODIR/$CURRENTDATE/stats_count.$statlogs.log.gz" ];
  then
    echo $statlogs
    mkdir -p "$ARMORELOG/$CURRENTDATE"_"$statlogs"

    cp $BRODIR/$CURRENTDATE/stats_col.$statlogs.log.gz \
      "$ARMORELOG/$CURRENTDATE"_"$statlogs"/stats_col.log.gz

    cp $BRODIR/$CURRENTDATE/stats_count.$statlogs.log.gz \
      "$ARMORELOG/$CURRENTDATE"_"$statlogs"/stats_count.log.gz

    #gzip -d -f "$ARMORELOG/$CURRENTDATE"_"$statlogs"/*
    
    python3 $CREATEJSON/create_json_graph.py -v -t "$CURRENTDATE"_"$statlogs" -i \
      "$ARMORELOG/$CURRENTDATE"_"$statlogs"

  fi
done
	

#Note: baseline will be generated every x hrs on a restart
for baseline in `ls -1 $BRODIR/$CURRENTDATE \
  | grep anodet_log_base.$PRIORHR:* \
  | sed 's/anodet_log_base.//g' \
  | sed 's/.log.gz//g'`; do

  if [ -e  "$BRODIR/$CURRENTDATE/anodet_log_base.$baseline.log.gz" ];
  then

    echo $baseline
    mkdir -p "$ARMOREBASE/$CURRENTDATE"_"$baseline"


    cp $BRODIR/$CURRENTDATE/anodet_log_base.$baseline.log.gz \
      "$ARMOREBASE/$CURRENTDATE"_"$baseline"/anodet_log_base.log.gz

    gzip -d -f "$ARMOREBASE/$CURRENTDATE"_"$baseline"/*
    
    python3 $CREATEJSON/create_json_graph.py -t "$CURRENTDATE"_"$baseline" -b \
      "$ARMOREBASE/$CURRENTDATE"_"$baseline"/anodet_log_base* 

  fi

done

#Note: anomalies will be generated every hr after baseline
for anom in `ls -1 $BRODIR/$CURRENTDATE \
  | grep anodet_log_ano.$PRIORHR:* \
  | sed 's/anodet_log_ano.//g' \
  | sed 's/.log.gz//g'`; do

  if [ -e  "$BRODIR/$CURRENTDATE/anodet_log_ano.$anom.log.gz" ];
  then

    echo $anom
    mkdir -p "$ARMOREANOM/$CURRENTDATE"_"$anom"


    cp $BRODIR/$CURRENTDATE/anodet_log_ano.$anom.log.gz \
      "$ARMOREANOM/$CURRENTDATE"_"$anom"/anodet_log_ano.log.gz

    gzip -d -f "$ARMOREANOM/$CURRENTDATE"_"$anom"/*
    
    python3 $CREATEJSON/create_json_graph.py -t "$CURRENTDATE"_"$anom" -a \
      "$ARMOREANOM/$CURRENTDATE"_"$anom"/anodet_log_ano* 

  fi

done
