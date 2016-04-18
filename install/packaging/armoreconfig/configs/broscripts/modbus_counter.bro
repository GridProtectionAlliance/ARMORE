##! The Modbus statistic analyzer.

module StatisticAnalyzer;

global modbus_start_address: count;          # global variable to store the start address of the targets
global modbus_start_address2: count;         # global variable to store extra start address (only useful for the reand&write)
global modbus_quantity: count;               # global variable to store the quantity of the targets
global modbus_quantity2: count;              # global variable to store extra quantity (only useful for the read&write)
global modbus_coils: vector of bool;         # global vector to store multiple coil values
global modbus_registers: vector of count;    # global vector to store multiple register values
global modbus_coil: bool;                    # global variable to store a single coil value
global modbus_register: count;               # global variable to store a single register value
global modbus_and_mask: count;               # global variable to store the and mask
global modbus_or_mask: count;                # global variable to store the or mask

# Get the protocol name and the function code from the Modbus messege header
event modbus_message(c: connection, headers: ModbusHeaders, is_orig: bool)
{
    if (levels >= 3)
    {
        g_pro = "Modbus";
        SumStats::observe("modbus", [], [$str=cat(g_src, ";", g_dst, ";", g_pro), $num=1]);
        statsd_increment("ModbusMessage L3", 1);
    }

    if (levels >= 4)
    {
        g_fun = Modbus::function_codes[headers$function_code];
        if (is_orig)
            g_fun = cat(g_fun, "_REQUEST");
        else
            g_fun = cat(g_fun, "_RESPONSE");
        SumStats::observe("modbus_fun", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun), $num=1]);
        
        statsd_increment("ModbusMessage L4", 1);
    }
        statsd_increment("ModbusMessage", 1);
}

# The following event handlers deal with different Modbus functions and store their targets statistics in the target level

event modbus_read_coils_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    g_tgt = vector();
    if (quantity > 0)
    {
        modbus_start_address = start_address;
        modbus_quantity = quantity;
        #print fmt("%d ~ %d", start_address, start_address + quantity - 1);

        if (levels >= 5)
        {
            local v = range(modbus_start_address, modbus_start_address + modbus_quantity - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Coil"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Coil:", v[i]), $num=1]);
            }
        }
    }
        statsd_increment("ModbusReadCoil", 1);
}

event modbus_read_coils_response(c: connection, headers: ModbusHeaders, coils: ModbusCoils)
{
    modbus_coils = vector();
    for (i in coils)
    {
        if (i < modbus_quantity)
        {
            modbus_coils[i] = coils[i];
        }
    }

    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Coil:", g_tgt[i]["Coil"]), $num=1]);
        }
    }
    
    #print modbus_coils;
        statsd_increment("ModbusCoilResponse", 1);
}

event modbus_read_discrete_inputs_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    g_tgt = vector();
    if (quantity > 0)
    {
        modbus_start_address = start_address;
        modbus_quantity = quantity;
        if (levels >= 5)
        {
            local v = range(modbus_start_address, modbus_start_address + modbus_quantity - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Discrete_Input"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Discrete_Input:", v[i]), $num=1]);
            }
        }
        #print fmt("%d ~ %d", start_address, start_address + quantity - 1);
    }     
        statsd_increment("ModbusCoilInput", 1);
}

event modbus_read_discrete_inputs_response(c: connection, headers: ModbusHeaders, coils: ModbusCoils)
{
    modbus_coils = vector();
    for (i in coils)
    {
        if (i < modbus_quantity)
        {
            modbus_coils[i] = coils[i];
        }
    }

    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Discrete_Input:", g_tgt[i]["Discrete_Input"]), $num=1]);
        }
    }
    #print modbus_coils;
        statsd_increment("ModbusCoilInput", 1);
}

event modbus_read_holdinmodbus_registers_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    g_tgt = vector();
    if (quantity > 0)
    {
        modbus_start_address = start_address;
        modbus_quantity = quantity;
        #print fmt("%d ~ %d", start_address, start_address + quantity - 1);

        if (levels >= 5)
        {
            local v = range(modbus_start_address, modbus_start_address + modbus_quantity - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Holdinmodbus_register"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", v[i]), $num=1]);
            }
        }
    }
}

event modbus_read_holdinmodbus_registers_response(c: connection, headers: ModbusHeaders, registers: ModbusRegisters)
{
    modbus_registers = registers;
    
    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", g_tgt[i]["Holdinmodbus_register"]), $num=1]);
        }
    }
    
    #print modbus_registers;
}

event modbus_read_input_registers_request(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    g_tgt = vector();
    if (quantity > 0)
    {
        modbus_start_address = start_address;
        modbus_quantity = quantity;
        #print fmt("%d ~ %d", start_address, start_address + quantity - 1);

        if (levels >= 5)
        {
            local v = range(modbus_start_address, modbus_start_address + modbus_quantity - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Input_Register"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Input_Register:", v[i]), $num=1]);
            }
        }
    }
        statsd_increment("ModbusCoilRequest", 1);
}

event modbus_read_input_registers_response(c: connection, headers: ModbusHeaders, registers: ModbusRegisters)
{
    modbus_registers = registers;

    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Input_Register:", g_tgt[i]["Input_Register"]), $num=1]);
        }
    }
        statsd_increment("ModbusCoilResponse", 1);

    #print modbus_registers;
}

event modbus_write_single_coil_request(c: connection, headers: ModbusHeaders, address: count, value: bool)
{
    g_tgt = vector();
    modbus_start_address = address;
    modbus_coil = value;
    #print fmt("%d: %s", address, value);

    if (levels >= 5)
    {
        g_tgt[|g_tgt|] = table(["Coil"] = address);
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Coil:", address), $num=1]);
    }
        statsd_increment("ModbusCoilRequest", 1);
}

event modbus_write_single_coil_response(c: connection, headers: ModbusHeaders, address: count, value: bool)
{
    if (address != modbus_start_address || value != modbus_coil)
    {
        print "Unexpected write single coil response!";
    }

    if (levels >= 5)
    {
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Coil:", g_tgt[0]["Coil"]), $num=1]);
    }
}

event modbus_write_single_register_request(c: connection, headers: ModbusHeaders, address: count, value: count)
{
    g_tgt = vector();
    modbus_start_address = address;
    modbus_register = value;
    #print fmt("%d: %d", address, value);

    if (levels >= 5)
    {
        g_tgt[|g_tgt|] = table(["Holdinmodbus_register"] = address);
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", address), $num=1]);
    }
        statsd_increment("ModbusCoilRegister", 1);
}

event modbus_write_single_register_response(c: connection, headers: ModbusHeaders, address: count, value: count)
{
    if (address != modbus_start_address || value != modbus_register)
    {
        print "Unexpected write single register response!";
    }

    if (levels >= 5)
    {
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", g_tgt[0]["Holdinmodbus_register"]), $num=1]);
    }
        statsd_increment("ModbusCoilRegisterResponse", 1);
}

event modbus_write_multiple_coils_request(c: connection, headers: ModbusHeaders, start_address: count, coils: ModbusCoils)
{
    modbus_start_address = start_address;
    modbus_coils = coils;
    modbus_quantity = |coils|;

    if (modbus_quantity > 0)
    {
        g_tgt = vector();

        if (levels >= 5)
        {
            local v = range(modbus_start_address, modbus_start_address + modbus_quantity - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Coil"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Coil:", v[i]), $num=1]);
            }
        }
    }
}

event modbus_write_multiple_coils_response(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    local coils = modbus_coils;
    modbus_coils = vector();
    for (i in coils)
    {
        if (i < modbus_quantity)
        {
            modbus_coils[i] = coils[i];
        }
    }
    #print fmt("%d ~ %d", modbus_start_address, modbus_start_address + modbus_quantity - 1);
    #print modbus_coils;

    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Coil:", g_tgt[i]["Coil"]), $num=1]);
        }
    }
}

event modbus_write_multiple_registers_request(c: connection, headers: ModbusHeaders, start_address: count, registers: ModbusRegisters)
{
    modbus_start_address = start_address;
    modbus_registers = registers;
    modbus_quantity = |registers|;

    if (modbus_quantity > 0)
    {
        g_tgt = vector();

        if (levels >= 5)
        {
            local v = range(modbus_start_address, modbus_start_address + modbus_quantity - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Holdinmodbus_register"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", v[i]), $num=1]);
            }
        }
    }
}

event modbus_write_multiple_registers_response(c: connection, headers: ModbusHeaders, start_address: count, quantity: count)
{
    #print fmt("%d ~ %d", modbus_start_address, modbus_start_address + modbus_quantity - 1);
    #print modbus_registers;

    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", g_tgt[i]["Holdinmodbus_register"]), $num=1]);
        }
    }
}

event modbus_read_file_record_request(c: connection, headers: ModbusHeaders)
{
    g_tgt = vector();
}

event modbus_file_record_request(c: connection, ref_tpe: count, file_num: count, record_num: count, record_len: count)
{
    if (levels >= 5)
    {
        local v = range(record_num, record_len-1);
        for (i in v)
        {
            g_tgt[|g_tgt|] = table(["File_Number"] = file_num, ["Record_Number"] = v[i]);
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "File_Number:", file_num, " ", "Record_Number:", i), $num=1]);
        }
    }
}

event modbus_file_record_response(c: connection, file_len: count, ref_type: count)
{
    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "File_Number:", g_tgt[i]["File_Number"], " ", "Record_Number:",g_tgt[i]["Record_Number"]), $num=1]);
        }
    }
        statsd_increment("ModbusFileRecord", 1);
}

event modbus_write_file_record_request(c: connection, headers: ModbusHeaders)
{
    g_tgt = vector();
}

event modbus_reference_with_data(c: connection, ref_type: count, file_num: count, record_num: count, word_count: count)
{
    if (levels >= 5)
    {
        local v = range(record_num, word_count-1);
        for (i in v)
        {
            g_tgt[|g_tgt|] = table(["File_Number"] = file_num, ["Record_Number"] = v[i]);
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "File_Number:", file_num, " ", "Record_Number:", i), $num=1]);
        }
    }
        statsd_increment("ModbusData", 1);
}

event modbus_write_file_record_response(c: connection, headers: ModbusHeaders)
{
    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "File_Number:", g_tgt[i]["File_Number"], " ", "Record_Number:",g_tgt[i]["Record_Number"]), $num=1]);
        }
    }
}

event modbus_mask_write_register_request(c: connection, headers: ModbusHeaders, address: count, and_mask: count, or_mask: count)
{
    g_tgt = vector();
    modbus_start_address = address;
    modbus_and_mask = and_mask;
    modbus_or_mask = or_mask;

    if (levels >= 5)
    {
        g_tgt[|g_tgt|] = table(["Holdinmodbus_register"] = address);
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", address), $num=1]);
    }
}

event modbus_mask_write_register_response(c: connection, headers: ModbusHeaders, address: count, and_mask: count, or_mask: count)
{
    if (address != modbus_start_address || and_mask != modbus_and_mask || or_mask != modbus_or_mask)
    {
        print "Unexpected mask write register response!";
    }

    if (levels >= 5)
    {
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Holdinmodbus_register:", g_tgt[0]["Holdinmodbus_register"]), $num=1]);
    }
}

event modbus_read_write_multiple_registers_request(c: connection, headers: ModbusHeaders, read_start_address: count, read_quantity: count, write_start_address: count, write_registers: ModbusRegisters)
{
    modbus_start_address = write_start_address;
    modbus_registers = write_registers;
    modbus_start_address2 = read_start_address;
    modbus_quantity = |write_registers|;
    modbus_quantity2 = read_quantity;

    g_tgt = vector();

    if (levels >= 5)
    {
        if (modbus_quantity > 0)
        {
            local v = range(modbus_start_address, modbus_start_address + modbus_quantity - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Write_Holdinmodbus_register"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Write_Holdinmodbus_register:", v[i]), $num=1]);
            }
        }

        if (modbus_quantity2 > 0)
        {
            v = range(modbus_start_address, modbus_start_address2 + modbus_quantity2 - 1);
            for (i in v)
            {
                g_tgt[|g_tgt|] = table(["Read_Holdinmodbus_register"] = v[i]);
                SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Read_Holdinmodbus_register:", v[i]), $num=1]);
            }
        }
    }
}

event modbus_read_write_multiple_registers_response(c: connection, headers: ModbusHeaders, written_registers: ModbusRegisters)
{
    modbus_registers = written_registers;

    if (levels >= 5)
    {
        for (i in g_tgt)
        {
            if ("Write_Holdinmodbus_register" in g_tgt[i])
            {
                 SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Write_Holdinmodbus_register:", g_tgt[i]["Write_Holdinmodbus_register"]), $num=1]);
            }
            if ("Read_Holdinmodbus_register" in g_tgt[i])
            {
                 SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "Read_Holdinmodbus_register:", g_tgt[i]["Read_Holdinmodbus_register"]), $num=1]);
            }
        }
    }
    
}

event modbus_read_fifo_queue_request(c: connection, headers: ModbusHeaders, start_address: count)
{
    g_tgt = vector();
    modbus_start_address = start_address;

    if (levels >= 5)
    {
        g_tgt[|g_tgt|] = table(["FIFO_Queue"] = start_address);
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "FIFO_Queue:", start_address), $num=1]);
    }
}

event modbus_read_fifo_queue_response(c: connection, headers: ModbusHeaders, fifos: ModbusRegisters)
{
    modbus_registers = fifos;
    
    if (levels >= 5)
    {
        SumStats::observe("modbus_tgt", [], [$str=cat(g_src, ";", g_dst, ";", g_pro, ";", g_fun, ";", "FIFO_Queue:", g_tgt[0]["FIFO_Queue"]), $num=1]);
    }
}

event bro_done()
{
}

