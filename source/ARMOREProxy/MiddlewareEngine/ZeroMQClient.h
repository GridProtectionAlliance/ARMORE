//******************************************************************************************************
//  ZeroMQClient.h - Gbtc
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
//  09/09/2015 - Ritchie
//       Generated original version of source code.
//
//******************************************************************************************************

#ifndef _ZEROMQCLIENT_H
#define _ZEROMQCLIENT_H

#include "MiddlewareEngine.h"

#include <czmq.h>
#include <string>

#include "../Thread.h"

using namespace std;

// Represents a fully asynchronous ZeroMQ client style middleware engine implementation
class ZeroMQClient : public MiddlewareEngine
{
public:
    // Constructors
    explicit ZeroMQClient(BufferReceivedCallback callback, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput, const UUID& connectionID, zcert_t* localCertificate);
    ~ZeroMQClient();
    
    // Methods
    void Initialize(const string endPoint, const string instanceName, const string instanceID) override;
    void Shutdown() override;   
    bool SendBuffer(const Buffer& buffer) override;    
    bool SendBuffer(const UUID& /*connectionID*/, const Buffer& buffer) override;

private:
    void Dispose();
    zsock_t* GetConnectedSocket(char mode);

    static void ReceiveDataHandler(void* arg);

    // Fields
    string m_endPoint;
    zsock_t* m_sendSocket;          // TCP dealer client-like socket - send only
    zsock_t* m_receiveSocket;       // TCP dealer client-like socket - receive only
    zactor_t* m_monitor;            // Socket event monitor
    zcert_t* m_localCertificate;    // Complete local certificate used for curve security
    zcert_t* m_serverCertificate;   // Public server certificate used for curve security
    Thread* m_receiveDataThread;
    volatile bool m_enabled;
    bool m_disposed;
};

#endif