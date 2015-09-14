//******************************************************************************************************
//  StandardLogger.cpp - Gbtc
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

#include <stdio.h>
#include <time.h>
#include "StandardLogger.h"

static void StandardLogInfo(const char *message);
static void StandardLogError(const char *message);
static void GetDateTime(char* datetime, int size);

StandardLogger::StandardLogger(bool logToConsole)
{
    m_logToConsole = logToConsole;
}

StandardLogger::~StandardLogger()
{
}

LogFunction StandardLogger::GetLogInfoFunction()
{
    if (m_logToConsole)
        return StandardLogInfo;

    return nullptr;
}

LogFunction StandardLogger::GetLogErrorFunction()
{
    if (m_logToConsole)
        return StandardLogError;

    return nullptr;
}

static void StandardLogInfo(const char *message)
{
    char datetime[256];
    GetDateTime(datetime, 256);
    fprintf(stdout, "%s %s\n", datetime, message);
}

static void StandardLogError(const char *message)
{
    char datetime[256];
    GetDateTime(datetime, 256);
    fprintf(stderr, "%s %s\n", datetime, message);
}

static void GetDateTime(char* datetime, int size)
{
    time_t now;
    tm* timeinfo;

    // Get current time
    time(&now);
    timeinfo = localtime(&now);

    strftime(datetime, size, "%y-%m-%d %H:%M:%S", timeinfo);
}