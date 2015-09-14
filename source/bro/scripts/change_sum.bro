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
module SumStats;

export {
        redef enum Calculation += { COUNTTABLE };

        redef record ResultVal += {
            counttable: table[string] of count &optional;
        };
}

function add_ct_entry(mytable: table[string] of count, str: string, num: count)
{
    if ( str !in mytable )
    {    
        mytable[str] = 0;
    }
    mytable[str] += num;
}

hook register_observe_plugins()
{
    register_observe_plugin(COUNTTABLE, function(r: Reducer, val: double, obs: Observation, rv: ResultVal)
    {
        # observations for us always need a string and a num. Otherwhise - complain.
        if ( ! obs?$str )
        {
            Reporter::error("COUNTTABLE sumstats plugin needs str in observation");
            return;
        }

        local increment = 1;
        if ( obs?$num )
        {
            increment = obs$num;
        }
        if ( ! rv?$counttable )
        {
            rv$counttable = table();
        }
        add_ct_entry(rv$counttable, obs$str, increment);
    });
}

hook compose_resultvals_hook(result: ResultVal, rv1: ResultVal, rv2: ResultVal)
{
    if ( ! (rv1?$counttable || rv2?$counttable ) )
    {
        return;
    }
    if ( !rv1?$counttable )
    {
        result$counttable = copy(rv2$counttable);
        return;
    }

    if ( !rv2?$counttable )
    {
        result$counttable = copy(rv1$counttable);
        return;
    }

    result$counttable = copy(rv1$counttable);

    for ( i in rv2$counttable )
    {
        add_ct_entry(result$counttable, i, rv2$counttable[i]);
    }
}

