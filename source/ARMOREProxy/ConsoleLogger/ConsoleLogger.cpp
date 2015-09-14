//******************************************************************************************************
//  ConsoleLogger.cpp - Gbtc
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

#include "ConsoleLogger.h"
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include "../SpinLock.h"

using namespace std;

static SpinLock s_consoleLock;
static LogFunction s_log_info = nullptr;
static LogFunction s_log_error = nullptr;
static char s_buffer[512];

ConsoleLogger::ConsoleLogger()
{
}

ConsoleLogger::~ConsoleLogger()
{
}

void InitializeConsoleLogger(ConsoleLogger* logger)
{
    s_log_info = logger->GetLogInfoFunction();
    s_log_error = logger->GetLogErrorFunction();
}

void log_info(const char *format, ...)
{
    if (s_log_info)
    {
        spinlock(s_consoleLock,
        {
            va_list argptr;
            va_start(argptr, format);
            vsprintf(s_buffer, format, argptr);
            va_end(argptr);
            s_log_info(s_buffer);
        });
    }
}

void log_error(const char *format, ...)
{
    if (s_log_error)
    {
        spinlock(s_consoleLock,
        {
            va_list argptr;
            va_start(argptr, format);
            vsprintf(s_buffer, format, argptr);
            va_end(argptr);
            s_log_error(s_buffer);
        });
    }
}
