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

module SumStats;

export {
        redef enum Calculation += { KEYED_AVERAGE };

        type keyed_avg_value: record{
            num: count;
            avg: double;
        };

        redef record ResultVal += {
            avg_table: table[string] of keyed_avg_value &optional;
        };
}

function add_avg_entry(mytable: table[string] of keyed_avg_value, str: string, another: keyed_avg_value)
{
    if ( str !in mytable )
    {    
        mytable[str] = [$num=0, $avg=0.0];
    }
    mytable[str]$num += another$num;
    mytable[str]$avg += (another$avg-mytable[str]$avg)*another$num/mytable[str]$num; 
}

hook register_observe_plugins()
{
    register_observe_plugin(KEYED_AVERAGE, function(r: Reducer, val: double, obs: Observation, rv: ResultVal)
    {
        # observations for us always need a string and a num. Otherwhise - complain.
        if ( ! obs?$str || ! obs?$dbl)
        {
            Reporter::error("KEYED_AVERAGE sumstats plugin needs str and dbl in observation");
            return;
        }

        local increment = 1;
        if ( obs?$num )
        {
            increment = obs$num;
        }
        if ( ! rv?$avg_table )
        {
            rv$avg_table = table();
        }
        add_avg_entry(rv$avg_table, obs$str, [$num=increment, $avg=obs$dbl]);
    });
}

hook compose_resultvals_hook(result: ResultVal, rv1: ResultVal, rv2: ResultVal)
{
    if ( ! (rv1?$avg_table || rv2?$avg_table ) )
    {
        return;
    }
    if ( !rv1?$avg_table )
    {
        result$avg_table = copy(rv2$avg_table);
        return;
    }

    if ( !rv2?$avg_table )
    {
        result$avg_table = copy(rv1$avg_table);
        return;
    }

    result$avg_table = copy(rv1$avg_table);

    for ( i in rv2$avg_table )
    {
        add_avg_entry(result$avg_table, i, rv2$avg_table[i]);
    }
}

