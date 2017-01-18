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
##! The Modbus statistic collector.

module StatsCol;

# Get the protocol name and the function code from the Modbus messege header
event modbus_message(c: connection, headers: ModbusHeaders, is_orig: bool)
{
    if (col_levels >= 3)
    {
        if (measure)
        {
            g_time_20 = current_time();
            g_packet_20 += g_time_20 - g_time_12;
        }
        g_pro = "Modbus";
        g_info$protocol = g_pro;
        if (col_levels >= 4)
        {
            g_uid = cat(headers$uid);
            g_info$uid = g_uid;

            if (col_levels >= 5)
            {

                g_fun = Modbus::function_codes[headers$function_code];
                if (is_orig)
                {
                    g_fun = cat(g_fun, "_REQUEST");
                    g_is_response = F;
                }
                else
                {
                    g_fun = cat(g_fun, "_RESPONSE");
                    g_is_response = T;
                }
                g_info$func = g_fun;
                g_info$is_response = g_is_response;
            }
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

# The following event handlers deal with different Modbus functions and store their targets statistics in the target level

event modbus_read_coils_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Coil", start_address, quantity);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_coils_response(c: connection, headers: ModbusHeaders, coils: ModbusCoils)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_discrete_inputs_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Discrete_Input", start_address, quantity);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_discrete_inputs_response(c: connection, headers: ModbusHeaders, coils: ModbusCoils)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_holding_registers_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Holding_Register", start_address, quantity);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_holding_registers_response(c: connection, headers: ModbusHeaders, registers: ModbusRegisters)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_input_registers_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Input_Register", start_address, quantity);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_input_registers_response(c: connection, headers: ModbusHeaders, registers: ModbusRegisters)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_single_coil_request(c: connection, headers: ModbusHeaders, address: count, value: bool)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Coil", address, 1);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_single_coil_response(c: connection, headers: ModbusHeaders, address: count, value: bool)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_single_register_request(c: connection, headers: ModbusHeaders, address: count, value: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Holdinmodbus_Register", address, 1);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_single_register_response(c: connection, headers: ModbusHeaders, address: count, value: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_multiple_coils_request(c: connection, headers: ModbusHeaders, start_address: count, coils: ModbusCoils)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Coil", start_address, |coils|);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_multiple_coils_response(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_multiple_registers_request(c: connection, headers: ModbusHeaders, start_address: count, registers: ModbusRegisters)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Holding_Register", start_address, |registers|);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_multiple_registers_response(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_file_record_request(c: connection, ref_tpe: count, file_num: count, record_num: count, record_len: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("File_Number", file_num, 1);
        g_tgt_table[g_conn$uid] = cat(g_tgt_table[g_conn$uid], " ", target_gen("Record_Number", record_num, record_len));
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_file_record_response(c: connection, file_len: count, ref_type: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_reference_with_data(c: connection, ref_type: count, file_num: count, record_num: count, word_count: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("File_Number", file_num, 1);
        g_tgt_table[g_conn$uid] = cat(g_tgt_table[g_conn$uid], " ", target_gen("Record_Number", record_num, word_count));
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_write_file_record_response(c: connection, headers: ModbusHeaders)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_mask_write_register_request(c: connection, headers: ModbusHeaders, address: count, and_mask: count, or_mask: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Holdinmodbus_Register", address, 1);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_mask_write_register_response(c: connection, headers: ModbusHeaders, address: count, and_mask: count, or_mask: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_write_multiple_registers_request(c: connection, headers: ModbusHeaders, read_start_address: count, read_quantity: count, write_start_address: count, write_registers: ModbusRegisters)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("Write_Holdinmodbus_Register", write_start_address, |write_registers|);
        g_tgt_table[g_conn$uid] = cat(g_tgt_table[g_conn$uid], " ", target_gen("Read_Holdinmodbus_Register", read_start_address, read_quantity));
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_write_multiple_registers_response(c: connection, headers: ModbusHeaders, written_registers: ModbusRegisters)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_fifo_queue_request(c: connection, headers: ModbusHeaders, start_address: count)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_tgt_table[g_conn$uid] = target_gen("FIFO_Queue", start_address, 1);
        g_info$target = g_tgt_table[g_conn$uid];
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

event modbus_read_fifo_queue_response(c: connection, headers: ModbusHeaders, fifos: ModbusRegisters)
{
    if (col_levels == 6)
    {
        if (measure)
        {
            g_time_30 = current_time();
            g_packet_30 += g_time_30 - g_time_22;
        }
        g_wait = F;
        g_info$target = g_tgt_table[g_conn$uid];
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

