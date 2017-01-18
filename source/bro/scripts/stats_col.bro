# University of Illinois/NCSA Open Source License
# Copyright (c) 2015 Information Trust Institute
# All rights reserved.
#
# Developed by:
#
# Information Trust Institute
# University of Illinois
# http://www.iti.illinois.edu
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal with
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# Redistributions of source code must retain the above copyright notice, this list
# of conditions and the following disclaimers. Redistributions in binary form must
# reproduce the above copyright notice, this list of conditions and the following
# disclaimers in the documentation and/or other materials provided with the
# distribution.
#
# Neither the names of Information Trust Institute, University of Illinois, nor
# the names of its contributors may be used to endorse or promote products derived
# from this Software without specific prior written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
#
#
##! Extract the statistics of senders, receivers, protocols, functions and targets in traffic.

module StatsCol;

export {
    redef enum Log::ID += { LOG };

    ## Record for the data item.
    type Info: record {
        ## Timestamp when the data is extracted
        ts: time &log;

	    ## Packet length
	    len: count &log;

        ## Sender's IP
        sender: addr &log &optional;

        ## Receiver's IP
        receiver: addr &log &optional;

        ## Protocol name
        protocol: string &log &optional;

        ## Additional unit id
        uid: string &log &optional;

        ## Function name
        func: string &log &optional;

        ## Target name
        target: string &log &optional;

        ## Whether the item is a response or not
        is_response: bool &log &optional;

        ## Connection on which the item is found
        conn: connection &optional;
        
    };

    global log_stats_col: event(rec: Info);
    global item_gen: event(info: Info);
    global item_seen: event(info: Info);

    

    ## The number of level needed.
    global col_levels = 6 &redef;
    ## Whether to enable time measurement.
    global measure = F &redef;
}

global g_ts: time;                              # global variable to store the timestamp
global g_start: time;                           # global variable to store the start time of the program
global g_end: time;                             # global variable to store the end time of the program
global g_dur: interval;                         # global variable to store the duration of the program
global g_len: count;				            # global variable to store the length of the packet
global g_conn: connection;                      # global variable to store the connection
global g_src: addr;                             # global variable to store the sender's ip address
global g_dst: addr;                             # global variable to store the receiver's ip address
global g_pro: string;                           # global variable to store the protocol name
global g_uid: string;                           # global variable to store the unit id
global g_fun: string;                           # global variable to store the function name
global g_is_response: bool;                     # global variable to indicate whether the packet is a response
global g_tgt: string;                           # global variable to store all the information about the target
global g_tgt_table: table[string] of string;    # global variable to store all the information about the target for each connection
global g_wait = F;                              # global variable to indicate whether the collector is waiting for lower level information
global g_info: Info;                            # global variable to store the current data item

global g_time_10: time;                         # global variable to store the timestamp to start the sender and receiver extractor
global g_time_11: time;                         # global variable to store the timestamp to get the sender and receiver of each packet
global g_time_12: time;                         # global variable to store the timestamp to trigger the first item_seen event

global g_time_20: time;                         # global variable to store the timestamp to start the protocol and function extractor
global g_time_21: time;                         # global variable to store the timestamp to get the protocol and function of each packet
global g_time_22: time;                         # global variable to store the timestamp to trigger the second item_seen event

global g_time_30: time;                         # global variable to store the timestamp to start the target extractor
global g_time_31: time;                         # global variable to store the timestamp to get the targets of each packet
global g_time_32: time;                         # global variable to store the timestamp to trigger the third item_seen event

global g_time_l2: time;                         # global variable to store the timestamp to trigger the last item_seen event

global g_time_40: time;                         # global variable to store the timestamp to start triggering the item_gen of each packet

global g_packet_11: interval = 0sec;            # global variable to store the time to get the sender and receiver of each packet
global g_packet_12: interval = 0sec;            # global variable to store the time to trigger the first item_seen event
global g_packet_13: interval = 0sec;            # global variable to store the time to receive the first item_seen event

global g_packet_20: interval = 0sec;            # global variable to store the time to start the protocol and function extractor
global g_packet_21: interval = 0sec;            # global variable to store the time to get the protocol and function of each packet
global g_packet_22: interval = 0sec;            # global variable to store the time to trigger the second item_seen event
global g_packet_23: interval = 0sec;            # global variable to store the time to receive the second item_seen event

global g_packet_30: interval = 0sec;            # global variable to store the time to start the target extractor
global g_packet_31: interval = 0sec;            # global variable to store the time to get the targets of each packet
global g_packet_32: interval = 0sec;            # global variable to store the time to trigger the third item_seen event
global g_packet_33: interval = 0sec;            # global variable to store the time to receive the third item_seen event

global g_packet_40: interval = 0sec;            # global variable to store the time to start triggering the item_gen of each packet
global g_packet_41: interval = 0sec;            # global variable to store the time to trigger the item_gen of each packet
global g_packet_42: interval = 0sec;            # global variable to store the time to receive the item_gen of each packet

global g_packet_c1: count = 0;                  # global variable to store the number of first item_seen event
global g_packet_c2: count = 0;                  # global variable to store the number of second item_seen event
global g_packet_c3: count = 0;                  # global variable to store the number of third item_seen event
global g_packet_c4: count = 0;                  # global variable to store the number of item_gen event

global g_packet_process: interval = 0sec;       # global variable to store the process time of each packet

# Create a vector containing counts from lower to upper
function range(lower: count, upper: count): vector of count 
{
    if (lower > upper)
    {
        return vector();
    }
    else
    {
        local v: vector of count = vector(lower);
        local i: count = 1;
        local r: vector of count = range(lower + 1, upper);
        for (n in r)
        {
            v[i] = r[n];
            ++i;
        }
        return v;
    }
}

# A helper function to generate the target value
function target_gen(name: string, start_address: count, quantity: count): string
{
    if (quantity == 0)
    {
        return cat(name, ":", "Nothing");
    }
    else if (quantity == 1)
    {
        return cat(name, ":", start_address);
    }
    else
    {
        return cat(name, ":", start_address, "-", start_address+quantity-1);
    }
}

event bro_init() &priority=5
{
    Log::create_stream(LOG, [$columns=Info, $ev=log_stats_col]);
    if (measure)
    {
        g_start = current_time();
    }
}

event item_gen(info: Info)
{
    if (measure)
    {
        g_packet_42 += current_time() - g_time_40;
        g_packet_c4 += 1;
        g_packet_process += current_time() - g_time_10;
    }
    Log::write(StatsCol::LOG, info);
}

# Count the senders and receivers
event new_packet(c: connection, p: pkt_hdr) &priority=5
{
    g_ts = network_time();

    if(g_wait)
    {
        if (measure)
        {
            g_time_40 = current_time();
            g_packet_40 += g_time_40 - g_time_l2;
        }
        event StatsCol::item_gen(g_info);
        if (measure)
        {
            g_packet_41 += current_time() - g_time_40;
        }
    }

    if (measure)
    {
        g_time_10 = current_time();
    }

    g_conn = c;

    if (p?$ip)
    {
	g_len = p$ip$len;
        g_src = p$ip$src;
        if(col_levels >= 2)
        {
            g_dst = p$ip$dst;
        }
    }
    else
    {
	g_len = p$ip6$len;
        g_src = p$ip6$src;
        if(col_levels >= 2)
        {
            g_dst = p$ip6$dst;
        }
    }

    if (col_levels <= 1)
    {
         g_info = [$ts=g_ts, $len=g_len, $sender=g_src, $conn=g_conn];
    }
    else
    {
         g_info = [$ts=g_ts, $len=g_len, $sender=g_src, $receiver=g_dst, $conn=g_conn];
    }

    if (col_levels <= 2)
    {
         if (measure)
         {
             g_time_11 = current_time();
             g_packet_11 += g_time_11 - g_time_10;
         }
         event StatsCol::item_seen(g_info);
         if (measure)
         {
             g_time_12 = current_time();
             g_packet_12 += g_time_12 - g_time_11;
             g_time_40 = current_time();
             g_packet_40 += g_time_40 - g_time_12;
         }
         event StatsCol::item_gen(g_info);
         if (measure)
         {
             g_packet_41 += current_time() - g_time_40;
         }
    }
    else
    {
         g_wait = T;
         if (measure)
         {
             g_time_11 = current_time();
             g_packet_11 += g_time_11 - g_time_10;
         }
         event StatsCol::item_seen([$ts=g_ts, $len=g_len, $sender=g_src, $receiver=g_dst, $conn=g_conn]);
         if (measure)
         {
             g_time_12 = current_time();
             g_packet_12 += g_time_12 - g_time_11;
             g_time_l2 = g_time_12;
         }
    }
}

event StatsCol::item_seen(item: StatsCol::Info)
{
    if (measure)
    {
        local temp_time = current_time();
        
        if(!item?$protocol)
        {
            g_packet_13 += temp_time - g_time_11;
            g_packet_c1 += 1;
        }
        else if(!item?$target)
        {
            g_packet_23 += temp_time - g_time_21;
            g_packet_c2 += 1;
        }
        else
        {
            g_packet_33 += temp_time - g_time_31;
            g_packet_c3 += 1;
        }
    }
}

event bro_done()
{
    if (measure)
    {
        g_dur = current_time() - g_start;
        print fmt("Duration: %s", g_dur);

        g_packet_11 = g_packet_11/g_packet_c1;
        g_packet_12 = g_packet_12/g_packet_c1;
        g_packet_13 = g_packet_13/g_packet_c1;
        print fmt("T11: %s", g_packet_11);
        print fmt("T12: %s", g_packet_12);
        print fmt("T13: %s", g_packet_13);

        if (col_levels >= 3)
        {
            g_packet_20 = g_packet_20/g_packet_c2;
            g_packet_21 = g_packet_21/g_packet_c2;
            g_packet_22 = g_packet_22/g_packet_c2;
            g_packet_23 = g_packet_23/g_packet_c2;
            print fmt("T20: %s", g_packet_20);
            print fmt("T21: %s", g_packet_21);
            print fmt("T22: %s", g_packet_22);
            print fmt("T23: %s", g_packet_23);
        }

        if (col_levels == 6)
        {
            g_packet_30 = g_packet_30/g_packet_c3;
            g_packet_31 = g_packet_31/g_packet_c3;
            g_packet_32 = g_packet_32/g_packet_c3;
            g_packet_33 = g_packet_33/g_packet_c3;
            print fmt("T30: %s", g_packet_30);
            print fmt("T31: %s", g_packet_31);
            print fmt("T32: %s", g_packet_32);
            print fmt("T33: %s", g_packet_33);
        }

        g_packet_40 = g_packet_40/g_packet_c4;
        g_packet_41 = g_packet_41/g_packet_c4;
        g_packet_42 = g_packet_42/g_packet_c4;
        print fmt("T40: %s", g_packet_40);
        print fmt("T41: %s", g_packet_41);
        print fmt("T42: %s", g_packet_42);

        g_packet_process = g_packet_process/g_packet_c4;
        print fmt("Process: %s", g_packet_process);
    }
}

@load stats_col_modbus.bro
@load stats_col_dnp3.bro
