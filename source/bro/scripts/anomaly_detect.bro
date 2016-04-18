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
##! Detect anomaly based on the statistics the StatsCount module provided.

@load stats_count.bro
@load base/utils/queue

module AnoDet;

export {
    redef enum Log::ID += { LOG };

    ## Enum type to represent various types of anomaly.
    type Ano_Type: enum {
        ## data field is out of normal range.
        DATA_FIELD,
        ## New kind of traffic.
        TRAFFIC_NEW,
        ## Old traffic disappear. 
        TRAFFIC_DIS,
        ## Unknown.
        UNKNOWN,
    };

    ## The record to store the information of each data field. 
    type Field_Info: record
    {
        ## A queue that contains the values in the k training periods.
        count_queue: Queue::Queue &default=Queue::init();
        ## Average value in the last k periods.
        count_avg: double &default=0;
        ## Standard deviation.
        std: double &default=0;
    };

    ## The record to store information of each node.
    type Count_Info: record
    {
        ## Count of the node.
        num: Field_Info;
        ## Total bytes of the node.
        bytes: Field_Info;
        ## Only active for the function-level: Ratio of responded requests.
        ratio_res: Field_Info &optional;
        ## Only active for the function-level: Average delay of the response.
        avg_delay: Field_Info &optional;
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

    ## Record used for the logging framework which contains the information of an anomaly.
    type Ano_Info: record {
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
        ## Field of anomaly
        field: string &log &optional;
        ## Anomaly description
        desc: Ano_Type &log;
        ## Anomaly score
        score: double &log &default=0;
    };

    ## The root of the normal tree.
    global normal_tree: Root_Node;

    ## The number of periods we based on to build the normal tree.
    global num_period = 5 &redef;

    ## Current period number.
    global current_period: count;

    ## The threshold of anomaly score to generate alarm
    global threshold_anomaly_score = 0.5 &redef;

    ## This event is generated for each new/disappeared node or each field
    global score_calculated: event(anomaly: Ano_Info); 

    ## This event is generated when an anomaly is detected.
    global anomaly_detected: event(anomaly: Ano_Info);

    global log_ano_det: event(rec: Ano_Info);

}

# A global vriable to store the anomaly information.
global g_ano_info: Ano_Info;

global ano_det_time: interval = 0sec;     # variable to store the anomaly detection time
global ano_det_count: count = 0;          # variable to store the number of anomaly detection

global cal_avg_std_node: function(normal_info: Count_Info);
global cal_avg_std: function(field: Field_Info);

global cal_anomaly_score: function(field: Field_Info, x: double): double;
global gen_ano_info: function(level: count): Ano_Info;

global initialize_node: function(normal_info: Count_Info, current_info: StatsCount::Count_Info, current_period: count);
global check_node: function(normal_info: Count_Info, current_info: StatsCount::Count_Info, level: count);
global node_disappear: function(normal_info: Count_Info, level: count);
global node_new: function(level: count);

global initialize_tree: function(current_tree: StatsCount::Root_Node, current_period: count);
global initialize_tree_level1: function(current_tree: StatsCount::Root_Node, normal_tree: Root_Node, current_period: count);
global initialize_tree_level2: function(l1_node_c: StatsCount::Level1_Node, l1_node: Level1_Node, current_period: count);
global initialize_tree_level3: function(l2_node_c: StatsCount::Level2_Node, l2_node: Level2_Node, current_period: count);
global initialize_tree_level4: function(l3_node_c: StatsCount::Level3_Node, l3_node: Level3_Node, current_period: count);
global initialize_tree_level5: function(l4_node_c: StatsCount::Level4_Node, l4_node: Level4_Node, current_period: count);
global initialize_tree_level6: function(l5_node_c: StatsCount::Level5_Node, l5_node: Level5_Node, current_period: count); 

global check_tree: function(current_tree: StatsCount::Root_Node);
global check_tree_level1: function(current_tree: StatsCount::Root_Node, normal_tree: Root_Node);
global check_tree_level2: function(l1_node_c: StatsCount::Level1_Node, l1_node: Level1_Node);
global check_tree_level3: function(l2_node_c: StatsCount::Level2_Node, l2_node: Level2_Node);
global check_tree_level4: function(l3_node_c: StatsCount::Level3_Node, l3_node: Level3_Node);
global check_tree_level5: function(l4_node_c: StatsCount::Level4_Node, l4_node: Level4_Node);
global check_tree_level6: function(l5_node_c: StatsCount::Level5_Node, l5_node: Level5_Node);

# Calculate the average and standard deviation value of data fields of a node in the normal tree
function cal_avg_std_node(normal_info: Count_Info)
{
    cal_avg_std(normal_info$num);
    cal_avg_std(normal_info$bytes);
    if(normal_info?$ratio_res)
    {
        cal_avg_std(normal_info$ratio_res);
    }
    if(normal_info?$avg_delay)
    {
        cal_avg_std(normal_info$avg_delay);
    } 
}

# Calculate the average and standard deviation of a vector
function cal_avg_std(field: Field_Info)
{
    local x: vector of double;
    Queue::get_vector(field$count_queue, x);
    field$count_avg = 0.0;
    for(i in x)
    {
        field$count_avg += x[i];
    }
    field$count_avg = field$count_avg/|x|;
    field$std = 0.0;
    for(i in x)
    {
        field$std += (x[i]-field$count_avg)*(x[i]-field$count_avg);
    }
    field$std = field$std/|x|;
    field$std = sqrt(field$std); 
}

# Calculate the anomaly score
function cal_anomaly_score(field: Field_Info, x: double): double
{
    local u = field$count_avg;
    local std = field$std;
    if(x >= u-std && x<= u+std)
    {
        return 0;
    }
    return 1-std*std/((x-u)*(x-u));
}

# Create a piece of anomaly information
function gen_ano_info(level: count): Ano_Info 
{
    local rst: Ano_Info;
    rst = [$ts=g_ano_info$ts, $period=g_ano_info$period, $desc=g_ano_info$desc, $score=g_ano_info$score];
    if(level >= 1)
    {
        rst$sender = g_ano_info$sender;
    }
    if(level >= 2)
    {
        rst$receiver = g_ano_info$receiver;
    }
    if(level >= 3)
    {
        rst$protocol = g_ano_info$protocol;
    }
    if(level >= 4)
    {
        rst$uid = g_ano_info$uid;
    }
    if(level >= 5)
    {
        rst$func = g_ano_info$func;
    }
    if(level >= 6)
    {
        rst$target = g_ano_info$target;
    }
    if(rst$desc == DATA_FIELD)
    {
        rst$field = g_ano_info$field;
    }
    return rst;
}

# Initialize all the data field of a node in the normal tree)
function initialize_node(normal_info: Count_Info, current_info: StatsCount::Count_Info, current_period: count)
{
    Queue::put(normal_info$num$count_queue, current_info$num+0.0);
    Queue::put(normal_info$bytes$count_queue, current_info$bytes+0.0);
    if(current_info?$num_res)
    {
        if(!normal_info?$ratio_res)
        {
            normal_info$ratio_res = [];
        }
        Queue::put(normal_info$ratio_res$count_queue, (current_info$num_res+0.0)/current_info$num);
    }
    
    if(current_info?$avg_delay)
    {
        if(!normal_info?$avg_delay)
        {
            normal_info$avg_delay = [];
        }
        Queue::put(normal_info$avg_delay$count_queue, interval_to_double(current_info$avg_delay));
    }

    if(current_period == num_period)
    {
        cal_avg_std_node(normal_info);
    }
} 

# Check the data fields of a node in the normal tree
function check_node(normal_info: Count_Info, current_info: StatsCount::Count_Info, level: count) 
{
    g_ano_info$desc = DATA_FIELD;
    g_ano_info$field = "number";
    g_ano_info$score = cal_anomaly_score(normal_info$num, current_info$num);
    event AnoDet::score_calculated(gen_ano_info(level));
    
    g_ano_info$field = "bytes";
    g_ano_info$score = cal_anomaly_score(normal_info$bytes, current_info$bytes);
    event AnoDet::score_calculated(gen_ano_info(level));

    if(current_info?$num_res && normal_info?$ratio_res)
    {
        g_ano_info$field = "response_ratio";
        g_ano_info$score = cal_anomaly_score(normal_info$ratio_res, current_info$num_res/current_info$num);
        event AnoDet::score_calculated(gen_ano_info(level));
    }

    if(current_info?$avg_delay && normal_info?$avg_delay)
    {
        g_ano_info$field = "average_response_delay";
        g_ano_info$score = cal_anomaly_score(normal_info$avg_delay, interval_to_double(current_info$avg_delay));
        event AnoDet::score_calculated(gen_ano_info(level));
    }
}

# Set the score of disappeared node 
function node_disappear(normal_info: Count_Info, level: count) 
{
    g_ano_info$desc = TRAFFIC_DIS;
    g_ano_info$score = cal_anomaly_score(normal_info$num, 0);
    event AnoDet::score_calculated(gen_ano_info(level));
}

# Set the score of new node 
function node_new(level: count) 
{
    g_ano_info$desc = TRAFFIC_NEW;
    g_ano_info$score = 1;
    event AnoDet::score_calculated(gen_ano_info(level));
}

# Function to initialize the normal tree
function initialize_tree(current_tree: StatsCount::Root_Node, current_period: count)
{
    initialize_node(normal_tree$info, current_tree$info, current_period);

    if(current_tree?$children)
    {
        initialize_tree_level1(current_tree, normal_tree, current_period);
    } 
}

# Function to initialize the level 1 of the tree
function initialize_tree_level1(current_tree: StatsCount::Root_Node, normal_tree: Root_Node, current_period: count)
{
    local l1_node: Level1_Node;
    local l1_node_c: StatsCount::Level1_Node;
    
    for ( i in current_tree$children )
    {
        g_ano_info$sender = i;
        if(!normal_tree?$children)
        {
            normal_tree$children = table();
        }
        if(i !in normal_tree$children)
        {
            normal_tree$children[i] = [$info=[$num=[], $bytes=[]]];
        }
        l1_node_c = current_tree$children[i];
        l1_node = normal_tree$children[i];
        initialize_node(l1_node$info, l1_node_c$info, current_period);
            
        if(l1_node_c?$children)
        {
            initialize_tree_level2(l1_node_c, l1_node, current_period); 
        }
    }
}

# Function to initialize the level 2 of the tree
function initialize_tree_level2(l1_node_c: StatsCount::Level1_Node, l1_node: Level1_Node, current_period: count)
{
    local l2_node: Level2_Node;
    local l2_node_c: StatsCount::Level2_Node;
    
    for (j in l1_node_c$children)
    {
        g_ano_info$receiver = j;
        if(!l1_node?$children)
        {
            l1_node$children = table();
        }
        if(j !in l1_node$children)
        {
            l1_node$children[j] = [$info=[$num=[], $bytes=[]]];
        }
        l2_node_c = l1_node_c$children[j];
        l2_node = l1_node$children[j];
        initialize_node(l2_node$info, l2_node_c$info, current_period);
                    
        if(l2_node_c?$children)
        {
            initialize_tree_level3(l2_node_c, l2_node, current_period);
        }
    }
}

# Function to initialize the level 3 of the tree
function initialize_tree_level3(l2_node_c: StatsCount::Level2_Node, l2_node: Level2_Node, current_period: count)
{
    local l3_node: Level3_Node;
    local l3_node_c: StatsCount::Level3_Node;

    for (k in l2_node_c$children)
    {
        g_ano_info$protocol = k;
        if(!l2_node?$children)
        {
            l2_node$children = table();
        }
        if(k !in l2_node$children)
        {
            l2_node$children[k] = [$info=[$num=[], $bytes=[]]];
        }
        l3_node_c = l2_node_c$children[k];
        l3_node = l2_node$children[k];
        initialize_node(l3_node$info, l3_node_c$info, current_period);
        
        if(l3_node_c?$children)
        {
            initialize_tree_level4(l3_node_c, l3_node, current_period);
        }
    }
}

# Function to initialize the level 4 of the tree
function initialize_tree_level4(l3_node_c: StatsCount::Level3_Node, l3_node: Level3_Node, current_period: count)
{
    local l4_node: Level4_Node;
    local l4_node_c: StatsCount::Level4_Node;
    
    for (l in l3_node_c$children)
    {
        g_ano_info$uid = l;
        if(!l3_node?$children)
        {
            l3_node$children = table();
        }
        if(l !in l3_node$children)
        {
            l3_node$children[l] = [$info=[$num=[], $bytes=[]]];
        }
        l4_node_c = l3_node_c$children[l];
        l4_node = l3_node$children[l];
        initialize_node(l4_node$info, l4_node_c$info, current_period);
        
        if(l4_node_c?$children)
        {
            initialize_tree_level5(l4_node_c, l4_node, current_period); 
        }
    }
}

# Function to initialize the level 5 of the tree
function initialize_tree_level5(l4_node_c: StatsCount::Level4_Node, l4_node: Level4_Node, current_period: count)
{
    local l5_node: Level5_Node;
    local l5_node_c: StatsCount::Level5_Node;
    
    for (m in l4_node_c$children)
    {
        g_ano_info$func = m;
        if(!l4_node?$children)
        {
            l4_node$children = table();
        }
        if(m !in l4_node$children)
        {
            l4_node$children[m] = [$info=[$num=[], $bytes=[]]];
        }
        l5_node_c = l4_node_c$children[m];
        l5_node = l4_node$children[m];
        initialize_node(l5_node$info, l5_node_c$info, current_period);
        
        if(l5_node_c?$children)
        {
            initialize_tree_level6(l5_node_c, l5_node, current_period); 
        }
    }
}


# Function to initialize the level 6 of the tree
function initialize_tree_level6(l5_node_c: StatsCount::Level5_Node, l5_node: Level5_Node, current_period: count)
{
    for (n in l5_node_c$children)
    {
        g_ano_info$target = n;
        if(!l5_node?$children)
        {
            l5_node$children = table();
        }
        if(n !in l5_node$children)
        {
            l5_node$children[n] = [$num=[], $bytes=[]];
        }
        initialize_node(l5_node$children[n], l5_node_c$children[n], current_period);
    }
}

# Function to check the current tree using the normal tree
function check_tree(current_tree: StatsCount::Root_Node)
{
    check_node(normal_tree$info, current_tree$info, 0);
    check_tree_level1(current_tree, normal_tree);
}

# Function to check the level 1 of the tree
function check_tree_level1(current_tree: StatsCount::Root_Node, normal_tree: Root_Node)
{
    local l1_node: Level1_Node;
    local l1_node_c: StatsCount::Level1_Node;

    if(current_tree?$children)
    {
        for(i in current_tree$children)
        {
            g_ano_info$sender = i;
            if(!normal_tree?$children || i !in normal_tree$children)
            {
                node_new(1);
            }
            else
            {
                l1_node_c = current_tree$children[i];
                l1_node = normal_tree$children[i];
                check_node(l1_node$info, l1_node_c$info, 1);
                check_tree_level2(l1_node_c, l1_node);
            }
        }
    }
    
    if(normal_tree?$children)
    {
        for(i in normal_tree$children)
        {
            g_ano_info$sender = i;
            if(!current_tree?$children || i !in current_tree$children)
            {
                node_disappear(normal_tree$children[i]$info, 1); 
            }
        }
    }
}

# Function to check the level 2 of the tree
function check_tree_level2(l1_node_c: StatsCount::Level1_Node, l1_node: Level1_Node)
{
    local l2_node: Level2_Node;
    local l2_node_c: StatsCount::Level2_Node;

    if(l1_node_c?$children)
    {
        for(j in l1_node_c$children)
        {
            g_ano_info$receiver = j;
            if(!l1_node?$children || j !in l1_node$children)
            {
                node_new(2);
            }
            else
            {
                l2_node_c = l1_node_c$children[j];
                l2_node = l1_node$children[j];
                check_node(l2_node$info, l2_node_c$info, 2);
                check_tree_level3(l2_node_c, l2_node);
            }
        }
    }
    
    if(l1_node?$children)
    {
        for(j in l1_node$children)
        {
            g_ano_info$receiver = j;
            if(!l1_node_c?$children || j !in l1_node_c$children)
            {
                node_disappear(l1_node$children[j]$info, 2); 
            }
        }
    }
}

# Function to check the level 3 of the tree
function check_tree_level3(l2_node_c: StatsCount::Level2_Node, l2_node: Level2_Node)
{
    local l3_node: Level3_Node;
    local l3_node_c: StatsCount::Level3_Node;

    if(l2_node_c?$children)
    {
        for(k in l2_node_c$children)
        {
            g_ano_info$protocol = k;
            if(!l2_node?$children || k !in l2_node$children)
            {
                node_new(3);
            }
            else
            {
                l3_node_c = l2_node_c$children[k];
                l3_node = l2_node$children[k];
                check_node(l3_node$info, l3_node_c$info, 3);
                check_tree_level4(l3_node_c, l3_node);
            }
        }
    }
    
    if(l2_node?$children)
    {
        for(k in l2_node$children)
        {
            g_ano_info$protocol = k;
            if(!l2_node_c?$children || k !in l2_node_c$children)
            {
                node_disappear(l2_node$children[k]$info, 3); 
            }
        }
    }
}

# Function to check the level 4 of the tree
function check_tree_level4(l3_node_c: StatsCount::Level3_Node, l3_node: Level3_Node)
{
    local l4_node: Level4_Node;
    local l4_node_c: StatsCount::Level4_Node;

    if(l3_node_c?$children)
    {
        for(l in l3_node_c$children)
        {
            g_ano_info$uid = l;
            if(!l3_node?$children || l !in l3_node$children)
            {
                node_new(4);
            }
            else
            {
                l4_node_c = l3_node_c$children[l];
                l4_node = l3_node$children[l];
                check_node(l4_node$info, l4_node_c$info, 4);
                check_tree_level5(l4_node_c, l4_node);
            }
        }
    }
    
    if(l3_node?$children)
    {
        for(l in l3_node$children)
        {
            g_ano_info$uid = l;
            if(!l3_node_c?$children || l !in l3_node_c$children)
            {
                node_disappear(l3_node$children[l]$info, 4); 
            }
        }
    }
}

# Function to check the level 5 of the tree
function check_tree_level5(l4_node_c: StatsCount::Level4_Node, l4_node: Level4_Node)
{
    local l5_node: Level5_Node;
    local l5_node_c: StatsCount::Level5_Node;

    if(l4_node_c?$children)
    {
        for(m in l4_node_c$children)
        {
            g_ano_info$func = m;
            if(!l4_node?$children || m !in l4_node$children)
            {
                node_new(5);
            }
            else
            {
                l5_node_c = l4_node_c$children[m];
                l5_node = l4_node$children[m];
                check_node(l5_node$info, l5_node_c$info, 5);
                check_tree_level6(l5_node_c, l5_node);
            }
        }
    }
    
    if(l4_node?$children)
    {
        for(m in l4_node$children)
        {
            g_ano_info$func = m;
            if(!l4_node_c?$children || m !in l4_node_c$children)
            {
                node_disappear(l4_node$children[m]$info, 5); 
            }
        }
    }
}

# Function to check the level 6 of the tree
function check_tree_level6(l5_node_c: StatsCount::Level5_Node, l5_node: Level5_Node)
{
    if(l5_node_c?$children)
    {
        for (n in l5_node_c$children)
        {
            g_ano_info$target = n;
            if(!l5_node?$children || n !in l5_node$children)
            {
                node_new(6);
            }
            else
            {
                check_node(l5_node$children[n], l5_node_c$children[n], 6);
            }
        }
    }

    if(l5_node?$children)
    {
        for(n in l5_node$children)
        {
            g_ano_info$target = n;
            if(!l5_node_c?$children || n !in l5_node_c$children)
            {
                node_disappear(l5_node$children[n], 6); 
            }
        }
    }
}

event bro_init() &priority=5
{
    Log::create_stream(LOG, [$columns=Ano_Info, $ev=log_ano_det]);
}

event bro_init()
{
    normal_tree = [$info=[$num=[], $bytes=[]]];
    current_period = 0;
}

event bro_done()
{
    ano_det_time = ano_det_time/ano_det_count;
    if (StatsCol::measure)
    {
        print fmt("anomaly detection time: %s", ano_det_time);
    }
}

# Log the data structure and update the interval after the process.
event StatsCount::process_finish(result: StatsCount::CountStat)
{
    local ano_det_start = current_time();

    current_period += 1;
    g_ano_info = [$ts=result$ts, $period=result$period, $desc=UNKNOWN];
    if(current_period <= num_period)
    {
        initialize_tree(result$tree, current_period);
    }
    else
    {
        check_tree(result$tree);
    }

    local ano_det_end = current_time();
    if(current_period > num_period)
    {
        ano_det_time += ano_det_end - ano_det_start;
        ano_det_count += 1;
    }
}

event score_calculated(anomaly: Ano_Info)
{
    if(anomaly$score >= threshold_anomaly_score)
    {
        event AnoDet::anomaly_detected(anomaly);
    }
}

event anomaly_detected(anomaly: Ano_Info)
{
    Log::write(AnoDet::LOG, anomaly);
}

