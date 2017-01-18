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
##! Recognize identities of units in the network based on predefined rules in the input file

@load base/frameworks/intel
@load frameworks/intel/seen
@load frameworks/intel/do_notice
@load base/utils/addrs
@load base/bif/stats_col.bro

module IdRecog;

# Read the predefined recognition rules
redef Intel::read_files += {
    fmt("%s/id_recog.dat", @DIR)
};

export {
    redef enum Log::ID += { LOG };

    ## Add another Intel::Where member
    redef enum Intel::Where += {
        Iden::IN_PROTOCOL_HEADER
    };

    ## Add another type of intelligence data to represent 
    ## the function pair in the request and reponse.
    redef enum Intel::Type += {
        Intel::FUNCTION_PAIR
    };

    ## Record used for the logging framework representing a recognition
    ## of the unit identity.
    type Info: record {
        ## Time the identity is first recognized
        ts: time &log;
        ## IP of the unit
        ip: addr &log;
        ## Identity of the unit
        identity: string &log;
    };

    ## A table used for holding all the recognitions of the identities.
    global identity_map: table[addr] of set[string];

    global log_identify: event(rec: Info);

    ## A function to output the identities recognized.
    global print_identity: function();
}

# The in memory data structure for holding a piece of request information.
type req_info: record {
    requestor: addr;
    responder: addr;
    protocol: string;
    req_func: string;
};

# An in memory table for holding request information related to different connections.
global req_table: table[string] of req_info;

global id_recog_time: interval = 0sec;    # variable to store the identity recognition time
global id_recog_count: count = 0;         # variable to store the number of identity recognition

event bro_init() &priority=5
{
    Log::create_stream(LOG, [$columns=Info, $ev=log_identify]);
}

event bro_done()
{
    id_recog_time = id_recog_time/id_recog_count;
    print fmt("identity recognition time: %s", id_recog_time);
}

event StatsCol::item_gen(item: StatsCol::Info)
{
    local id_recog_start = current_time();

    if(item?$func)
    {
        local index = item$conn$uid;
        if(item$is_response==F)
        {
            req_table[index] = [$requestor=item$sender,$responder=item$receiver, $protocol=item$protocol, $req_func=item$func];
        }
        else if(item$is_response==T && index in req_table
            && req_table[index]$requestor==item$receiver
            && req_table[index]$responder==item$sender
            && req_table[index]$protocol==item$protocol)
        {
            local func_pair = cat(item$protocol,";",req_table[index]$req_func,";",item$func);
            Intel::seen([$indicator=func_pair, $indicator_type=Intel::FUNCTION_PAIR, $conn=item$conn, $where=Iden::IN_PROTOCOL_HEADER]);
        }
    }
    
    local id_recog_end = current_time();
    id_recog_time += id_recog_end - id_recog_start;
    id_recog_count += 1;
}

# A helper function to update the recognition table when a match is found.
function add_identity(ip: addr, identity: string)
{
    local info: Info;
    info = Info($ts=network_time(), $ip=ip, $identity=identity);

    if(ip !in identity_map)
    {
        identity_map[ip] = set(identity);
        Log::write(IdRecog::LOG, info);
    }
    else if(identity !in identity_map[ip])
    {
        add identity_map[ip][identity];
        Log::write(IdRecog::LOG, info);
    }
}

event Intel::match(s: Intel::Seen, items: set[Intel::Item])
{
    local matched_req_info = req_table[s$conn$uid];

    for(item in items)
    {
        local identity = split_string(item$meta$desc, /;/);
        if(identity[0]!="")
        {
            add_identity(matched_req_info$requestor, identity[0]);
        }
        if(identity[1]!="")
        {
            add_identity(matched_req_info$responder, identity[1]);
        }
    }
}

## Output the identity map
function print_identity()
{
    for( ip in identity_map )
    {
        local identities = "";
        for(identity in identity_map[ip])
        {
            identities = cat(identities, identity, " ");
        }
        print fmt("IP: %s, Indentity: %s", ip, identities);
    }
}
