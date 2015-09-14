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
#! The DNP3 statistic analyzer.

module StatisticAnalyzer;

global dnp3_group: count;      # global variable to store the group number
global dnp3_variation: count;  # global variable to store the variation number
global dnp3_quantity: count;   # global variable to store the count value
global dnp3_prefix: bool;      # global variable to indicate whether following prefixes should be paid attention to

# Extract the group number from a combination of the group and variation number
function get_group(obj_type: count): count
{
    return (obj_type - (obj_type % 256)) / 256;
}

# Extract the variation number from a combination of the group and variation number
function get_variation(obj_type: count): count
{
    return obj_type % 256;
}

# Extract the boject prefix code from the qualifier code
function get_obj_prefix_code(qua_field: count): count
{
    return ((qua_field - (qua_field % 16)) / 16) % 8;
}

# Extract the range specification code from the qualifier code
function get_range_spec_code(qua_field: count): count
{
    return qua_field % 16;
}

# Get the DNP3 protocol name and the function code from the dnp3 application request header
event dnp3_application_request_header(c: connection, is_orig: bool, fc: count)
{
    g_pro = "DNP3";
    if (levels >= 3)
    {
        SumStats::observe("dnp3", [], [$str=cat(g_src, ";", g_dst, ";", g_pro), $num=1]);
    }

    if (levels >= 4)
    {
        g_fun = DNP3::function_codes[fc];
        SumStats::observe("dnp3_fun", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun), $num=1]);
    }
}

# Get the DNP3 protocol name and the function code from the dnp3 application response header
event dnp3_application_response_header(c: connection, is_orig: bool, fc: count, iin: count)
{
    g_pro = "DNP3";
    if (levels >= 3)
    {
        SumStats::observe("dnp3", [], [$str=cat(g_src, ";", g_dst, ";", g_pro), $num=1]);
    }

    if (levels >= 4)
    {
        g_fun = DNP3::function_codes[fc];
        SumStats::observe("dnp3_fun", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun), $num=1]);
    }
}

# Depending on the qualifier code, store different statistics in the target level
event dnp3_object_header(c: connection, is_orig: bool, obj_type: count, qua_field: count, number: count, rf_low: count, rf_high: count)
{
    if (levels >= 5)
    {
        g_pro = "DNP3";
        dnp3_group = get_group(obj_type);
        dnp3_variation = get_variation(obj_type);

        local obj_prefix_code = get_obj_prefix_code(qua_field);
        local range_spec_code = get_range_spec_code(qua_field);
        g_tgt = vector();
        dnp3_prefix = F;


        if (obj_prefix_code == 0 && (range_spec_code == 0 || range_spec_code == 1))
        {
            local v = range(rf_low, rf_high);
            for (i in v)
                {
                    g_tgt[|g_tgt|] = table(["Group"] = dnp3_group, ["Variation"] = dnp3_variation, ["Target"] = v[i]);
                    SumStats::observe("dnp3_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Group:", dnp3_group, " ",
                                                                  "Variation:", dnp3_variation, " ", "Target:", v[i]), $num=1]);
                }
        }

        if (obj_prefix_code == 0 && range_spec_code == 6)
        {
            g_tgt[|g_tgt|] = table(["Group"] = dnp3_group, ["Variation"] = dnp3_variation, ["Target_All"] = 1);
            SumStats::observe("dnp3_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Group:", dnp3_group, " ",
                                                                  "Variation:", dnp3_variation, " ", "Target_All:", 1), $num=1]);
        }

        if (obj_prefix_code == 0 && (range_spec_code == 7 || range_spec_code == 8))
        {
            dnp3_quantity = rf_low;
            g_tgt[|g_tgt|] = table(["Group"] = dnp3_group, ["Variation"] = dnp3_variation, ["Quantity"] = dnp3_quantity);
            SumStats::observe("dnp3_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Group:", dnp3_group, " ",
                                                                  "Variation:", dnp3_variation, " ", "Quantity:", dnp3_quantity), $num=1]);
        }

        if ((obj_prefix_code == 1 && range_spec_code == 7) || (obj_prefix_code == 2 && range_spec_code == 8))
        {
            dnp3_quantity = rf_low;
            dnp3_prefix = T;
        }

        if (obj_prefix_code == 5 && range_spec_code == 11)
        {
            dnp3_quantity = rf_low;
            g_tgt[|g_tgt|] = table(["Group"] = dnp3_group, ["Variation"] = dnp3_variation, ["Quantity"] = dnp3_quantity);
            SumStats::observe("dnp3_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Group:", dnp3_group, " ",
                                                                  "Variation:", dnp3_variation, " ", "Quantity:", dnp3_quantity), $num=1]);
        }
    }
}

# In case the qualifier code is 0x17 or 0x28, look at the following prefixes to get the indexes
event dnp3_object_prefix(c: connection, is_orig: bool, prefix_value: count)
{
    g_pro = "DNP3";
    if (levels >=5 && dnp3_prefix)
    {
        #print fmt("group:%d variation:%d target:%d", dnp3_group, dnp3_variation, prefix_value);
        g_tgt[|g_tgt|] = table(["Group"] = dnp3_group, ["Variation"] = dnp3_variation, ["Target"] = prefix_value);
        SumStats::observe("dnp3_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Group:", dnp3_group, " ",
                                                              "Variation:", dnp3_variation, " ", "Target:", prefix_value), $num=1]);
    }
}
