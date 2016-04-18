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
##! Count number of senders, receivers, protocols, functions and targets in traffic and build a tree structure to hold the statistics.

@load base/frameworks/sumstats
@load base/bif/sumstats_count.bro
@load base/bif/sumstats_keyed_avg.bro
@load base/bif/stats_col.bro

module StatsCount;

export {
    redef enum Log::ID += { LOG };

    type Count_Info: record
    {
        ## Count of the node.
        num: count;
	    ## Total bytes of the node.
	    bytes: count;
        ## Only active for the function-level: Whether the function is a request.
        is_request: bool &optional;
        ## Only active for the function-level: Num of the responded requests.
        num_res: count &optional;
        ## Only active for the function-level: Average delay of the response.
        avg_delay: interval &optional;
        ## Only active for the target-level: Name and the index of the target.
        tgt: table[string] of string &optional;
    };

    ## The record corresponding to a function-level node in the tree.
    type Level5_Node: record {
        ## The table contains the subtree.
        children: table[string] of Count_Info &optional;
        ## Count info.
        info: Count_Info;
    };
    
    ## The record corresponding to a uid-level node in the tree.
    type Level4_Node: record {
        ## The table contains the subtree.
        children: table[string] of Level5_Node &optional;
        ## Count info.
        info: Count_Info;
    };

    ## The record corresponding to a protocol-level node in the tree.
    type Level3_Node: record {
        ## The table contains the subtree.
        children: table[string] of Level4_Node &optional;
        ## Count info.
        info: Count_Info;
    };

    ## The record corresponding to a receiver-level node in the tree.
    type Level2_Node: record {
        ## The table contains the subtree.
        children: table[string] of Level3_Node &optional;
        ## Count info.
        info: Count_Info;
    };

    ## The record corresponding to a sender-level node in the tree.
    type Level1_Node: record {
        ## The table contains the subtree.
        children: table[string] of Level2_Node &optional;
        ## Count info.
        info: Count_Info;
    };

    ## The record corresponding to a sender-level node in the tree.
    type Root_Node: record {
        ## The table contains the subtree.
        children: table[string] of Level1_Node &optional;
        ## Count info.
        info: Count_Info;
    };

    ## The root of the normal tree.
    global current_tree: Root_Node;

    ## The number of level needed.
    global levels = 6 &redef;

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

    ## Record used for the logging framework which contains the information of a node in the tree sturcture.
    type Info: record {
        ## Timestamp when the data is written.
        ts: time &log;
        ## Length of the last period
        period: interval &log;
        ## Sender's IP
        sender: string &log &optional;
        ## Receiver's IP
        receiver: string &log &optional;
        ## Protocol name
        protocol: string &log &optional;
        ## Unit ID
        uid: string &log &optional;
        ## Function name
        func: string &log &optional;
        ## Target name
        target: string &log &optional;
        ## Number of instances seen in the period
        num: count &log &default=0;
	    ## Total length of instances in bytes seen in the period
	    bytes: count &log &default=0;
        ## Whether this is a request
        is_request: bool &log &optional;
        ## Number of responded requests
        num_response: count &log &optional;
        ## Average of the response delay if this is request
        avg_delay: interval &log &optional;
    };

    ## Record containing the information of the complete period.
    type CountStat: record {
        ## Timestamp when the data is written.
        ts: time;
        ## Length of the last period
        period: interval;
        ## The tree structure which contains all the counts
        tree: Root_Node;
    };

    global log_stats_count: event(rec: Info);

    ## This is event is generated at the end of each period after the data construction is finished.
    global process_finish: event(result: CountStat);
}

# global variable to store the Sumstat
global g_ss: SumStats::SumStat;

# global variable to store the number of total packets in this period
global g_total = 0;

# Log the statistics for the previous period
global log_stats: function();

# Update the interval
global update_interval: function(total: count);

#Process the result for each epoch
global process_result: function (ts: time, key: SumStats::Key, result: SumStats::Result);

global after_process: function (ts: time);

# The in memory data structure for holding a piece of request information.
type req_info: record {
    requestor: addr;
    responder: addr;
    protocol: string;
    uid: string;
    req_func: string;
    req_time: time;
    is_responded: bool &default=F;
};

# An in memory table for holding request information related to different connections.
global req_table: table[string] of req_info;

global item_time: interval = 0sec;    # variable to store the item process time
global item_count: count = 0;         # variable to store the number of items
global aggre_start: time;             # variable to store the start time of the aggregation
global aggre_end: time;               # variable to store the end time of the aggregation
global aggre_time: interval = 0sec;   # variable to store the aggregation time
global aggre_count: count = 0;        # variable to store the number of aggregation


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

event bro_init() &priority=5
{
    Log::create_stream(LOG, [$columns=Info, $ev=log_stats_count]);
}

# Declare the reducers
event bro_init()
{
    local r0c = SumStats::Reducer($stream="total_count", $apply=set(SumStats::SUM));
    local r0b = SumStats::Reducer($stream="total_byte", $apply=set(SumStats::SUM));
    local r1c = SumStats::Reducer($stream="senders_count", $apply=set(SumStats::COUNTTABLE));
    local r1b = SumStats::Reducer($stream="senders_byte", $apply=set(SumStats::COUNTTABLE));
    local r2c = SumStats::Reducer($stream="receivers_count", $apply=set(SumStats::COUNTTABLE));
    local r2b = SumStats::Reducer($stream="receivers_byte", $apply=set(SumStats::COUNTTABLE));
    local r3c = SumStats::Reducer($stream="protocols_count", $apply=set(SumStats::COUNTTABLE));
    local r3b = SumStats::Reducer($stream="protocols_byte", $apply=set(SumStats::COUNTTABLE));
    local r4c = SumStats::Reducer($stream="uid_count", $apply=set(SumStats::COUNTTABLE));
    local r4b = SumStats::Reducer($stream="uid_byte", $apply=set(SumStats::COUNTTABLE));
    local r5c = SumStats::Reducer($stream="functions_count", $apply=set(SumStats::COUNTTABLE));
    local r5b = SumStats::Reducer($stream="functions_byte", $apply=set(SumStats::COUNTTABLE));
    local r5_res = SumStats::Reducer($stream="functions_res", $apply=set(SumStats::KEYED_AVERAGE));
    local r6c = SumStats::Reducer($stream="targets_count", $apply=set(SumStats::COUNTTABLE));
    local r6b = SumStats::Reducer($stream="targets_byte", $apply=set(SumStats::COUNTTABLE));

    g_ss = [$name="counters", $epoch=sample_interval, $reducers=set(r0c,r0b,r1c,r1b,r2c,r2b,r3c,r3b,r4c,r4b,r5c,r5b,r5_res,r6c,r6b),
        $epoch_result=process_result, $epoch_finished=after_process];

    SumStats::create(g_ss);
    
    if (levels > StatsCol::col_levels)
    {
        levels = StatsCol::col_levels;
    }
}

event bro_done()
{
    if (StatsCol::measure)
    {
      item_time = item_time/item_count;
      aggre_time = aggre_time/aggre_count;
      print fmt("item time: %s", item_time);
      print fmt("aggregation time: %s", aggre_time);
    }
}

event process_finish(result: CountStat)
{
    
}

# Log the data structure and update the interval after the process.
function after_process(ts: time)
{
    event StatsCount::process_finish([$ts=network_time(),$period=g_ss$epoch,$tree=current_tree]);
    #print fmt("Period: %s  Total: %d", g_ss$epoch, current_tree$info$num);
    
    log_stats();
    #print_stats();
    
    if(dynamic_interval)
    {
        update_interval(current_tree$info$num);
    }

    current_tree = [$info=[$num=0,$bytes=0]];

    if (StatsCol::measure)
    {
        aggre_end = current_time();
        aggre_time += aggre_end - aggre_start;
        aggre_count += 1;
    }
}

#Process the result for each epoch
function process_result(ts: time, key: SumStats::Key, result: SumStats::Result)
{
    if (StatsCol::measure)
    {
        aggre_start = current_time();
    }

    local r1: SumStats::ResultVal;
    local r2: SumStats::ResultVal;
    local l1_node: Level1_Node;
    local l2_node: Level2_Node;
    local l3_node: Level3_Node;
    local l4_node: Level4_Node;
    local l5_node: Level5_Node;
    local count_table: table[string] of count;
    local byte_table: table[string] of count;
    local avg_table: table[string] of SumStats::keyed_avg_value;
    local src: string;
    local dst: string;
    local indexes: string_vec;
    local pro: string;
    local uid: string;
    local fun: string;
    local fun_info: string_vec;
    local tgt_string: string;
    local sub_tgts: string_vec;
    local v: vector of count;
    local ind: count;
    local tgt: table[string] of string;
    local sub_tgt: string_vec;
    
    if("total_count" in result)
    {
        r1 = result["total_count"];
        r2 = result["total_byte"];
        current_tree$info$num = double_to_count(r1$sum);
        current_tree$info$bytes = double_to_count(r2$sum);
    }

    if(levels >= 1 && "senders_count" in result)
    {
        r1 = result["senders_count"];
        r2 = result["senders_byte"];
        # abort if we have no results
        if(!r1?$counttable)
        {
            return;
        }
        count_table = r1$counttable;
        byte_table = r2$counttable;

        for(i in count_table)
        {
            src = i;
            if(! current_tree?$children)
            {
                current_tree$children = table();
            }
            if(src !in current_tree$children)
            {
                current_tree$children[src] = [$info=[$num=0,$bytes=0]];
            }
            current_tree$children[src]$info$num = count_table[i];
            current_tree$children[src]$info$bytes = byte_table[i];
        }
    }

    if(levels >= 2 && "receivers_count" in result)
    {
        r1 = result["receivers_count"];
        r2 = result["receivers_byte"];
        # abort if we have no results
        if(!r1?$counttable )
        {
            return;
        }

        count_table = r1$counttable;
        byte_table = r2$counttable;
        for(i in count_table)
        {
            indexes = split_string(i, /;/);
            src = indexes[0];
            dst = indexes[1];
            l1_node = current_tree$children[src];
            if(! l1_node?$children)
            {
                l1_node$children = table();
            }
            if(dst !in l1_node$children)
            {
                l1_node$children[dst] = [$info=[$num=0,$bytes=0]];
            }
            l1_node$children[dst]$info$num = count_table[i];
            l1_node$children[dst]$info$bytes = byte_table[i];
        }
    }

    if(levels >= 3 && "protocols_count" in result)
    {
        r1 = result["protocols_count"];
        r2 = result["protocols_byte"];
        # abort if we have no results
        if (!r1?$counttable )
        {
            return;
        }

        count_table = r1$counttable;
        byte_table = r2$counttable;
        for(i in count_table)
        {
            indexes = split_string(i, /;/);
            src = indexes[0];
            dst = indexes[1];
            pro = indexes[2];
            l2_node = current_tree$children[src]$children[dst];
            if(! l2_node?$children)
            {
                l2_node$children = table();
            }
            if(pro !in l2_node$children)
            {
                l2_node$children[pro] = [$info=[$num=0,$bytes=0]];
            }
            l2_node$children[pro]$info$num = count_table[i];
            l2_node$children[pro]$info$bytes = byte_table[i];
        }
    }

    if(levels >= 4 && "uid_count" in result)
    {
        r1 = result["uid_count"];
        r2 = result["uid_byte"];
        # abort if we have no results
        if (!r1?$counttable )
        {
            return;
        }

        count_table = r1$counttable;
        byte_table = r2$counttable;
        for(i in count_table)
        {
            indexes = split_string(i, /;/);
            src = indexes[0];
            dst = indexes[1];
            pro = indexes[2];
            uid = indexes[3];
            l3_node = current_tree$children[src]$children[dst]$children[pro];
            if(! l3_node?$children)
            {
                l3_node$children = table();
            }
            if(uid !in l3_node$children)
            {
                l3_node$children[uid] = [$info=[$num=0,$bytes=0]];
            }
            l3_node$children[uid]$info$num = count_table[i];
            l3_node$children[uid]$info$bytes = byte_table[i];
        }
    }

    if(levels >= 5)
    {
        if("functions_count" in result)
        {
            r1 = result["functions_count"];
            r2 = result["functions_byte"];
            # abort if we have no results
            if (!r1?$counttable )
            {
                return;
            }

            count_table = r1$counttable;
            byte_table = r2$counttable;
            for (i in count_table )
            {
                indexes = split_string(i, /;/);
                src = indexes[0];
                dst = indexes[1];
                pro = indexes[2];
                uid = indexes[3];
                fun = indexes[4];
                fun_info = split_string(fun, /:/);
                l4_node = current_tree$children[src]$children[dst]$children[pro]$children[uid];
                if(! l4_node?$children)
                {
                    l4_node$children = table();
                }
                if(fun_info[0] !in l4_node$children)
                {
                    l4_node$children[fun_info[0]] = [$info=[$num=0,$bytes=0]];
                }
                l4_node$children[fun_info[0]]$info$num = count_table[i];
                l4_node$children[fun_info[0]]$info$bytes = byte_table[i];
                if(fun_info[1] == "T")
                {
                    l4_node$children[fun_info[0]]$info$is_request = T;
                    l4_node$children[fun_info[0]]$info$num_res = 0;
                }
                if(fun_info[1] == "F")
                {
                    l4_node$children[fun_info[0]]$info$is_request = F;
                }
            }
        }

        if("functions_res" in result)
        {
            r1 = result["functions_res"];
            # abort if we have no results
            if ( ! r1?$avg_table )
            {
                return;
            }

            avg_table = r1$avg_table;
            for ( i in avg_table )
            {
                indexes = split_string(i, /;/);
                src = indexes[0];
                dst = indexes[1];
                pro = indexes[2];
                uid = indexes[3];
                fun = indexes[4];
                if(current_tree?$children && src in current_tree$children && current_tree$children[src]?$children &&
                    dst in current_tree$children[src]$children && current_tree$children[src]$children[dst]?$children &&
                    pro in current_tree$children[src]$children[dst]$children &&
                    current_tree$children[src]$children[dst]$children[pro]?$children &&
                    uid in current_tree$children[src]$children[dst]$children[pro]$children &&
                    current_tree$children[src]$children[dst]$children[pro]$children[uid]?$children &&
                    fun in current_tree$children[src]$children[dst]$children[pro]$children[uid]$children)
                {
                    l4_node = current_tree$children[src]$children[dst]$children[pro]$children[uid];
                    l4_node$children[fun]$info$num_res = avg_table[i]$num;
                    l4_node$children[fun]$info$avg_delay = double_to_interval(avg_table[i]$avg);
                }
            }
        }
    }

    if(levels >= 6 && "targets_count" in result)
    {
        r1 = result["targets_count"];
        r2 = result["targets_byte"];
        # abort if we have no results
        if(!r1?$counttable)
        {
            return;
        }

        count_table = r1$counttable;
        byte_table = r2$counttable;
        for(i in count_table)
        {
            indexes = split_string(i, /;/);
            src = indexes[0];
            dst = indexes[1];
            pro = indexes[2];
            uid = indexes[3];
            fun = indexes[4];
            tgt_string = indexes[5];
            sub_tgts = split_string(tgt_string, / /);
            v = range(1, |sub_tgts|);
            ind = 0;
            tgt = table();
            for(j in v)
            {
                sub_tgt = split_string(sub_tgts[ind], /:/);
                tgt[sub_tgt[0]] = sub_tgt[1];
                ++ind;
            }
            l5_node = current_tree$children[src]$children[dst]$children[pro]$children[uid]$children[fun];
            if(! l5_node?$children)
            {
                l5_node$children = table();
            }
            if(tgt_string !in l5_node$children)
            {
                l5_node$children[tgt_string] = [$num=0,$bytes=0];
            }
            l5_node$children[tgt_string]$num = count_table[i];
            l5_node$children[tgt_string]$bytes = byte_table[i];
            l5_node$children[tgt_string]$tgt = tgt;
        }
    }
}

# Update the sample_interval
function update_interval(total: count)
{
    if(sample_interval==sa1)
    {
        if(total<low_thre)
        {
            if(total*(sa2/sa1)<low_thre)
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
    local l1_node: Level1_Node;
    local l2_node: Level2_Node;
    local l3_node: Level3_Node;
    local l4_node: Level4_Node;
    local l5_node: Level5_Node;
    print fmt("Total: %d, %d", current_tree$info$num, current_tree$info$bytes);

    if(current_tree?$children)
    {
        for ( i in current_tree$children )
        {
            l1_node = current_tree$children[i];
            print fmt("Sender: %s, count: %d, bytes: %d", i, l1_node$info$num, l1_node$info$bytes);
            if(l1_node?$children)
            {
                for (j in l1_node$children)
                {
                    l2_node = l1_node$children[j];
                    print fmt("    Receiver: %s, count: %d, bytes: %d", j, l2_node$info$num, l2_node$info$bytes);
                    if(l2_node?$children)
                    {
                        for (k in l2_node$children)
                        {
                            l3_node = l2_node$children[k];
                            print fmt("        Protocol: %s, count: %d, bytes: %d", k, l3_node$info$num, l3_node$info$bytes);
                            if(l3_node?$children)
                            {
                                for (l in l3_node$children)
                                {
                                    l4_node = l3_node$children[l];
                                    print fmt("           Unit ID: %s, count: %d, bytes: %d", l, l4_node$info$num, l4_node$info$bytes);
                                    if(l4_node?$children)
                                    {
                                        for(m in l4_node$children)
                                        {
                                            l5_node = l4_node$children[m];
                                            if(l5_node$info?$is_request)
                                            {
                                                if(l5_node$info$is_request==T && l5_node$info?$num_res)
                                                {
                                                    print fmt("                Function: %s (request), count: %d, bytes: %d, num_res: %d, avg_delay: %s",
                                                              m, l5_node$info$num, l5_node$info$bytes, l5_node$info$num_res, l5_node$info$avg_delay);
                                                }
                                                else
                                                {
                                                    print fmt("                Function: %s (response), count: %d, bytes: %d",m, l5_node$info$num, l5_node$info$bytes);
                                                }
                                            }
                                            else
                                            {
                                                print fmt("                Function: %s, count: %d, bytes: %d", m, l5_node$info$num, l5_node$info$bytes);
                                            }
                                            if(l5_node?$children)
                                            {
                                                for (n in l5_node$children)
                                                {
                                                    print fmt("                    %s, count: %d, bytes: %d", n, l5_node$children[n]$num, l5_node$children[n]$bytes);
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
        }
    }
}

# Log the statistics
function log_stats()
{
    local info: Info;
    local ts = network_time();
    local l1_node: Level1_Node;
    local l2_node: Level2_Node;
    local l3_node: Level3_Node;
    local l4_node: Level4_Node;
    local l5_node: Level5_Node;
    info = Info($ts=ts, $period=sample_interval, $num=current_tree$info$num, $bytes=current_tree$info$bytes);
    Log::write(StatsCount::LOG, info);

    if(current_tree?$children)
    {
        for ( i in current_tree$children )
        {
            l1_node = current_tree$children[i];
            info = Info($ts=ts, $period=sample_interval, $sender=i, $num=l1_node$info$num, $bytes=l1_node$info$bytes);
            Log::write(StatsCount::LOG, info);
            if(l1_node?$children)
            {
                for (j in l1_node$children)
                {
                    l2_node = l1_node$children[j];
                    info = Info($ts=ts, $period=sample_interval, $sender=i, $receiver=j, 
                                $num=l2_node$info$num, $bytes=l2_node$info$bytes);
                    Log::write(StatsCount::LOG, info);
                    if(l2_node?$children)
                    {
                        for (k in l2_node$children)
                        {
                            l3_node = l2_node$children[k];
                            info = Info($ts=ts, $period=sample_interval, $sender=i, $receiver=j, 
                                        $protocol=k, $num=l3_node$info$num, $bytes=l3_node$info$bytes);
                            Log::write(StatsCount::LOG, info);
                            if(l3_node?$children)
                            {
                                for (l in l3_node$children)
                                {
                                    l4_node = l3_node$children[l];
                                    info = Info($ts=ts, $period=sample_interval, $sender=i, $receiver=j, $protocol=k, 
                                                $uid=l, $num=l4_node$info$num, $bytes=l4_node$info$bytes);
                                    Log::write(StatsCount::LOG, info);
                                    if(l4_node?$children)
                                    {
                                        for (m in l4_node$children)
                                        {
                                            l5_node = l4_node$children[m];
                                            info = Info($ts=ts, $period=sample_interval, $sender=i, $receiver=j, $protocol=k, 
                                                        $uid=l, $func=m, $num=l5_node$info$num, $bytes=l5_node$info$bytes);
                                            if(l5_node$info?$is_request)
                                            {
                                                info$is_request = l5_node$info$is_request;
                                                if(l5_node$info$is_request==T && l5_node$info?$num_res)
                                                {
                                                    info$num_response = l5_node$info$num_res;
                                                    if(l5_node$info?$avg_delay)
                                                    {
                                                        info$avg_delay = l5_node$info$avg_delay;
                                                    }
                                                }
                                            }
                                            Log::write(StatsCount::LOG, info);
                                            if(l5_node?$children)
                                            {
                                                for (n in l5_node$children)
                                                {
                                                    info = Info($ts=ts, $period=sample_interval, $sender=i, $receiver=j, $protocol=k,
                                                                $uid=l, $func=m, $target=n, $num=l5_node$children[n]$num, 
                                                                $bytes=l5_node$children[n]$bytes);
                                                    Log::write(StatsCount::LOG, info);
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
        }
    }
}

# Update the counter
event StatsCol::item_seen(item: StatsCol::Info)
{
    if (StatsCol::measure)
    {
        local item_start = current_time();
    }
    
    if(!item?$protocol)
    {
        if (StatsCol::measure)
        {
            item_count += 1;
        }
        SumStats::observe("total_count", [], [$num=1]);
        SumStats::observe("total_byte", [], [$num=item$len]);
        SumStats::observe("senders_count", [], [$str=cat(item$sender)]);
        SumStats::observe("senders_byte", [], [$str=cat(item$sender),$num=item$len]);
        if (levels >= 2)
        {
            SumStats::observe("receivers_count", [], [$str=cat(item$sender,";",item$receiver)]);
            SumStats::observe("receivers_byte", [], [$str=cat(item$sender,";",item$receiver),$num=item$len]);
        }
    }
    else if(!item?$target)
    {
        SumStats::observe("protocols_count", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol)]);
        SumStats::observe("protocols_byte", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol),$num=item$len]);
         
        if (levels >= 4)
        {
            SumStats::observe("uid_count", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid)]);
            SumStats::observe("uid_byte", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid),$num=item$len]);
        }
        
        if (levels >= 5)
        {
            local index = item$conn$uid;
            if(!item?$is_response)
            {
                SumStats::observe("functions_count", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,":-")]);
                SumStats::observe("functions_byte", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,":-"),$num=item$len]);
            }
            if(item$is_response == F)
            {
                SumStats::observe("functions_count", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,":T")]);
                SumStats::observe("functions_byte", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,":T"),$num=item$len]);
                req_table[index] = [$requestor=item$sender,$responder=item$receiver, $protocol=item$protocol, $uid=item$uid, $req_func=item$func, $req_time=item$ts];
            }
            if(item$is_response == T)
            {
                SumStats::observe("functions_count", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,":F")]);
                SumStats::observe("functions_byte", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,":F"),$num=item$len]);
                local uid = item$uid;
                local uids = split_string(uid, /:/);
                if(|uids| == 2)
                {
                   uid = cat(uids[1], ":", uids[0]); 
                }
                if(index in req_table && req_table[index]$requestor==item$receiver && req_table[index]$responder==item$sender
                    && req_table[index]$protocol==item$protocol && req_table[index]$uid==uid && req_table[index]$is_responded==F)
                {
                    local delay = time_to_double(item$ts)-time_to_double(req_table[index]$req_time);
                    req_table[index]$is_responded = T;
                    SumStats::observe("functions_res", [], [$str=cat(item$receiver,";",item$sender,";",item$protocol,";",uid,";",req_table[index]$req_func), $dbl=delay]);
                }
            }
        }
    }
    else
    {
        SumStats::observe("targets_count", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,";",item$target)]);
        SumStats::observe("targets_byte", [], [$str=cat(item$sender,";",item$receiver,";",item$protocol,";",item$uid,";",item$func,";",item$target),$num=item$len]);
    }

    if (StatsCol::measure)
    {
        local item_end = current_time();
        item_time += item_end - item_start;
    }
}
