//******************************************************************************************************
//  MiddlewareEngine.cpp - Gbtc
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
//  06/11/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "MiddlewareEngine.h"
#include <stdexcept>

using namespace std;

MiddlewareEngine::MiddlewareEngine(BufferReceivedCallback callback, bool serverMode, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput) : 
    MiddlewareEngine(callback, serverMode, securityEnabled, inactiveClientTimeout, verboseOutput, UUID::NewUUID())
{
}

MiddlewareEngine::MiddlewareEngine(BufferReceivedCallback callback, bool serverMode, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput, const UUID& connectionID)	:
    m_connectionID(connectionID)
{
    if (callback == nullptr)
        throw invalid_argument("MiddlewareEngine BufferReceivedCallback is null");

    m_bufferReceivedCallback = callback;
    m_serverMode = serverMode;
    m_securityEnabled = securityEnabled;
    m_inactiveClientTimeout = inactiveClientTimeout;
    m_verboseOutput = verboseOutput;
}

MiddlewareEngine::~MiddlewareEngine()
{
}

void MiddlewareEngine::OnBufferReceived(const UUID& connectionID, const Buffer& buffer)
{
    m_bufferReceivedCallback(connectionID, buffer);
}

bool MiddlewareEngine::ServerMode() const
{
    return m_serverMode;
}

bool MiddlewareEngine::SecurityEnabled() const
{
    return m_securityEnabled;
}

int MiddlewareEngine::InactiveClientTimeout() const
{
    return m_inactiveClientTimeout;
}

bool MiddlewareEngine::VerboseOutput() const
{
    return m_verboseOutput;
}

const UUID& MiddlewareEngine::ConnectionID() const
{
    return m_connectionID;
}