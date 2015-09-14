//******************************************************************************************************
//  ZeroMQServer.h - Gbtc
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

#ifndef _ZEROMQSERVER_H
#define _ZEROMQSERVER_H

#include "MiddlewareEngine.h"

#include <czmq.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Thread.h"
#include "../Timer.h"
#include "../SpinLock.h"

#define CURVECERTSTORENAME ".curve"
#define PUBCERTFILENAMEEXT ".pub"
#define PVTCERTFILENAMEEXT ".secret"
#define CURVECERTLOCALDIR  CURVECERTSTORENAME "/local"
#define CURVECERTREMOTEDIR CURVECERTSTORENAME "/remote"
#define SVRPUBCERTFILENAME "/server" PUBCERTFILENAMEEXT
#define SVRPVTCERTFILENAME "/server" PVTCERTFILENAMEEXT

using namespace std;

// Represents a fully asynchronous ZeroMQ server style middleware engine implementation
class ZeroMQServer : public MiddlewareEngine
{
public:
    // Constructors
    explicit ZeroMQServer(BufferReceivedCallback callback, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput, const UUID& connectionID, zcert_t* localCertificate);
    ~ZeroMQServer();
    
    // Methods
    void Initialize(const string endPoint, const string instanceName, const string instanceID) override;
    void Shutdown() override;
    bool SendBuffer(const Buffer& buffer) override;
    bool SendBuffer(const UUID& connectionID, const Buffer& buffer) override;

private:
    void Dispose();    
    void UpdateClientActivityTime(const UUID& connectionID);
    void CheckActiveClients();
    void ReceiveNextBuffer(zsock_t* socket);
    void SendQueuedBuffers(zsock_t* socket);

    static void SocketPollHandler(void* arg);
    static void ActiveClientTimerElapsed(void* state);

    // Fields
    int m_fileDescriptor;
    zactor_t* m_proxy;                  // Host for TCP to inproc data routing proxy for thread-safe internal I/O
    zactor_t* m_authenticator;          // Authentication handler for all zmq client instances
    zactor_t* m_monitor;                // Socket event monitor
    zcert_t* m_localCertificate;        // Local certificate used for curve security
    vector<zsock_t*> m_sockets;         // inproc dealer client-like sockets
    vector<Thread*> m_socketPollThreadPool;
    vector<pair<UUID, Buffer>>* m_sendBuffers;
    SpinLock m_sendBuffersLock;
    unordered_map<UUID, time_t> m_clients;
    SpinLock m_clientsLock;
    volatile bool m_checkActiveClients;
    Timer m_activeClientTimer;
    volatile bool m_enabled;
    bool m_disposed;
};

#endif