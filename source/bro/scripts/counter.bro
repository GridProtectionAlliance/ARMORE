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
#! Count number of senders, receivers, protocols, functions and targets in traffic and build a tree structure to hold the statistics.

@load base/frameworks/sumstats
@load base/bif/change_sum.bro

module StatisticAnalyzer;

export {
        redef enum Log::ID += { LOG };

        type Level5_Value: record {
            ## The name and the index of the target.
            tgt: table[string] of count;
            ## Count of the target.
            num: count;
        };
        ## The value record for the target-level.

        type Level5_Map: table[string] of Level5_Value;
        ## The table for the target-level.

        type Level4_Value: record {
            ## The target-level table.
            value: Level5_Map &optional;
            ## Count of the function.
            num: count;
        };
        ## The value record for the function-level.

        type Level4_Map: table[string] of Level4_Value;
        ## The table for the function-level.

        type Level3_Value: record {
            ## The function-level table.
            value: Level4_Map &optional;
            ## Count of the protocol.
            num: count;
        };
        ## The value record for the protocol-level

        type Level3_Map: table[string] of Level3_Value;
        ## The table for the protocol-level.

        type Level2_Value: record {
            ## The protocol-level table.
            value: Level3_Map &optional;
            ## Count of the receiver.
            num: count;
        };
        ## The value record for the receiver-level.

        type Level2_Map: table[addr] of Level2_Value;
        ## The table for the receiver-level.

        type Level1_Value: record {
            ## The receiver-level table.
            value: Level2_Map &optional;
            ## Count of the sender.
            num: count;
        };
        ## The value record for the sender-level.

        type Level1_Map: table[addr] of Level1_Value;
        ## The table for the sender-level.

        ## The data structure that contains all the statistics.
        global map: Level1_Map;

        ## The number of level needed.
        global levels = 5 &redef;

        ## Three potential sampling intervals
        global sa1 = 1min &redef;
        global sa2 = 5min &redef;
        global sa3 = 20min &redef;

        ## The sampling interval of the analyzer.
        global sample_interval = sa1 &redef;

        ## The lower threshold to control the sample interval.
        global low_thre = 20 &redef;

        ## The upper threshold to control the sample interval.
        global up_thre = 100 &redef;

        ## Whether to use the dynamic sample interval or not.
        global dynamic_interval = F &redef;

        ## Output the statistics for the previous period
        global print_stats: function();

        ## Record used for the logging framework.
        type Info: record {
            ## Timestamp when the data is written.
            ts: time &log;

            ## Length of the last period
            period: interval &log;

            ## Sender's IP
            sender: string &log &default=" - ";

            ## Receiver's IP
            receiver: string &log &default=" - ";

            ## Protocol name
            protocol: string &log &default=" - ";

            ## Function name
            func: string &log &default=" - ";

            ## Target name
            target: string &log &default=" - ";

            ## Number of instances seen in the period
            num: count &log &default=0;
        };

        global log_counter: event(rec: Info);
}

    global g_src: addr;                             # global variable to store the sender's ip address
    global g_dst: addr;                             # global variable to store the receiver's ip address
    global g_pro: string;                           # global variable to store the protocol name
    global g_fun: string;                           # global variable to store the function name
    global g_tgt: vector of table[string] of count; # global variable to store all the information about the target
    global g_ss: SumStats::SumStat;                 # global variable to store the Sumstat

    # Log the statistics for the previous period
    global log_stats: function();

    # Update the interval
    global update_interval: function(total: count);

    #Process the result for each epoch
    global process_result: function (ts: time, key: SumStats::Key, result: SumStats::Result);

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

# Now we can write our script using the new sumstat plugin that we just created.
# This is basically equivalent to what we did before

event bro_init() &priority=5
{
    Log::create_stream(LOG, [$columns=Info, $ev=log_counter]);
}

# Declare the reducers
event bro_init()
{
    local r1 = SumStats::Reducer($stream="senders", $apply=set(SumStats::COUNTTABLE));
    local r2 = SumStats::Reducer($stream="receivers", $apply=set(SumStats::COUNTTABLE));
    local r3_modbus = SumStats::Reducer($stream="modbus", $apply=set(SumStats::COUNTTABLE));
    local r3_dnp3 = SumStats::Reducer($stream="dnp3", $apply=set(SumStats::COUNTTABLE));
    local r4_modbus = SumStats::Reducer($stream="modbus_fun", $apply=set(SumStats::COUNTTABLE));
    local r4_dnp3 = SumStats::Reducer($stream="dnp3_fun", $apply=set(SumStats::COUNTTABLE));
    local r5_modbus = SumStats::Reducer($stream="modbus_tgt", $apply=set(SumStats::COUNTTABLE));
    local r5_dnp3 = SumStats::Reducer($stream="dnp3_tgt", $apply=set(SumStats::COUNTTABLE));

    g_ss = [$name="counters", $epoch=sample_interval, $reducers=set(r1,r2,r3_modbus,r3_dnp3,r4_modbus,r4_dnp3,r5_modbus,r5_dnp3),
        $epoch_result=process_result];

    SumStats::create(g_ss);
}

#Process the result for each epoch
function process_result(ts: time, key: SumStats::Key, result: SumStats::Result)
{
            map = table();
            local r: SumStats::ResultVal;
            local counttable: table[string] of count;
            local src: addr;
            local dst: addr;
            local indexes: string_vec;
            local pro: string;
            local fun: string;
            local tgt_string: string;
            local sub_tgts: string_vec;
            local v: vector of count;
            local ind: count;
            local tgt: table[string] of count;
            local sub_tgt: string_vec;
            local total = 0;

            if(levels >= 1)
            {
                if("senders" in result)
                {
                    r = result["senders"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }
                    counttable = r$counttable;
                    for ( i in counttable )
                    {
                        src = to_addr(i);
                        if ( ! (src in map))
                        {
                            map[src] = [$num=0];
                        }
                        map[src]$num = counttable[i];
                        total = total+counttable[i];
                        #print fmt("Sender: %s, count: %d", i, counttable[i]);
                    }

                    #print fmt("%d", total);
                }
            }

            if(levels >= 2)
            {
                if("receivers" in result)
                {
                    r = result["receivers"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }
                    counttable = r$counttable;

                    for ( i in counttable )
                    {
                        indexes = split_string(i, /;/);
                        src = to_addr(indexes[0]);
                        dst = to_addr(indexes[1]);
                        if ( ! map[src]?$value )
                        {
                            map[src]$value = Level2_Map();
                        }
                        if ( dst !in map[src]$value)
                        {
                            map[src]$value[dst] = [$num=0];
                        }
                        map[src]$value[dst]$num = counttable[i];
                        #print fmt("Receiver: %s, count: %d", i, counttable[i]);
                    }
                }
            }

            if (levels >= 3)
            {
                if("modbus" in result)
                {
                    r = result["modbus"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }

                    counttable = r$counttable;
                    for ( i in counttable )
                    {
                        indexes = split_string(i, /;/);
                        src = to_addr(indexes[0]);
                        dst = to_addr(indexes[1]);
                        pro = indexes[2];
                        if ( ! map[src]$value[dst]?$value)
                        {
                            map[src]$value[dst]$value = Level3_Map();
                        }
                        if ( pro !in map[src]$value[dst]$value)
                        {
                            map[src]$value[dst]$value[pro] = [$num=0];
                        }
                        map[src]$value[dst]$value[pro]$num = counttable[i];
                    }
                }

                if("dnp3" in result)
                {
                    r = result["dnp3"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }

                    counttable = r$counttable;
                    for ( i in counttable )
                    {
                        indexes = split_string(i, /;/);
                        src = to_addr(indexes[0]);
                        dst = to_addr(indexes[1]);
                        pro = indexes[2];
                        if ( ! map[src]$value[dst]?$value)
                        {
                            map[src]$value[dst]$value = Level3_Map();
                        }
                        if ( pro !in map[src]$value[dst]$value)
                        {
                            map[src]$value[dst]$value[pro] = [$num=0];
                        }
                        map[src]$value[dst]$value[pro]$num = counttable[i];
                     }
                 }
             }

             if (levels >= 4)
             {
                if("modbus_fun" in result)
                {
                    r = result["modbus_fun"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }

                    counttable = r$counttable;
                    for ( i in counttable )
                    {
                        indexes = split_string(i, /;/);
                        src = to_addr(indexes[0]);
                        dst = to_addr(indexes[1]);
                        pro = indexes[2];
                        fun = indexes[3];
                        if ( ! map[src]$value[dst]$value[pro]?$value)
                        {
                            map[src]$value[dst]$value[pro]$value = Level4_Map();
                        }
                        if ( fun !in map[src]$value[dst]$value[pro]$value)
                        {
                            map[src]$value[dst]$value[pro]$value[fun] = [$num=0];
                        }
                        map[src]$value[dst]$value[pro]$value[fun]$num = counttable[i];
                    }
                }

                if("dnp3_fun" in result)
                {
                    r = result["dnp3_fun"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }

                    counttable = r$counttable;
                    for ( i in counttable )
                    {
                        indexes = split_string(i, /;/);
                        src = to_addr(indexes[0]);
                        dst = to_addr(indexes[1]);
                        pro = indexes[2];
                        fun = indexes[3];
                        if ( ! map[src]$value[dst]$value[pro]?$value)
                        {
                            map[src]$value[dst]$value[pro]$value = Level4_Map();
                        }
                        if ( fun !in map[src]$value[dst]$value[pro]$value)
                        {
                            map[src]$value[dst]$value[pro]$value[fun] = [$num=0];
                        }
                        map[src]$value[dst]$value[pro]$value[fun]$num = counttable[i];
                    }
                }
            }

            if (levels >= 5)
            {
                if("modbus_tgt" in result)
                {
                    r = result["modbus_tgt"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }

                    counttable = r$counttable;
                    for ( i in counttable )
                    {
                        indexes = split_string(i, /;/);
                        src = to_addr(indexes[0]);
                        dst = to_addr(indexes[1]);
                        pro = indexes[2];
                        fun = indexes[3];
                        tgt_string = indexes[4];
                        sub_tgts = split_string(tgt_string, / /);
                        v = range(1, |sub_tgts|);
                        ind = 0;
                        tgt = table();
                        for (j in v)
                        {
                            sub_tgt = split_string(sub_tgts[ind], /:/);
                            tgt[sub_tgt[0]] = to_count(sub_tgt[1]);
                            ++ind;
                        }
                        if ( ! map[src]$value[dst]$value[pro]$value[fun]?$value)
                        {
                            map[src]$value[dst]$value[pro]$value[fun]$value = Level5_Map();
                        }
                        map[src]$value[dst]$value[pro]$value[fun]$value[tgt_string] = [$tgt=tgt, $num=counttable[i]];
                    }
                }

                if("dnp3_tgt" in result)
                {
                    r = result["dnp3_tgt"];
                    # abort if we have no results
                    if ( ! r?$counttable )
                    {
                        return;
                    }

                    counttable = r$counttable;
                    for ( i in counttable )
                    {
                        indexes = split_string(i, /;/);
                        src = to_addr(indexes[0]);
                        dst = to_addr(indexes[1]);
                        pro = indexes[2];
                        fun = indexes[3];
                        tgt_string = indexes[4];
                        sub_tgts = split_string(tgt_string, / /);
                        v = range(1, |sub_tgts|);
                        ind = 0;
                        tgt = table();
                        for (j in v)
                        {
                            sub_tgt = split_string(sub_tgts[ind], /:/);
                            tgt[sub_tgt[0]] = to_count(sub_tgt[1]);
                            ++ind;
                        }
                        if ( ! map[src]$value[dst]$value[pro]$value[fun]?$value)
                        {
                            map[src]$value[dst]$value[pro]$value[fun]$value = Level5_Map();
                        }
                        map[src]$value[dst]$value[pro]$value[fun]$value[tgt_string] = [$tgt=tgt, $num=counttable[i]];
                   }
               }
           }

           log_stats();

           print fmt("Period: %s  Total: %d", g_ss$epoch, total);

           if(dynamic_interval)
           {
               update_interval(total);
           }
        }

# Update the sample_interval
function update_interval(total: count)
{
    if(sample_interval==sa1)
    {
        if(total<low_thre)
        {
           if(total*(sa2 / sa1) < low_thre)
           {
               sample_interval = sa3;
           }
           else
           {
               sample_interval = sa2;
           }
        }
    }

    if(sample_interval==sa2)
    {
        if(total<low_thre)
        {
           sample_interval = sa3;
        }
        if(total>up_thre)
        {
           sample_interval = sa1;
        }
    }

    if(sample_interval==sa3)
    {
        if(total>up_thre)
        {
           if(total/(sa3/sa2)>up_thre)
           {
               sample_interval = sa1;
           }
           else
           {
               sample_interval = sa2;
           }
        }
    }

    g_ss$epoch = sample_interval;
}

# Output the statistics
function print_stats()
{
    for ( i in map )
    {
        print fmt("Sender: %s, count: %d", i, map[i]$num);
        if(map[i]?$value)
        {
            for (j in map[i]$value)
            {
                print fmt("    Receiver: %s, count: %d", j, map[i]$value[j]$num);
                if(map[i]$value[j]?$value)
                {
                    for (k in map[i]$value[j]$value)
                    {
                        print fmt("        Protocol: %s, count: %d", k, map[i]$value[j]$value[k]$num);
                        if(map[i]$value[j]$value[k]?$value)
                        {
                            for (l in map[i]$value[j]$value[k]$value)
                            {
                                print fmt("            Function: %s, count: %d", l, map[i]$value[j]$value[k]$value[l]$num);
                                if(map[i]$value[j]$value[k]$value[l]?$value)
                                {
                                    for (m in map[i]$value[j]$value[k]$value[l]$value)
                                    {
                                        print fmt("                %s, count: %d", m, map[i]$value[j]$value[k]$value[l]$value[m]$num);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

# Log the statistics
function log_stats()
{
    local info: Info;
    local ts = network_time();

    for ( i in map )
    {
        info = Info($ts=ts, $period=sample_interval,
                    $sender=cat(i), $num=map[i]$num);
        Log::write(StatisticAnalyzer::LOG, info);
        if(map[i]?$value)
        {
            for (j in map[i]$value)
            {
                info = Info($ts=ts, $period=sample_interval,
                    $sender=cat(i), $receiver=cat(j), $num=map[i]$value[j]$num);
                Log::write(StatisticAnalyzer::LOG, info);
                if(map[i]$value[j]?$value)
                {
                    for (k in map[i]$value[j]$value)
                    {
                        info = Info($ts=ts, $period=sample_interval,
                            $sender=cat(i), $receiver=cat(j), $protocol=k,
                            $num=map[i]$value[j]$value[k]$num);
                        Log::write(StatisticAnalyzer::LOG, info);
                        if(map[i]$value[j]$value[k]?$value)
                        {
                            for (l in map[i]$value[j]$value[k]$value)
                            {
                                info = Info($ts=ts, $period=sample_interval,
                                    $sender=cat(i), $receiver=cat(j), $protocol=k, $func=l,
                                    $num=map[i]$value[j]$value[k]$value[l]$num);
                                Log::write(StatisticAnalyzer::LOG, info);
                                if(map[i]$value[j]$value[k]$value[l]?$value)
                                {
                                    for (m in map[i]$value[j]$value[k]$value[l]$value)
                                    {
                                        info = Info($ts=ts, $period=sample_interval,
                                            $sender=cat(i), $receiver=cat(j), $protocol=k, $func=l,
                                            $target=m, $num=map[i]$value[j]$value[k]$value[l]$value[m]$num);
                                        Log::write(StatisticAnalyzer::LOG, info);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

event bro_done()
{
}

# Count the senders and receivers
event new_packet(c: connection, p: pkt_hdr)
{
    if (levels >=1 )
    {
        if (p?$ip)
        {
            SumStats::observe("senders", [], [$str=cat(p$ip$src), $num=1]);
        }
        else
        {
            SumStats::observe("senders", [], [$str=cat(p$ip6$src), $num=1]);
        }
    }
    if (levels >=2)
    {
        if (p?$ip)
        {
            SumStats::observe("receivers", [], [$str=cat(p$ip$src,";",p$ip$dst), $num=1]);
        }
        else
        {
            SumStats::observe("receivers", [], [$str=cat(p$ip6$src,";",p$ip6$dst), $num=1]);
        }
    }

    if (p?$ip)
    {
        g_src = p$ip$src;
        g_dst = p$ip$dst;
    }
    else
    {
        g_src = p$ip6$src;
        g_dst = p$ip6$dst;
    }
}

@load base/bif/modbus_counter.bro
