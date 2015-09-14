//******************************************************************************************************
//  ConsoleLogger.h - Gbtc
//
//  Copyright � 2015, Grid Protection Alliance.  All Rights Reserved.
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

#ifndef _CONSOLELOGGER_H
#define _CONSOLELOGGER_H

typedef void(*LogFunction)(const char *message);

// Thread-safe global logging functions
extern void log_info(const char *format, ...);
extern void log_error(const char *format, ...);

enum class ConsoleLoggerOption : int
{
    ZeroMQ,
    Standard,
    Default = ZeroMQ
};

class ConsoleLogger
{
public:
    // Constructors
    ConsoleLogger();
    virtual ~ConsoleLogger();

    // Methods
    virtual LogFunction GetLogInfoFunction() = 0;
    virtual LogFunction GetLogErrorFunction() = 0;
};

void InitializeConsoleLogger(ConsoleLogger* logger);

#endif