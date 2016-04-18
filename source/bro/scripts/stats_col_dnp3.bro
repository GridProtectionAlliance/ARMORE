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
##! The DNP3 statistic collector.

module StatsCol;

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

# Get the additional addresses from the header
event dnp3_header_block(c: connection, is_orig: bool, start: count, len: count, ctrl: count, dest_addr: count, src_addr: count)
{
    if (col_levels >=4)
    {
        g_uid = cat(src_addr, ":", dest_addr);
        g_info$uid = g_uid;
    } 
}

# Get the DNP3 protocol name and the function code from the dnp3 application request header
event dnp3_application_request_header(c: connection, is_orig: bool, application: count, fc: count)
{
    if (col_levels >= 3)
    {
        if (measure)
        {
            g_time_20 = current_time();
            g_packet_20 += g_time_20 - g_time_12;
        }
        g_pro = "DNP3";
        g_info$protocol = g_pro;
        if (col_levels >= 5)
        {
            g_fun = DNP3::function_codes[fc];
            g_info$func = g_fun;
            g_is_response = F;
            g_info$is_response = F;
        }

        if (col_levels <= 5)
        {
            g_wait = F;
            if (measure)
            {
                 g_time_21 = current_time();
                 g_packet_21 += g_time_21 - g_time_20;
            }
            event StatsCol::item_seen(g_info);
            if (measure)
            {
                 g_time_22 = current_time();
                 g_packet_22 += g_time_22 - g_time_21;
                 g_time_40 = current_time();
                 g_packet_40 += g_time_40 - g_time_22;
            }
            event StatsCol::item_gen(g_info);
            if (measure)
            {
                 g_packet_41 += current_time() - g_time_40;
            }
        }
        else
        {
            if (measure)
            {
                 g_time_21 = current_time();
                 g_packet_21 += g_time_21 - g_time_20;
            }
            event StatsCol::item_seen([$ts=g_ts, $len=g_len, $sender=g_src, $receiver=g_dst, $conn=c, $protocol=g_pro, $uid=g_uid, $func=g_fun, $is_response=g_is_response]);
            if (measure)
            {
                 g_time_22 = current_time();
                 g_packet_22 += g_time_22 - g_time_21;
                 g_time_l2 = g_time_22;
            }
        }
    }
}

# Get the DNP3 protocol name and the function code from the dnp3 application response header
event dnp3_application_response_header(c: connection, is_orig: bool, application: count, fc: count, iin: count)
{
    if (col_levels >= 3)
    {
        if (measure)
        {
            g_time_20 = current_time();
            g_packet_20 += g_time_20 - g_time_12;
        }
        g_pro = "DNP3";
        g_info$protocol = g_pro;
        if (col_levels >= 5)
        {
            g_fun = DNP3::function_codes[fc];
            g_info$func = g_fun;
            g_is_response = T;
            g_info$is_response = T;
        }
        
        if (col_levels <= 5)
        {
            g_wait = F;
            if (measure)
            {
                 g_time_21 = current_time();
                 g_packet_21 += g_time_21 - g_time_20;
            }
            event StatsCol::item_seen(g_info);
            if (measure)
            {
                 g_time_22 = current_time();
                 g_packet_22 += g_time_22 - g_time_21;
                 g_time_40 = current_time();
                 g_packet_40 += g_time_40 - g_time_22;
            }
            event StatsCol::item_gen(g_info);
            if (measure)
            {
                 g_packet_41 += current_time() - g_time_40;
            }
        }
        else
        {
            if (measure)
            {
                 g_time_21 = current_time();
                 g_packet_21 += g_time_21 - g_time_20;
            }
            event StatsCol::item_seen([$ts=g_ts, $len=g_len, $sender=g_src, $receiver=g_dst, $conn=c, $protocol=g_pro, $uid=g_uid, $func=g_fun, $is_response=g_is_response]);
            if (measure)
            {
                 g_time_22 = current_time();
                 g_packet_22 += g_time_22 - g_time_21;
                 g_time_l2 = g_time_22;
            }
        }
    }
}

# Depending on the qualifier code, store different statistics in the target level
event dnp3_object_header(c: connection, is_orig: bool, obj_type: count, qua_field: count, number: count, rf_low: count, rf_high: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        dnp3_group = get_group(obj_type);
        dnp3_variation = get_variation(obj_type);

        local obj_prefix_code = get_obj_prefix_code(qua_field);
        local range_spec_code = get_range_spec_code(qua_field);
        dnp3_prefix = F;

        if (obj_prefix_code == 0 && (range_spec_code == 0 || range_spec_code == 1))
        {
            g_tgt = target_gen("Group", dnp3_group, 1);
            g_tgt = cat(g_tgt, " ", target_gen("Variation", dnp3_variation, 1));
            g_tgt = cat(g_tgt, " ", target_gen("Target", rf_low, rf_high-rf_low+1));
            g_info$target = g_tgt;
            g_wait = F;
            if (measure)
            {
                g_time_31 = current_time();
                g_packet_31 += g_time_31 - g_time_30;
            }
            event StatsCol::item_seen(g_info);
            if (measure)
            {
                g_time_32 = current_time();
                g_packet_32 += g_time_32 - g_time_31;
                g_time_40 = current_time();
                g_packet_40 += g_time_40 - g_time_32;
            }
            event StatsCol::item_gen(g_info);
            if (measure)
            {
                g_packet_41 += current_time() - g_time_40;
            }
        }

        if (obj_prefix_code == 0 && range_spec_code == 6)
        {
            g_tgt = target_gen("Group", dnp3_group, 1);
            g_tgt = cat(g_tgt, " ", target_gen("Variation", dnp3_variation, 1));
            g_tgt = cat(g_tgt, " ", "Target:all");
            g_info$target = g_tgt;
            g_wait = F;
            if (measure)
            {
                g_time_31 = current_time();
                g_packet_31 += g_time_31 - g_time_30;
            }
            event StatsCol::item_seen(g_info);
            if (measure)
            {
                g_time_32 = current_time();
                g_packet_32 += g_time_32 - g_time_31;
                g_time_40 = current_time();
                g_packet_40 += g_time_40 - g_time_32;
            }
            event StatsCol::item_gen(g_info);
            if (measure)
            {
                g_packet_41 += current_time() - g_time_40;
            }
        }

        if (obj_prefix_code == 0 && (range_spec_code == 7 || range_spec_code == 8))
        {
            dnp3_quantity = rf_low;

            g_tgt = target_gen("Group", dnp3_group, 1);
            g_tgt = cat(g_tgt, " ", target_gen("Variation", dnp3_variation, 1));
            g_tgt = cat(g_tgt, " ", target_gen("Quantity", dnp3_quantity, 1));
            g_info$target = g_tgt;
            g_wait = F;
            if (measure)
            {
                g_time_31 = current_time();
                g_packet_31 += g_time_31 - g_time_30;
            }
            event StatsCol::item_seen(g_info);
            if (measure)
            {
                g_time_32 = current_time();
                g_packet_32 += g_time_32 - g_time_31;
                g_time_40 = current_time();
                g_packet_40 += g_time_40 - g_time_32;
            }
            event StatsCol::item_gen(g_info);
            if (measure)
            {
                g_packet_41 += current_time() - g_time_40;
            }
        }

        if ((obj_prefix_code == 1 && range_spec_code == 7) || (obj_prefix_code == 2 && range_spec_code == 8))
        {
            dnp3_quantity = rf_low;
            dnp3_prefix = T;
        }

        if (obj_prefix_code == 5 && range_spec_code == 11)
        {
            dnp3_quantity = rf_low;
            g_tgt = target_gen("Group", dnp3_group, 1);
            g_tgt = cat(g_tgt, " ", target_gen("Variation", dnp3_variation, 1));
            g_tgt = cat(g_tgt, " ", target_gen("Quantity", dnp3_quantity, 1));
            g_info$target = g_tgt;
            g_wait = F;
            if (measure)
            {
                g_time_31 = current_time();
                g_packet_31 += g_time_31 - g_time_30;
            }
            event StatsCol::item_seen(g_info);
            if (measure)
            {
                g_time_32 = current_time();
                g_packet_32 += g_time_32 - g_time_31;
                g_time_40 = current_time();
                g_packet_40 += g_time_40 - g_time_32;
            }
            event StatsCol::item_gen(g_info);
            if (measure)
            {
                g_packet_41 += current_time() - g_time_40;
            }
        }
    }
}

# In case the qualifier code is 0x17 or 0x28, look at the following prefixes to get the indexes
event dnp3_object_prefix(c: connection, is_orig: bool, prefix_value: count)
{
    if (col_levels == 6)
    {
        if (dnp3_prefix)
        {
            #print fmt("group:%d variation:%d target:%d", dnp3_group, dnp3_variation, prefix_value);

            g_tgt = target_gen("Group", dnp3_group, 1);
            g_tgt = cat(g_tgt, " ", target_gen("Variation", dnp3_variation, 1));
            g_tgt = cat(g_tgt, " ", target_gen("Target", prefix_value, 1));
            g_info$target = g_tgt;
            g_wait = F;
            if (measure)
            {
                g_time_31 = current_time();
                g_packet_31 += g_time_31 - g_time_30;
            }
            event StatsCol::item_seen(g_info);
            if (measure)
            {
                g_time_32 = current_time();
                g_packet_32 += g_time_32 - g_time_31;
                g_time_40 = current_time();
                g_packet_40 += g_time_40 - g_time_32;
            }
            event StatsCol::item_gen(g_info);
            if (measure)
            {
                g_packet_41 += current_time() - g_time_40;
            }
        }
    }
}
