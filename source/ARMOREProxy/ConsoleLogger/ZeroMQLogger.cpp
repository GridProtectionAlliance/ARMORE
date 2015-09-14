//******************************************************************************************************
//  ZeroMQLogger.cpp - Gbtc
//
//  Copyright © 2015, Grid Protection Alliance.  All Rights Reserved.
//
//  Licensed to the Grid Protection Alliance (GPA) under one or more contributor license agreements. See
//  the NOTICE file distributed with this work for additional information regarding copyright ownership.
//  The GPA licenses this file to you under the MIT License (MIT), the "License"; you may
//  not use this file except in compliance with the License. You may obtain a copy of the License at:
//
//      http://opensource.org/licenses/MIT
//
//  Unless agreed to in writing, the subject software distributed under the License is distributed on an
//  "AS-IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. Refer to the
//  License for the specific language governing permissions and limitations.
//
//  Code Modification History:
//  ----------------------------------------------------------------------------------------------------
//  06/10/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "ZeroMQLogger.h"
#include <czmq.h>

static void zmq_log_info(const char *message);
static void zmq_log_error(const char *message);

ZeroMQLogger::ZeroMQLogger(const char* logIdentity, bool logToConsole)
{
    // Set logging identity
    zsys_set_logident(logIdentity);
    
    // Enable system logging
    zsys_set_logsystem(true);
    
    if (logToConsole)
    {
        // Enable subscription based zmq logging
        zsys_set_logsender("inproc://logging");
        m_zmqConsoleLogger = zsys_socket(ZMQ_SUB, nullptr, 0);
        zmq_connect(m_zmqConsoleLogger, "inproc://logging");
        zmq_setsockopt(m_zmqConsoleLogger, ZMQ_SUBSCRIBE, "", 0);
    }
    else
    {
        // Disable console output
        zsys_set_logstream(nullptr);
    }
}

ZeroMQLogger::~ZeroMQLogger()
{
    // Terminate zmq console logging
    zsys_close(m_zmqConsoleLogger, nullptr, 0);
}

LogFunction ZeroMQLogger::GetLogInfoFunction()
{
    return zmq_log_info;
}

LogFunction ZeroMQLogger::GetLogErrorFunction()
{
    return zmq_log_error;
}

static void zmq_log_info(const char *message)
{
    zsys_info(message);
}

static void zmq_log_error(const char *message)
{
    zsys_error(message);
}