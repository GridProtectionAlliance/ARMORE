//******************************************************************************************************
//  PacketProcessor.h - Gbtc
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
//  06/15/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#ifndef _PACKET_MANAGER
#define _PACKET_MANAGER

#include "UUID.h"
#include "Buffer.h"
#include "MacAddress.h"
#include "IPAddress.h"
#include "Timer.h"
#include "SpinLock.h"
#include "CaptureEngine/CaptureEngine.h"
#include "MiddlewareEngine/MiddlewareEngine.h"
#include <time.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <unordered_map>
#include <vector>
#include <tuple>

using namespace std;

class PacketProcessor
{
public:
    // Constructors
    PacketProcessor(CaptureEngine* captureEngine, MiddlewareEngine* middlewareEngine, bool transparentOperation, bool permissiveMode, IPAddress ipv4Netmask, int inactiveIPTimeout, int macResolutionTimeout, bool verboseOutput, long feedbackInterval);
    ~PacketProcessor();

    // Methods

    // Gets a file descriptor for the packet processor for polling use (signals when data has arrived)
    int GetFileDescriptor() const;

    // Gets count of queued buffers
    u_int32_t GetQueuedBufferCount();
    
    // Queue buffer received from middleware engine for processing when the captured
    // local interface reports ready, i.e., POLLOUT
    void QueueReceivedBuffer(const UUID& connectionID, const Buffer& buffer);

    // Encode captured packets and send via middleware, i.e., take packets captured from
    // the local interface and send through the middleware engine
    void EncodePackets();   // captured interface => middleware
    
    // Decode queued packets received from middleware, i.e., take packets received from
    // the middleware engine and inject onto the captured local interface
    void DecodePackets();   // middleware => captured interface

private:
    // Handle individual packets
    void EncodePacket(Buffer& packet);
    void DecodePacket(pair<UUID, Buffer>& connectionBuffer);
    
    // IPv4 packet handling
    bool ProcessIPv4Packet(const UUID& sourceConnectionID, ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID, IPAddress* destinationIP);
    bool ProcessARPPacket(const UUID& sourceConnectionID, ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID, IPAddress* destinationIP);
    Buffer CreateARPRequestPacket(const ip* ipmsg);
    
    // IPv6 packet handling
    bool ProcessIPv6Packet(const UUID& sourceConnectionID, ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID, IPAddress* destinationIP);
    bool ProcessNDPPacket(ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID);
    Buffer CreateNDPRequestPacket(const ip6_hdr* ipmsg);
    
    // Inject packet onto local interface
    void InjectPacket(const Buffer& packet);
    
    // Send packet via middleware
    void SendPacket(const IPAddress& sourceIP, const u_int8_t* sourceMac, const IPAddress& destinationIP, const Buffer& packet, const ip* ipmsg);
    void SendPacket(const IPAddress& sourceIP, const u_int8_t* sourceMac, const IPAddress& destinationIP, const Buffer& packet, const ip6_hdr* ipmsg);
    bool SendPacket(const IPAddress& destinationIP, const Buffer& packet);
    void SendMacRequestPacket(const IPAddress& destinationIP, const Buffer& requestPacket);
        
    // Update IP tables
    bool UpdateIPTable(const IPAddress& ip, const u_int8_t* mac, const UUID& connectionID = UUID::Empty);
        
    // Lookup IP and return associated MAC address and/or connection ID
    bool TryIPTableLookup(const IPAddress& ip, UUID* connectionID);
    bool TryIPTableLookup(const IPAddress& ip, MacAddress* mac);
    bool TryIPTableLookup(const IPAddress& ip, MacAddress* mac, UUID* connectionID);

    // Check if a pending MAC address resolution request exists for IP
    bool PendingIPMacRequestExists(const IPAddress& ip);
    
    void CheckActiveIPAddresses();
    void CheckPendingIPMacRequests();

    // Static    
    static void IPActivityTimerElapsed(void* state);    
    static void PendingIPMacRequestsTimerElasped(void* state);
    
    // Fields
    bool m_verboseOutput;
    long m_feedbackInterval;
    long m_packetsSent;
    long m_packetsReceived;
    long m_packetsInjected;
    bool m_transparentOperation;
    bool m_permissiveMode;
    IPAddress m_ipv4Netmask;
    int m_inactiveIPTimeout;
    int m_macResolutionTimeout;
    int m_fileDescriptor;
    volatile int m_processingState;
    volatile int m_queuedBufferCount;
    CaptureEngine* m_captureEngine;
    MiddlewareEngine* m_middlewareEngine;
    vector<pair<UUID, Buffer>>* m_receivedBuffers;
    SpinLock m_receivedBuffersLock;
    vector<Buffer> m_injectionQueue;
    unordered_map<IPAddress, tuple<MacAddress, UUID, time_t>> m_ipTable;
    unordered_map<IPAddress, time_t> m_pendingIPMacRequests;
    volatile bool m_checkIPActivity;
    volatile bool m_checkPendingIPMacRequests;
    Timer m_ipActivityTimer;
    Timer m_pendingIPMacRequestsTimer;
};

#endif