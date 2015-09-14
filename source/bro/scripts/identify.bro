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
#! Recognize identities of units in the network based on predefined rules in the input file

@load base/frameworks/intel
@load frameworks/intel/seen
@load frameworks/intel/do_notice
@load base/utils/addrs

module IdentityRecognizer;

# Read the predefined recognition rules
redef Intel::read_files += {
    fmt("%s/identify.dat", @DIR)
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
        ## IP of the unit
        ip: addr &log;
        ## Identity of the unit
        identity: string &log;
    };

    ## A table used for holding all the recognitions of the identities.
    global identity_map: table[addr] of set[string];

    global log_identify: event(rec: Info);
}

    # The in memory data structure for holding a piece of function pair information.
    type function_pair_information: record {
        requestor: addr;
        responder: addr;
        conn: connection;
        protocol: string;
        request_func: string;
        response_func: string &optional;
    };

    # An in memory table for holding function pair information related to different connections.
    global function_pair: table[string] of function_pair_information;
    global g_src: addr;    # global variable to store the sender's ip address
    global g_dst: addr;    # global variable to store the receiver's ip address

event bro_init() &priority=5
{
    Log::create_stream(LOG, [$columns=Info, $ev=log_identify]);
}

# Record the packet sender and receiver.
event new_packet(c: connection, p: pkt_hdr)
{
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
    
# Get the function code from the Modbus messege header.
event modbus_message(c: connection, headers: ModbusHeaders, is_orig: bool)
{
    local func: string;

    if (is_orig)
    {
        func = cat(Modbus::function_codes[headers$function_code], "_REQUEST");
        function_pair[c$uid] = [$requestor=g_src, $responder=g_dst, $conn=c, $protocol="Modbus", $request_func=func];
    }
    else
    {
        if (c$uid in function_pair && function_pair[c$uid]$protocol=="Modbus")
        {
            func = cat(Modbus::function_codes[headers$function_code], "_RESPONSE");
            function_pair[c$uid]$response_func = func;
            func = cat("MODBUS_", function_pair[c$uid]$request_func, "_", function_pair[c$uid]$response_func);
            Intel::seen([$indicator=func, $indicator_type=Intel::FUNCTION_PAIR, $conn=c, $where=Iden::IN_PROTOCOL_HEADER]);
        }
    }
}

# Get the function code from the DNP3 application request header.
event dnp3_application_request_header(c: connection, is_orig: bool, application: count, fc: count)
{
    local func = DNP3::function_codes[fc];
    function_pair[c$uid] = [$requestor=g_src, $responder=g_dst, $conn=c, $protocol="DNP3", $request_func=func];
}

# Get the function code from the DNP3 application response header.
event dnp3_application_response_header(c: connection, is_orig: bool, application:count, fc: count, iin: count)
{
    local func = DNP3::function_codes[fc];
    if (c$uid in function_pair && function_pair[c$uid]$protocol=="DNP3")
    {
        function_pair[c$uid]$response_func = func;
        func = cat("DNP3_", function_pair[c$uid]$request_func, "_", function_pair[c$uid]$response_func);
        Intel::seen([$indicator=func, $indicator_type=Intel::FUNCTION_PAIR, $conn=c, $where=Iden::IN_PROTOCOL_HEADER]);
    }
}

# A helper function to update the recognition table when a match is found.
function add_identity(ip: addr, identity: string)
{
    local info: Info;
    info = Info($ip=ip, $identity=identity);

    if(ip !in identity_map)
    {
        identity_map[ip] = set(identity);
        Log::write(IdentityRecognizer::LOG, info);
    }
    else if(identity !in identity_map[ip])
    {
        add identity_map[ip][identity];
        Log::write(IdentityRecognizer::LOG, info);
    }
}

event Intel::match(s: Intel::Seen, items: set[Intel::Item])
{
    local func_pair_info = function_pair[s$conn$uid];

    for(item in items)
    {
        local identity = split_string(item$meta$desc, /;/);
        if(identity[0]!="")
        {
            add_identity(func_pair_info$requestor, identity[0]);
        }
        if(identity[1]!="")
        {
            add_identity(func_pair_info$responder, identity[1]);
        }
    }
}

event bro_done()
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
