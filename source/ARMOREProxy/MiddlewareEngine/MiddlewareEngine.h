//******************************************************************************************************
//  MiddlewareEngine.h - Gbtc
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

#ifndef _MIDDLEWAREENGINE_H
#define _MIDDLEWAREENGINE_H

#include "../UUID.h"
#include "../Buffer.h"
#include <string>

typedef void(*BufferReceivedCallback)(const UUID& connectionID, const Buffer& buffer);

enum class MiddlewareEngineOption : int
{
    ZeroMQ,
    DDS,
    Default = ZeroMQ
};

class MiddlewareEngine
{
public:
    // Constructors
    MiddlewareEngine(BufferReceivedCallback callback, bool serverMode, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput);
    MiddlewareEngine(BufferReceivedCallback callback, bool serverMode, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput, const UUID& connectionID);
    virtual ~MiddlewareEngine();

    // Methods

    // Initialize middleware engine
    virtual void Initialize(const string endPoint, const string instanceName, const string instanceID) = 0;

    // Shutdown middleware engine
    virtual void Shutdown() = 0;

    // Send buffer to all clients in server mode else just to server in client mode
    virtual bool SendBuffer(const Buffer& buffer) = 0;

    // Send buffer to specific client in server mode else just to server in client mode
    virtual bool SendBuffer(const UUID& connectionID, const Buffer& buffer) = 0;

    // Invoke buffer received callback
    virtual void OnBufferReceived(const UUID& connectionID, const Buffer& buffer);

    // Gets flag that determines if middleware engine is in server mode
    bool ServerMode() const;

    // Gets flag that determines if middleware engine has security enabled
    bool SecurityEnabled() const;
    
    // Gets inactive client timeout, in seconds
    int InactiveClientTimeout() const;

    // Gets flag that determines if middleware engine should produce verbose output
    bool VerboseOutput() const;
    
    // Gets the connection ID for the middleware engine
    const UUID& ConnectionID() const;

private:
    // Fields
    BufferReceivedCallback m_bufferReceivedCallback;
    bool m_serverMode;
    bool m_securityEnabled;
    int m_inactiveClientTimeout;
    bool m_verboseOutput;
    const UUID m_connectionID;
};

#endif