//******************************************************************************************************
//  ZeroMQServer.cpp - Gbtc
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

#include "ZeroMQServer.h"

#include <string.h>
#include <sys/eventfd.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "zproxycert.h"
#include "../ConsoleLogger/ConsoleLogger.h"

#define DATAPROXYENDPOINT "inproc://dataproxy"
#define MAXTHREADPOOLSIZE 20

// See the following link for a ZeroMQ asynchronous client/server example in C:
// http://zguide.zeromq.org/c:asyncsrv

ZeroMQServer::ZeroMQServer(BufferReceivedCallback callback, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput, const UUID& connectionID, zcert_t* localCertificate) : 
    MiddlewareEngine(callback, true, securityEnabled, inactiveClientTimeout, verboseOutput, connectionID),
    m_activeClientTimer(&ActiveClientTimerElapsed, this)
{
    m_proxy = nullptr;
    m_authenticator = nullptr;
    m_monitor = nullptr;
    m_localCertificate = localCertificate;
    m_checkActiveClients = false;
    m_enabled = false;
    m_disposed = false;

    // Create a new file descriptor for notification of data arrival
    m_fileDescriptor = eventfd(0, 0);

    if (m_fileDescriptor == -1)
        throw new runtime_error("Failed to create file descriptor for ZeroMQEngine: " + string(strerror(errno)));

    // Create a new send buffer queue
    m_sendBuffers = new vector<pair<UUID, Buffer>>();
}

ZeroMQServer::~ZeroMQServer()
{
    if (!m_disposed)
        Dispose();
}
    
void ZeroMQServer::Initialize(const string endPoint, const string /*instanceName*/, const string /*instanceID*/)
{
    // Setup server authentication handler, if security is enabled
    if (SecurityEnabled())
    {
        m_authenticator = zactor_new(zauth, nullptr);
    
        if (m_authenticator == nullptr)
            throw runtime_error("Failed to create client authenticator, cannot initialize ZeroMQ server security");
        
        if (VerboseOutput())
        {
            zstr_sendx(m_authenticator, "VERBOSE", nullptr);
            zsock_wait(m_authenticator);
        }
    
        // Authenticator dynamically watches folder for new, deleted or updated public client certificates
        zstr_sendx(m_authenticator, "CURVE", CURVECERTREMOTEDIR, nullptr);
        zsock_wait(m_authenticator);
    }
    
    // Create actor to proxy data from TCP "server" host socket to inter-thread transport socket
    // Note that this is a custom zactor for this project which allows certificate to be applied
    // to frontend proxy, see zproxycert.c
    m_proxy = zactor_new(zproxycert, m_localCertificate);
    
    if (m_proxy == nullptr)
        throw runtime_error("Failed to create ZeroMQEngine server data proxy");
    
    if (VerboseOutput())
    {
        zstr_sendx(m_proxy, "VERBOSE", nullptr);
        zsock_wait(m_proxy);
        
        m_monitor = zactor_new(zmonitor, m_proxy);
        
        if (m_monitor != nullptr)
        {
            zstr_sendx(m_monitor, "VERBOSE", nullptr);
            zstr_sendx(m_monitor, "LISTEN", "ALL", nullptr);
            zstr_sendx(m_monitor, "START", nullptr);
            zsock_wait(m_monitor);
        }
    }
    
    // Establish proxy socket bindings
    zstr_sendx(m_proxy, "FRONTEND", "ROUTER", ("tcp://" + endPoint).c_str(), nullptr);
    zsock_wait(m_proxy);
    
    zstr_sendx(m_proxy, "BACKEND", "DEALER", DATAPROXYENDPOINT, nullptr);
    zsock_wait(m_proxy);
    
    // Establish a client activity monitor
    timespec dueTime, period;
        
    // Monitoring once per minute
    dueTime.tv_sec = InactiveClientTimeout();
    dueTime.tv_nsec = 0;
    period = dueTime;

    // Set interval and start timer
    m_activeClientTimer.Change(dueTime, period);

    m_enabled = true;

    // Create initial data I/O thread
    m_socketPollThreadPool.push_back(new Thread(&SocketPollHandler, this, true, true));
}

void ZeroMQServer::Shutdown()
{
    Dispose();
}    
    
// Send to all clients
bool ZeroMQServer::SendBuffer(const Buffer& buffer)
{
    if (!m_enabled)
        return false;
        
    spinlock(m_clientsLock,
    {
        for (auto enumerator = m_clients.begin(); enumerator != m_clients.end(); ++enumerator)
            SendBuffer(enumerator->first, buffer);
    });
            
    return true;
}
    
// Send to specific client
bool ZeroMQServer::SendBuffer(const UUID& connectionID, const Buffer& buffer)
{
    if (!m_enabled)
        return false;

    if (connectionID.IsEmpty())
        throw invalid_argument("ZeroMQEngine in server mode requires a connection ID for the client");
      
    spinlock(m_sendBuffersLock,
    {
        m_sendBuffers->push_back(pair<UUID, Buffer>(connectionID, Buffer::Copy(buffer)));
        
        // Notify poller that there is now data to be sent (increments FD counter by one)
        if (eventfd_write(m_fileDescriptor, 1) != 0)
            log_error("Failed to increment ZeroMQEngine file descriptor.\n");
    });
                
    return true;
}

void ZeroMQServer::Dispose()
{
    m_enabled = false;

    // Deactivate client activity monitor
    m_activeClientTimer.Stop();
    
    // Close receive sockets
    for (int i = 0; i < m_sockets.size(); i++)
        zsock_destroy(&m_sockets[i]);
    
    m_sockets.clear();
    
    // Close the data proxy
    if (m_proxy)
        zactor_destroy(&m_proxy);
    
    // Shut down the authentication handler
    if (m_authenticator)
        zactor_destroy(&m_authenticator);
    
    // Delete any receive data threads in pool
    for (int i = 0; i < m_socketPollThreadPool.size(); i++)
        delete m_socketPollThreadPool[i];
    
    m_socketPollThreadPool.clear();

    // Stop socket event monitor
    if (m_monitor)
        zactor_destroy(&m_monitor);

    if (m_sendBuffers != nullptr)
    {
        delete m_sendBuffers;
        m_sendBuffers = nullptr;
    }

    if (m_fileDescriptor > -1)
    {
        close(m_fileDescriptor);
        m_fileDescriptor = -1;
    }
    
    if (m_localCertificate)
        zcert_destroy(&m_localCertificate);
    
    m_disposed = true;
}
    
void ZeroMQServer::UpdateClientActivityTime(const UUID& connectionID)
{
    time_t now;

    // Get current time
    time(&now);
        
    spinlock(m_clientsLock,
    {
        auto enumerator = m_clients.find(connectionID);

        if (enumerator == m_clients.end())
        {
            log_info(">> New client connection [%s]\n", connectionID.ToString().c_str());
            m_clients[connectionID] = now;
                
            // Add more socket polling threads to pool if needed - maintain one server worker per client
            // up to maximum thread pool size - data processing will be fairly balanced among threads
            if (m_socketPollThreadPool.size() < MAXTHREADPOOLSIZE && m_clients.size() > m_socketPollThreadPool.size())
                m_socketPollThreadPool.push_back(new Thread(&SocketPollHandler, this, true, true));
        }
        else
        {
            enumerator->second = now;
        }
    });
}

void ZeroMQServer::CheckActiveClients()
{    
    vector<UUID> oldClients;
    UUID connectionID;
    time_t lastActivity, now;
    string connectedClientsDump = "\nCurrent Client Connections:\n\n";
    double inactiveTime;
    int index = 0;
    bool hasItems = false;

    // Get current time
    time(&now);

    spinlock(m_clientsLock,
    {
        // Check for inactive client connections
        for (auto enumerator = m_clients.begin(); enumerator != m_clients.end(); ++enumerator)
        {
            connectionID = enumerator->first;
            lastActivity = enumerator->second;
            inactiveTime = difftime(now, lastActivity);

            // Add entry to string
            if (VerboseOutput())
            {                
                stringstream fmtstr;                
                
                // Pad index with zeros
                fmtstr << setfill('0') << setw(3) << to_string(index);
                
                connectedClientsDump += 
                    "[" + fmtstr.str() + "] = "
                    "{" + connectionID.ToString() + "}";
                
                if (inactiveTime > 0)
                    connectedClientsDump += 
                        " inactive for " + 
                        Timer::ToElapsedTimeString(inactiveTime);
                
                connectedClientsDump += "\n";
            }

            // Mark any clients for removal that have not been active in the last minute
            if (inactiveTime > InactiveClientTimeout())
                oldClients.push_back(connectionID);
    
            hasItems = true;
        }

        if (hasItems && VerboseOutput())
            log_info(connectedClientsDump.c_str());

        // Remove expired clients
        if (oldClients.size() > 0)
        {
            log_info("Removing %d expired clients from client table...\n", oldClients.size());
        
            for (int i = 0; i < oldClients.size(); i++)
                m_clients.erase(oldClients[i]);
        }               
    });
}

// Handle data reception from inter-thread transport
void ZeroMQServer::ReceiveNextBuffer(zsock_t* socket)
{
    zmsg_t* message;
    zframe_t* frame;
    UUID connectionID;
    
    message = zmsg_recv(socket);

    if (message == nullptr)
        return;

    // Router socket should provide identity, delimiter and data payload frames
    if (zmsg_size(message) == 3)
    {
        // Extract client identity (any client suffix, e.g., 'S', will be ignored)
        frame = zmsg_pop(message);
        connectionID = UUID(*reinterpret_cast<uuid_t*>(zframe_data(frame)));
        zframe_destroy(&frame);
            
        // Skip delimiter frame
        frame = zmsg_pop(message);
        zframe_destroy(&frame);

        // Get the data payload frame
        frame = zmsg_pop(message);

        // Execute buffer received call back 
        OnBufferReceived(connectionID, Buffer(reinterpret_cast<u_int8_t*>(zframe_data(frame)), zframe_size(frame)));

        // Free the received frame
        zframe_destroy(&frame);

        // Check for new client and update last client activity time
        UpdateClientActivityTime(connectionID);
            
        // When timer fires, check active clients
        if (m_checkActiveClients)
        {
            m_checkActiveClients = false;
            CheckActiveClients();
        }
    }
        
    // Free the received message
    zmsg_destroy(&message);
}

void ZeroMQServer::SendQueuedBuffers(zsock_t* socket)
{
    vector<pair<UUID, Buffer>>* workingBuffers = nullptr;
    zmsg_t* message;
    zframe_t* frame;
    UUID connectionID;
    eventfd_t count;
        
    // Retrieve any queued buffers to send - keep lock time to a minimum since data
    // could be queued to send asynchronously at any time on a different thread. We
    // only attempt to lock, instead of waiting, since many threads may be running
    // and attempting to handle this notification.
    tryspinlock(m_sendBuffersLock,
    {
        workingBuffers = m_sendBuffers;
        m_sendBuffers = new vector<pair<UUID, Buffer>>();

        // Reset file descriptor counter (checking buffer size to prevent a blocking read)
        if (workingBuffers->size() > 0 && eventfd_read(m_fileDescriptor, &count) != 0)
            log_error("Failed to reset ZeroMQEngine file descriptor.\n");        
    });
        
    if (workingBuffers)
    {
        for (int i = 0; i < workingBuffers->size() && m_enabled; i++)
        {
            pair<UUID, Buffer>& connectionBuffer = workingBuffers->at(i);
            connectionID = connectionBuffer.first;
            Buffer buffer = connectionBuffer.second;
                
            message = zmsg_new();
        
            // Define identity for client reception thread, e.g., 'R', so ROUTER can identify return path
            char identity[17];
            memcpy(identity, connectionID.value, 16);
            identity[16] = 'R';

            // Add identity, delimiter and data payload frames
            frame = zframe_new(identity, 17);
            zmsg_append(message, &frame);

            frame = zframe_new(nullptr, 0);
            zmsg_append(message, &frame);

            frame = zframe_new(buffer.value, buffer.length);
            zmsg_append(message, &frame);

            // Send message
            zmsg_send(&message, socket);

            // Check for new client and update last client activity time
            UpdateClientActivityTime(connectionID);
                
            deletebuffer(buffer);
        }

        delete workingBuffers;
    }
}

void ZeroMQServer::SocketPollHandler(void* arg)
{
    ZeroMQServer* server = static_cast<ZeroMQServer*>(arg);

    if (server == nullptr)
        throw invalid_argument("ZeroMQServer::SocketPollHandler ZeroMQServer instance is null");

    // Create receiving socket that connects through inter-thread transport
    zsock_t* socket = zsock_new_dealer(nullptr);
                                
    // Connect inter-thread receiver socket
    if (socket == nullptr || zsock_connect(socket, DATAPROXYENDPOINT) == -1)
        throw runtime_error("Failed to create ZeroMQEngine inter-thread receiving socket");

    // Track receive sockets so they can be closed outside of this thread
    server->m_sockets.push_back(socket);
    
    zmq_pollitem_t fds[2];

    // Setup file descriptor for socket - this descriptor
    // will notify when there is data to be read
    fds[0].socket = zsock_resolve(socket);
    fds[0].events = ZMQ_POLLIN;
        
    // Setup file descriptor for engine - this descriptor
    // will notify when there is data to be sent
    fds[1].socket = nullptr;
    fds[1].fd = server->m_fileDescriptor;
    fds[1].events = ZMQ_POLLIN;
    
    while (server->m_enabled)
    {
        int result = zmq_poll(fds, 2, 250);
        
        if (result > 0)
        {
            if (fds[0].revents & ZMQ_POLLIN)
                server->ReceiveNextBuffer(socket);
            
            if (fds[1].revents & ZMQ_POLLIN)
                server->SendQueuedBuffers(socket);
        }
        else
        {
            if (errno != EAGAIN && errno != EINTR && errno != ETERM && errno != ENOTSOCK)
                log_error("Error during ZeroMQEngine poll: %s\n", strerror(errno));
        }                
    }
}

void ZeroMQServer::ActiveClientTimerElapsed(void* state)
{
    ZeroMQServer* server = static_cast<ZeroMQServer*>(state);

    if (server == nullptr)
        throw invalid_argument("ZeroMQServer::ActiveClientTimerElapsed ZeroMQServer instance is null");

    server->m_checkActiveClients = true;
}
