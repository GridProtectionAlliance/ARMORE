//******************************************************************************************************
//  ZeroMQClient.cpp - Gbtc
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

#include "ZeroMQClient.h"
#include "ZeroMQServer.h"

#include <stdexcept>

// See the following link for a ZeroMQ asynchronous client/server example in C:
// http://zguide.zeromq.org/c:asyncsrv

ZeroMQClient::ZeroMQClient(BufferReceivedCallback callback, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput, const UUID& connectionID, zcert_t* localCertificate) : 
    MiddlewareEngine(callback, false, securityEnabled, inactiveClientTimeout, verboseOutput, connectionID)
{
    m_sendSocket = nullptr;
    m_receiveSocket = nullptr;
    m_monitor = nullptr;
    m_localCertificate = localCertificate;
    m_serverCertificate = nullptr;    
    m_receiveDataThread = nullptr;
    m_enabled = false;
    m_disposed = false;
}

ZeroMQClient::~ZeroMQClient()
{
    if (!m_disposed)
        Dispose();
}

void ZeroMQClient::Initialize(const string endPoint, const string /*instanceName*/, const string /*instanceID*/)
{
    if (SecurityEnabled())
    {
        string serverCertFileName = CURVECERTREMOTEDIR SVRPUBCERTFILENAME;
    
        // Load public certificate for server
        if (!zsys_file_exists(serverCertFileName.c_str()))
            throw runtime_error("Public curve certificate for server \"" + serverCertFileName + "\" does not exist, cannot initialize ZeroMQ client security");
            
        m_serverCertificate = zcert_load(serverCertFileName.c_str());
            
        if (m_serverCertificate == nullptr)
            throw runtime_error("Failed to load public curve certificate for server \"" + serverCertFileName + "\", cannot initialize ZeroMQ client security");
    }
    
    m_endPoint = endPoint;
    m_sendSocket = GetConnectedSocket('S');
    m_enabled = true;

    // Create data reception thread
    m_receiveDataThread = new Thread(&ReceiveDataHandler, this, true, true);
}

void ZeroMQClient::Shutdown()
{
    Dispose();
}
    
bool ZeroMQClient::SendBuffer(const Buffer& buffer)
{
    return SendBuffer(UUID::Empty, buffer);
}
    
bool ZeroMQClient::SendBuffer(const UUID& /*connectionID*/, const Buffer& buffer)
{
    if (!m_enabled)
        return false;
                
    zmsg_t* message = zmsg_new();
    zframe_t* frame;
    
    // Handle "client" mode, i.e., a zmq DEALER socket
    frame = zframe_new(nullptr, 0);
    zmsg_append(message, &frame);

    frame = zframe_new(buffer.value, buffer.length);
    zmsg_append(message, &frame);

    // Function should not be called from multiple threads, ZeroMQ API is not guaranteed to be thread-safe
    zmsg_send(&message, m_sendSocket);
    
    return true;
}

void ZeroMQClient::Dispose()
{
    m_enabled = false;

    if (m_receiveDataThread)
    {
        delete m_receiveDataThread;
        m_receiveDataThread = nullptr;
    }

    // Close send socket
    if (m_sendSocket)
        zsock_destroy(&m_sendSocket);

    // Close receive socket
    if (m_receiveSocket)
        zsock_destroy(&m_receiveSocket);

    // Stop socket event monitor
    if (m_monitor)
        zactor_destroy(&m_monitor);
    
    if (m_localCertificate)
        zcert_destroy(&m_localCertificate);
    
    if (m_serverCertificate)
        zcert_destroy(&m_serverCertificate);
    
    m_disposed = true;
}

zsock_t* ZeroMQClient::GetConnectedSocket(char mode)
{
    // Establish "client" socket, i.e., a zmq DEALER socket
    zsock_t* socket = zsock_new_dealer(nullptr);
        
    if (socket == nullptr)
        throw runtime_error("Failed to create ZeroMQEngine dealer socket");
    
    // Set socket identity
    char identity[17];
    
    // Copy in client identification
    memcpy(identity, ConnectionID().value, 16);
    
    // Suffix ID with mode, e.g., send 'S' or receive 'R', to handle multiple clients with same root ID.
    // Two clients cannot connect to ROUTER socket with the same ID and you can only safely use a socket
    // from a single thread, i.e., you cannot read and write at the same time with a single ZMQ socket.
    // To resolve we create one connection for sending data and another for receiving data and suffix a
    // common ID with an extra character to make the connection ID unique yet still be able to send and
    // receive from a ROUTER socket to a single client "instance" that has two client connections.
    identity[16] = mode;
    
    // Assign unique identity to socket
    zmq_setsockopt(zsock_resolve(socket), ZMQ_IDENTITY, identity, 17);
    
    if (SecurityEnabled())
    {
        // Apply local full certificate (public + private) to client socket
        zcert_apply(m_localCertificate, socket);
        
        // Assign public certificate for server to client socket
        zsock_set_curve_serverkey_bin(socket, zcert_public_key(m_serverCertificate));
    }
    
    // Attach event monitoring to first socket
    if (VerboseOutput() && m_monitor == nullptr)
    {
        m_monitor = zactor_new(zmonitor, socket);
        
        if (m_monitor != nullptr)
        {
            zstr_sendx(m_monitor, "VERBOSE", nullptr);
            zstr_sendx(m_monitor, "LISTEN", "ALL", nullptr);
            zstr_sendx(m_monitor, "START", nullptr);
            zsock_wait(m_monitor);            
        }
    }
    
    // Connect to endpoint
    if (zsock_connect(socket, "tcp://%s", m_endPoint.c_str()) == -1)
        throw runtime_error("Failed to connect ZeroMQEngine dealer socket - check endpoint");
    
    return socket;
}

void ZeroMQClient::ReceiveDataHandler(void* arg)
{
    ZeroMQClient* client = static_cast<ZeroMQClient*>(arg);

    if (client == nullptr)
        throw invalid_argument("ZeroMQClient::ReceiveDataHandler ZeroMQClient instance is null");

    // Create a ZeroMQ receive socket for this thread - separate
    // socket allows safe send and receive from different threads
    client->m_receiveSocket = client->GetConnectedSocket('R');
    zmsg_t* message;
    zframe_t* frame;

    while (client->m_enabled)
    {
        // Handle "client" mode, i.e., a zmq DEALER socket
        message = zmsg_recv(client->m_receiveSocket);

        if (message == nullptr)
            continue;
        
        // Dealer socket should have removed identity frame already,
        // should be left with delimiter and data payload frames
        if (zmsg_size(message) == 2)
        {
            // Skip delimiter frame
            frame = zmsg_pop(message);
            zframe_destroy(&frame);

            // Get the data payload frame
            frame = zmsg_pop(message);

            // Execute buffer received call back 
            client->OnBufferReceived(client->ConnectionID(), Buffer(reinterpret_cast<u_int8_t*>(zframe_data(frame)), zframe_size(frame)));

            // Free the received frame
            zframe_destroy(&frame);
        }

        // Free the received message
        zmsg_destroy(&message);
    }
}
