//******************************************************************************************************
//  PacketProcessor.cpp - Gbtc
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

#include "PacketProcessor.h"
#include "ConsoleLogger/ConsoleLogger.h"
#include "TraceFunctions/TraceFunctions.h"
#include <string.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <netinet/icmp6.h>
#include <stdexcept>

#define PROCESSING_INCOMING 0
#define PROCESSING_OUTGOING 1

static u_int16_t IPv6CheckSum(ip6_hdr* header, u_int16_t* data, int length);

PacketProcessor::PacketProcessor(CaptureEngine* captureEngine, MiddlewareEngine* middlewareEngine, bool transparentOperation, bool permissiveMode, IPAddress ipv4Netmask, int inactiveIPTimeout, int macResolutionTimeout, bool verboseOutput, long feedbackInterval)
    : m_ipActivityTimer(&IPActivityTimerElapsed, this), m_pendingIPMacRequestsTimer(&PendingIPMacRequestsTimerElasped, this)
{
    m_captureEngine = captureEngine;
    m_middlewareEngine = middlewareEngine;
    m_transparentOperation = transparentOperation;
    m_permissiveMode = permissiveMode;
    m_ipv4Netmask = ipv4Netmask;
    m_inactiveIPTimeout = inactiveIPTimeout;
    m_macResolutionTimeout = macResolutionTimeout;
    m_verboseOutput = verboseOutput;
    m_feedbackInterval = feedbackInterval;
    m_packetsSent = 0;
    m_packetsReceived = 0;
    m_packetsInjected = 0;
    m_processingState = -1;
    m_checkIPActivity = false;
    m_checkPendingIPMacRequests = false;

    // Create a new file descriptor for notification of data arrival
    m_fileDescriptor = eventfd(0, 0);

    if (m_fileDescriptor == -1)
        throw new runtime_error("Failed to create file descriptor for Packet Processor: " + string(strerror(errno)));

    // Default to netmask on captured interface if one was not provided
    if (*reinterpret_cast<const u_int32_t*>(m_ipv4Netmask.GetValue()) == 0)
        m_ipv4Netmask = m_captureEngine->LocalIPv4NetMask;

    // Create a new received buffer queue
    m_receivedBuffers = new vector<pair<UUID, Buffer>>();

    timespec dueTime, period;

    // Establish IP activity timer
    dueTime.tv_sec = m_inactiveIPTimeout;
    dueTime.tv_nsec = 0;
    period = dueTime;

    m_ipActivityTimer.Change(dueTime, period);

    // Establish pending MAC request timer
    dueTime.tv_sec = m_macResolutionTimeout;
    dueTime.tv_nsec = 0;
    period = dueTime;

    m_pendingIPMacRequestsTimer.Change(dueTime, period);
}

PacketProcessor::~PacketProcessor()
{
    if (m_receivedBuffers != nullptr)
        delete m_receivedBuffers;

    if (m_fileDescriptor > -1)
        close(m_fileDescriptor);
}

// This file descriptor is used to signal when there is queued data available that has been
// received from the middleware engine that is ready to be injected onto the captured interface.
int PacketProcessor::GetFileDescriptor() const
{
    return m_fileDescriptor;
}

// This function returns a count that is used to indicate that data is queued such that a
// POLLOUT can be processed on the capture interface.
u_int32_t PacketProcessor::GetQueuedBufferCount()
{
    return m_queuedBufferCount;
}

// This function will be called asynchronously on middleware receive thread when data arrives
void PacketProcessor::QueueReceivedBuffer(const UUID& connectionID, const Buffer& buffer)
{   
    if (m_verboseOutput && m_packetsReceived % m_feedbackInterval == 0)
        DescribePacket(buffer.value, "RxMW->TxCI");

    spinlock(m_receivedBuffersLock,
    {
        // Add a copy of received middleware buffer to queue for processing
        m_receivedBuffers->push_back(pair<UUID, Buffer>(connectionID, Buffer::Copy(buffer))) ;
        m_queuedBufferCount++ ;

        // Notify poller that there is now data to be decoded (increments FD counter by one)
        if (eventfd_write(m_fileDescriptor, 1) != 0)
            log_error("Failed to increment Packet Processor file descriptor.\n") ;
    });

    if (++m_packetsReceived % m_feedbackInterval == 0)
        log_info("Received %d packets so far...\n", m_packetsReceived);
}

void PacketProcessor::EncodePackets()
{
    // Update processing state to inbound
    m_processingState = PROCESSING_INCOMING;

    // Poll event signaled capture interface has data, read all
    // data from the interface and send via middleware
    Buffer packet = m_captureEngine->GetNextPacket();

    while (!packet.IsEmpty())
    {
        EncodePacket(packet);
        packet = m_captureEngine->GetNextPacket();
    }
    
    // Check IP table address expirations
    if (m_checkIPActivity)
    {
        m_checkIPActivity = false;        
        CheckActiveIPAddresses();
    }
    
    // Check IP based pending MAC requests
    if (m_checkPendingIPMacRequests)
    {
        m_checkPendingIPMacRequests = false;        
        CheckPendingIPMacRequests();
    }
}

void PacketProcessor::EncodePacket(Buffer& packet)
{
    ether_header* ethhdr;
    u_int16_t type;
    ip* ipv4msg;
    ether_arp* arpmsg;
    ip6_hdr* ipv6msg;
    in_addr sourceIPv4, destinationIPv4;
    in6_addr sourceIPv6, destinationIPv6;

    if (packet.length < sizeof(ether_header))
    {
        log_error("Error processing Ethernet packet: packet is too small, %d of %d bytes.\n", packet.length, sizeof(ether_header));
        return;
    }

    // Hold on to original packet pointer and length
    Buffer sourcePacket(packet);
    
    // Reference Ethernet header in frame data
    ethhdr = reinterpret_cast<ether_header*>(packet.value);
    type = ntohs(ethhdr->ether_type);
    
    // Skip past Ethernet header
    packet.value += sizeof(ether_header);
    packet.length -= sizeof(ether_header);
        
    switch (type)
    {
        case ETHERTYPE_ARP:
            // Get ARP message
            arpmsg = reinterpret_cast<ether_arp*>(packet.value);
            sourceIPv4 = *reinterpret_cast<in_addr*>(arpmsg->arp_spa);
            destinationIPv4 = *reinterpret_cast<in_addr*>(arpmsg->arp_tpa);
            SendPacket(sourceIPv4, ethhdr->ether_shost, destinationIPv4, sourcePacket, static_cast<ip*>(nullptr));
            break;
        case ETHERTYPE_IP:
            // Get IPv4 message
            ipv4msg = reinterpret_cast<ip*>(packet.value);
            sourceIPv4 = ipv4msg->ip_src;
            destinationIPv4 = ipv4msg->ip_dst;
            SendPacket(sourceIPv4, ethhdr->ether_shost, destinationIPv4, sourcePacket, ipv4msg);
            break;
        case ETHERTYPE_IPV6:
            // Get IPv6 message
            ipv6msg = reinterpret_cast<ip6_hdr*>(packet.value);
            sourceIPv6 = ipv6msg->ip6_src;
            destinationIPv6 = ipv6msg->ip6_dst;
            SendPacket(sourceIPv6, ethhdr->ether_shost, destinationIPv6, sourcePacket, ipv6msg);
            break;
        default:
            if (m_verboseOutput)
                log_info("Uncommon Ethernet packet type 0x%02X dropped.\n", type);
            break;
    }
}

void PacketProcessor::DecodePackets()
{
    // Update processing state to outbound
    m_processingState = PROCESSING_OUTGOING;

    // Poll event signaled data has arrived, inject any queued buffers and those
    // received from middleware engine onto captured interface
    vector<pair<UUID, Buffer>>* workingBuffers;
    eventfd_t count;

    // Handle any queued injections
    for (int i = 0; i < m_injectionQueue.size(); i++)
    {
        Buffer& packet = m_injectionQueue[i];
        InjectPacket(packet);
        deletebuffer(packet);
    }

    m_injectionQueue.clear();

    // Retrieve any queued middleware buffers - keep lock time to a minimum since
    // data is always arriving asynchronously on a different thread
    spinlock(m_receivedBuffersLock,
    {
        workingBuffers = m_receivedBuffers ;
        m_receivedBuffers = new vector<pair<UUID, Buffer>>() ;
        m_queuedBufferCount = 0 ;

        // Reset file descriptor counter (checking buffer size to prevent a blocking read)
        if (workingBuffers->size() > 0 && eventfd_read(m_fileDescriptor, &count) != 0)
            log_error("Failed to reset Packet Processor file descriptor.\n") ;
    });

    // Process each queued buffer
    for (int i = 0; i < workingBuffers->size(); i++)
    {
        pair<UUID, Buffer>& connectionBuffer = workingBuffers->at(i);
        DecodePacket(connectionBuffer);
        deletebuffer(connectionBuffer.second);
    }

    delete workingBuffers;
}

void PacketProcessor::DecodePacket(pair<UUID, Buffer>& connectionBuffer)
{
    UUID sourceConnectionID = connectionBuffer.first;
    Buffer packet = connectionBuffer.second;
    UUID destinationConnectionID;
    IPAddress destinationIP;
    bool injectPacket;
    ether_header* ethhdr;
    u_int16_t type;

    if (packet.length < sizeof(ether_header))
    {
        log_error("Error processing Ethernet packet: packet is too small, %d of %d bytes.\n", packet.length, sizeof(ether_header));
        return;
    }

    // Hold on to original packet pointer and length
    Buffer sourcePacket(packet);
    
    // Reference Ethernet header in frame data
    ethhdr = reinterpret_cast<ether_header*>(packet.value);
    type = ntohs(ethhdr->ether_type);
    
    if (!m_transparentOperation)
    {
        // Set packet source MAC to be local interface before transmission
        m_captureEngine->LocalMACAddress.CopyTo(ethhdr->ether_shost);
    }
    
    // Skip past Ethernet header
    packet.value += sizeof(ether_header);
    packet.length -= sizeof(ether_header);

    // Skip ahead if we captured VLAN headers
    while (type == ETHERTYPE_VLAN)
    {
        //u_int16_t vlanTag = ntohs(*reinterpret_cast<u_int16_t *>(packet.value)) & 0xfff;
        packet.value += 4;
        packet.length -= 4;
        type = ntohs(*reinterpret_cast<u_int16_t*>(packet.value - 2));
    }
    
    // Handle multi-network MAC address translation
    switch (type)
    {
        case ETHERTYPE_ARP:
            // Handle incoming ARP messages
            injectPacket = ProcessARPPacket(sourceConnectionID, ethhdr, packet, &destinationConnectionID, &destinationIP);
            break;
        case ETHERTYPE_IP:
            // Handle local IPv4 address translation and ARP requests
            injectPacket = ProcessIPv4Packet(sourceConnectionID, ethhdr, packet, &destinationConnectionID, &destinationIP);
            break;
        case ETHERTYPE_IPV6:
            // Handle local IPv6 address translation and NDP adverts/solicits
            injectPacket = ProcessIPv6Packet(sourceConnectionID, ethhdr, packet, &destinationConnectionID, &destinationIP);
            break;
        default:
            if (m_verboseOutput)
                log_info("Uncommon Ethernet packet type 0x%02X dropped.\n", type);
            injectPacket = false;
            break;
    }

    if (injectPacket)
    {
        // Proxy packet to destination client or inject onto local interface (or both)
        if (m_permissiveMode && m_middlewareEngine->ServerMode())
        {
            if (destinationIP.IsMulticast())
            {
                // Send multicast messages down all paths (includes IPv6 NDP messages)
                m_middlewareEngine->SendBuffer(sourcePacket);                
                InjectPacket(sourcePacket);
            }
            else
            {
                if (destinationConnectionID.IsEmpty())
                {
                    // Send ARP request messages down all paths
                    if (type == ETHERTYPE_ARP)
                        m_middlewareEngine->SendBuffer(sourcePacket);
                
                    InjectPacket(sourcePacket);
                }
                else
                {
                    // Destination connection is known, send directly to client
                    m_middlewareEngine->SendBuffer(destinationConnectionID, sourcePacket);
                }
            }
        }
        else
        {
            InjectPacket(sourcePacket);
        }
    }
}

bool PacketProcessor::ProcessIPv4Packet(const UUID& sourceConnectionID, ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID, IPAddress* destinationIP)
{
    if (packet.length < sizeof(ip))
    {
        log_error("Error processing IPv4 packet: packet is too small, %d of %d bytes.\n", packet.length, sizeof(ip));
        return false;
    }

    // Get IPv4 message
    ip* ipmsg = reinterpret_cast<ip*>(packet.value);
    *destinationIP = ipmsg->ip_dst;

    // Associate incoming IP with connection it was received on
    UpdateIPTable(ipmsg->ip_src, ethhdr->ether_shost, sourceConnectionID);
    
    if (m_transparentOperation)
    {
        // For transparent operation, just lookup destination IP to find destination connection
        TryIPTableLookup(*destinationIP, destinationConnectionID);
        return true;
    }

    if (destinationIP->IsValid(m_ipv4Netmask))
    {
        MacAddress mac;
    
        // Lookup MAC address to find local network IP for ARP responses
        if (TryIPTableLookup(*destinationIP, &mac, destinationConnectionID))
        {
            // MAC found, return proper local network MAC addresses for IP response
            mac.CopyTo(ethhdr->ether_dhost);
        }
        else if (!destinationIP->IsMulticast())
        {
            // Did not find MAC for target IP, send ARP request for IP
            bool sendARPRequest = false;

            // Make sure a pending ARP based MAC request for this IP does not already exist
            if (!PendingIPMacRequestExists(*destinationIP))
            {
                // Maintain pending MAC request table
                m_pendingIPMacRequests[*destinationIP] = time(nullptr);
                sendARPRequest = true;
            }
            
            // Send ARP request
            if (sendARPRequest)
            {
                if (m_verboseOutput)
                    log_info("IPv4: Could not find target IP %s, packet dropped - sending ARP request.\n", destinationIP->ToString().c_str());

                Buffer request = CreateARPRequestPacket(ipmsg);
                InjectPacket(request);
                deletebuffer(request);
            }
            
            // Packets will be dropped until MAC is resolved. IP is inherently unreliable, protocols like TCP can compensate.
            return false;
        }
    }

    return true;
}

bool PacketProcessor::ProcessARPPacket(const UUID& sourceConnectionID, ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID, IPAddress* destinationIP)
{
    if (packet.length < sizeof(ether_arp))
    {
        log_error("Error processing ARP packet: packet is too small, %d of %d bytes.\n", packet.length, sizeof(ether_arp));
        return false;
    }

    // Get ARP message
    ether_arp* arpmsg = reinterpret_cast<ether_arp*>(packet.value);
    u_int16_t op = ntohs(arpmsg->arp_op);
    *destinationIP = *reinterpret_cast<in_addr*>(arpmsg->arp_tpa);

    // Associate incoming IP with connection it was received on
    UpdateIPTable(*reinterpret_cast<in_addr*>(arpmsg->arp_spa), ethhdr->ether_shost, sourceConnectionID);

    if (m_transparentOperation)
    {
        // For transparent operation, just lookup destination IP to find destination connection
        TryIPTableLookup(*destinationIP, destinationConnectionID);
        return true;
    }

    if (op == ARPOP_REQUEST || op == ARPOP_RREQUEST || op == ARPOP_InREQUEST)
    {
        // ARP request messages contain sender MAC address, set address as local machine
        m_captureEngine->LocalMACAddress.CopyTo(arpmsg->arp_sha);
    }
    else
    {            
        MacAddress mac;
    
        // Return proper local network MAC addresses for ARP response, this
        // is to proxy response to some requester on local network - since
        // this is a response to ARP request made on local network, we should
        // already know sender MAC - if not, response is for ARP request that
        // occurred before app started, in which case there is nothing we can
        // do but drop packet and wait for local requester to retry ARP.
        if (TryIPTableLookup(*destinationIP, &mac, destinationConnectionID))
        {
            mac.CopyTo(ethhdr->ether_dhost);
            mac.CopyTo(arpmsg->arp_tha);
            m_captureEngine->LocalMACAddress.CopyTo(arpmsg->arp_sha);
        }
        else
        {
            // Did not find MAC for target IP, drop packet
            return false;
        }
    }

    return true;
}

Buffer PacketProcessor::CreateARPRequestPacket(const ip* ipmsg)
{
    // Construct a buffer large enough to hold ARP request
    Buffer requestPacket = Buffer::Create(sizeof(ether_header) + sizeof(ether_arp));

    // Define Ethernet header
    ether_header* ethhdr = reinterpret_cast<ether_header*>(requestPacket.value);
    ethhdr->ether_type = htons(ETH_P_ARP);

    MacAddress::Broadcast.CopyTo(ethhdr->ether_dhost);
    m_captureEngine->LocalMACAddress.CopyTo(ethhdr->ether_shost);

    // Define ARP request
    ether_arp* request = reinterpret_cast<ether_arp*>(ethhdr + 1);

    request->arp_hrd = htons(ARPHRD_ETHER);
    request->arp_pro = htons(ETH_P_IP);
    request->arp_hln = ETHER_ADDR_LEN;
    request->arp_pln = sizeof(in_addr);
    request->arp_op = htons(ARPOP_REQUEST);

    m_captureEngine->LocalMACAddress.CopyTo(request->arp_sha);
    MacAddress::Zero.CopyTo(request->arp_tha);

    memcpy(&request->arp_tpa, &ipmsg->ip_dst, sizeof(in_addr));
    memcpy(&request->arp_spa, &ipmsg->ip_src, sizeof(in_addr));

    // Consumer responsible for freeing returned buffer
    return requestPacket;
}

bool PacketProcessor::ProcessIPv6Packet(const UUID& sourceConnectionID, ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID, IPAddress* destinationIP)
{
    if (packet.length < sizeof(ip6_hdr))
    {
        log_error("Error parsing IPv6 packet: packet is too small, %d of %d bytes.\n", packet.length, sizeof(ip6_hdr));
        return false;
    }

    // Get IPv6 message
    ip6_hdr* ipmsg = reinterpret_cast<ip6_hdr*>(packet.value);
    *destinationIP = ipmsg->ip6_dst;

    // Associate incoming IP with connection it was received on
    UpdateIPTable(ipmsg->ip6_src, ethhdr->ether_shost, sourceConnectionID);

    if (m_transparentOperation)
    {
        // For transparent operation, just lookup destination IP to find destination connection
        TryIPTableLookup(*destinationIP, destinationConnectionID);
        return true;
    }

    MacAddress mac;

    if (TryIPTableLookup(*destinationIP, &mac, destinationConnectionID))
    {
        // MAC found, return proper local network MAC addresses for IP response
        mac.CopyTo(ethhdr->ether_dhost);
    }
    else if (!destinationIP->IsMulticast())
    {
        // Did not find MAC for target IP, send NDP request for IP
        bool sendNDPRequest = false;
            
        // Make sure a pending NDP based MAC request for this IP does not already exist
        if (!PendingIPMacRequestExists(*destinationIP))
        {
            // Maintain pending MAC request table
            m_pendingIPMacRequests[*destinationIP] = time(nullptr);
            sendNDPRequest = true;
        }
    
        // Send NDP request
        if (sendNDPRequest)
        {
            if (m_verboseOutput)
                log_info("IPv6: Could not find target IP %s, packet dropped - sending NDP request.\n", destinationIP->ToString().c_str());

            Buffer request = CreateNDPRequestPacket(ipmsg);
            InjectPacket(request);
            deletebuffer(request);
        }
    
        // Packets will be dropped until MAC is resolved. IP is inherently unreliable, protocols like TCP can compensate.
        return false;
    }

    // Skip past IPv6 header
    packet.value += sizeof(ip6_hdr);
    packet.length -= sizeof(ip6_hdr);

    // Skip past IPv6 extension headers
    int	nextHeader = ipmsg->ip6_nxt;
    ip6_ext* extmsg;
    
    while (nextHeader != IPPROTO_ICMPV6 && nextHeader != -1)
    {
        switch (nextHeader)
        {
            case IPPROTO_HOPOPTS:
            case IPPROTO_ROUTING:
            case IPPROTO_FRAGMENT:
            case IPPROTO_DSTOPTS:
                if (packet.length < 8)
                {
                    log_error("Error parsing IPv6 packet: extension header is too small, %d of %d bytes.\n", packet.length, 8);
                    return false;
                }
            
                extmsg = reinterpret_cast<ip6_ext*>(packet.value);
            
                if (packet.length < (extmsg->ip6e_len + 1) * 8)
                {
                    log_error("Error parsing IPv6 packet: extension header is too small, %d of %d bytes.\n", packet.length, (extmsg->ip6e_len + 1) * 8);
                    return false;
                }
            
                packet.value += (extmsg->ip6e_len + 1) * 8;
                packet.length -= (extmsg->ip6e_len + 1) * 8;
            
                nextHeader = extmsg->ip6e_nxt;
                break;
            default:
                nextHeader = -1;
                break;
        }
    }

    if (nextHeader == -1)
        return true;

    // Falling through to ICMPv6
    if (packet.length < sizeof(icmp6_hdr))
    {
        log_error("Error parsing ICMPv6 packet: header is too small, %d of %d bytes.\n", packet.length, sizeof(icmp6_hdr));
        return false;
    }

    // Get ICMPv6 message
    icmp6_hdr* icmp6 = reinterpret_cast<icmp6_hdr*>(packet.value);

    // Check for ICMPv6 type of ND advertisement
    if (icmp6->icmp6_type == ND_NEIGHBOR_ADVERT)
        return ProcessNDPPacket(ethhdr, packet, destinationConnectionID);

    return true;
}

bool PacketProcessor::ProcessNDPPacket(ether_header* ethhdr, Buffer& packet, UUID* destinationConnectionID)
{
    if (packet.length < sizeof(nd_neighbor_advert))
    {
        log_error("Error parsing ICMPv6 ND advertisement packet: packet is too small, %d of %d bytes.\n", packet.length, sizeof(nd_neighbor_advert));
        return false;
    }

    // Get ICMPv6 ND Advertisement message (basically IPv6 style ARP response message)
    nd_neighbor_advert* na = reinterpret_cast<nd_neighbor_advert*>(packet.value);
    IPAddress destinationIP = na->nd_na_target;
                
    // Lookup MAC address to find local network IP for NDP responses
    MacAddress mac;
                    
    // Return proper local network MAC addresses for NDP response, this
    // is to proxy response to some requester on local network - since
    // this is a response to NDP request made on local network, we should
    // already know sender MAC - if not, response is for NDP request that
    // occurred before app started, in which case there is nothing we can
    // do but drop packet and wait for local requester to retry NDP.
    if (!TryIPTableLookup(destinationIP, &mac, destinationConnectionID))
        return false;   // Did not find MAC for target IP, drop packet
        
    mac.CopyTo(ethhdr->ether_dhost);
    
    return true;
}

Buffer PacketProcessor::CreateNDPRequestPacket(const ip6_hdr* ipmsg)
{
    // Construct a buffer large enough to hold NDP solicitation request
    Buffer requestPacket = Buffer::Create(sizeof(ether_header) + sizeof(ip6_hdr) + sizeof(nd_neighbor_solicit));

    // Define Ethernet header
    ether_header* ethhdr = reinterpret_cast<ether_header*>(requestPacket.value);
    ethhdr->ether_type = htons(ETHERTYPE_IPV6);

    MacAddress::Broadcast.CopyTo(ethhdr->ether_dhost);
    m_captureEngine->LocalMACAddress.CopyTo(ethhdr->ether_shost);

    // Define IPv6 header
    ip6_hdr* ip6hdr = reinterpret_cast<ip6_hdr*>(ethhdr + 1);
    ip6hdr->ip6_vfc  = 0x60;
    ip6hdr->ip6_plen = htons(sizeof(nd_neighbor_solicit));
    ip6hdr->ip6_nxt  = IPPROTO_ICMPV6;
    ip6hdr->ip6_hlim = 255;
    ip6hdr->ip6_dst  = ipmsg->ip6_dst;

    // Define NDP request
    nd_neighbor_solicit* request = reinterpret_cast<nd_neighbor_solicit*>(ip6hdr + 1);
    request->nd_ns_type = ND_NEIGHBOR_SOLICIT;
    request->nd_ns_code = 0;
    request->nd_ns_target = ipmsg->ip6_dst;
    
    // Calculate IPv6 checksum
    request->nd_ns_cksum = IPv6CheckSum(ip6hdr, reinterpret_cast<u_int16_t*>(request), sizeof(nd_neighbor_solicit));

    // Consumer responsible for freeing returned buffer
    return requestPacket;
}

void PacketProcessor::InjectPacket(const Buffer& packet)
{
    // Make sure current state is "ready to send" on captured interface
    if (m_processingState == PROCESSING_OUTGOING)
    {
        // Only show injected packet details during MAC address translation mode with maximum feedback requested,
        // the injected packet details should only differ from received packet by updated MAC address
        if (m_verboseOutput && !m_transparentOperation && m_feedbackInterval == 1)
            DescribePacket(packet.value, "Injected");
    
        try
        {
            // Drop frame onto captured interface
            m_captureEngine->InjectPacket(packet);
        }
        catch (runtime_error ex)
        {
            log_error("Capture engine injection failure: %s\n", ex.what());
        }

        if (++m_packetsInjected % m_feedbackInterval == 0)
            log_info("Injected %d packets so far...\n", m_packetsInjected);
    }
    else
    {
        // Queue up injections for processing when capture engine is not being queried for new packets
        m_injectionQueue.push_back(Buffer::Copy(packet));

        // Notify poller to start decode cycle so queued injections can be handled
        spinlock(m_receivedBuffersLock,
        {
            m_queuedBufferCount++ ;
            
            if(eventfd_write(m_fileDescriptor, 1) != 0)
                log_error("Failed to increment Packet Processor file descriptor for queued injections.\n") ;
        });
    }
}

void PacketProcessor::SendPacket(const IPAddress& sourceIP, const u_int8_t* sourceMac, const IPAddress& destinationIP, const Buffer& packet, const ip* ipmsg)
{
    UpdateIPTable(sourceIP, sourceMac);

    // For ARP packets, ipmsg will be NULL
    if (ipmsg == nullptr)
    {
        // Broadcast ARP packets down all paths
        m_middlewareEngine->SendBuffer(packet);
        return;
    }

    if (!SendPacket(destinationIP, packet) && !m_transparentOperation && destinationIP.IsValid(m_ipv4Netmask))
    {
        // Failure to send packet means associated connection cannot be found, this likely means
        // packet path is from an existing connection prior to application start, in this case we
        // send a proxy ARP packet down all client paths to resolve MAC and associated connection
        Buffer request = CreateARPRequestPacket(ipmsg);
        SendMacRequestPacket(destinationIP, request);
        deletebuffer(request);
    }
}

void PacketProcessor::SendPacket(const IPAddress& sourceIP, const u_int8_t* sourceMac, const IPAddress& destinationIP, const Buffer& packet, const ip6_hdr* ipmsg)
{
    UpdateIPTable(sourceIP, sourceMac);
    
    // Check for NDP solicitation packet
    if (ipmsg->ip6_nxt == IPPROTO_ICMPV6)
    {
        const icmp6_hdr* icmp6 = reinterpret_cast<const icmp6_hdr*>(ipmsg + 1);

        // Broadcast NDP solicitation packets down all paths
        if (icmp6->icmp6_type == ND_NEIGHBOR_SOLICIT)
        {
            m_middlewareEngine->SendBuffer(packet);
            return;
        }
    }
    
    if (!SendPacket(destinationIP, packet) && !m_transparentOperation)
    {
        // Failure to send packet means associated connection cannot be found, this likely means
        // packet path is from an existing connection prior to application start, in this case we
        // send a proxy NDP packet down all client paths to resolve MAC and associated connection
        Buffer request = CreateNDPRequestPacket(ipmsg);
        SendMacRequestPacket(destinationIP, request);
        deletebuffer(request);
    }
}

bool PacketProcessor::SendPacket(const IPAddress& destinationIP, const Buffer& packet)
{
    if (m_verboseOutput && m_packetsSent % m_feedbackInterval == 0)
        DescribePacket(packet.value, "RxCI->TxMW");

    if (m_middlewareEngine->ServerMode() && !destinationIP.IsMulticast())
    {
        UUID connectionID;
        
        // Lookup connection ID associated with destination IP
        TryIPTableLookup(destinationIP, &connectionID);

        if (connectionID.IsEmpty())
            m_middlewareEngine->SendBuffer(packet);
        else
            m_middlewareEngine->SendBuffer(connectionID, packet);
    }
    else
    {
        m_middlewareEngine->SendBuffer(packet);
    }
    
    if (++m_packetsSent % m_feedbackInterval == 0)
        log_info("Sent %d packets so far...\n", m_packetsSent);
    
    return true;
}

void PacketProcessor::SendMacRequestPacket(const IPAddress& destinationIP, const Buffer& requestPacket)
{
    log_info("Failed to find associated connection for IP %s, sending proxy MAC address lookup request...\n", destinationIP.ToString().c_str());
    m_middlewareEngine->SendBuffer(requestPacket);
}

bool PacketProcessor::UpdateIPTable(const IPAddress& ip, const u_int8_t* etherMac, const UUID& connectionID)
{
    if (!ip.IsValid(m_ipv4Netmask))
        return false;

    MacAddress mac = MacAddress::CopyFrom(etherMac);
    bool tableUpdated = false;

    // Lookup IP in table
    auto enumerator = m_ipTable.find(ip);

    if (enumerator == m_ipTable.end())
    {
        // Add associated mac, connection ID and last access time for IP address
        m_ipTable[ip] = tuple<MacAddress, UUID, time_t>(mac, connectionID, time(nullptr));
        tableUpdated = true;
        
        if (mac != MacAddress::Zero)
            log_info(">> Mapped local network IP [%s] to MAC %s.\n", ip.ToString().c_str(), mac.ToString().c_str());
    }
    else
    {
        // Update associated mac, connection ID and last access time for IP address
        if (connectionID.IsEmpty())        
            get<2>(enumerator->second) = time(nullptr);
        else
            enumerator->second = tuple<MacAddress, UUID, time_t>(mac, connectionID, time(nullptr));
    }

    if (tableUpdated)
    {
        // Remove resolved MAC address from pending request list
        if (PendingIPMacRequestExists(ip))
            m_pendingIPMacRequests.erase(ip);
    }

    return tableUpdated;
}

inline bool PacketProcessor::TryIPTableLookup(const IPAddress& ip, UUID* connectionID)
{
    return TryIPTableLookup(ip, nullptr, connectionID);
}

inline bool PacketProcessor::TryIPTableLookup(const IPAddress& ip, MacAddress* mac)
{
    return TryIPTableLookup(ip, mac, nullptr);
}

bool PacketProcessor::TryIPTableLookup(const IPAddress& ip, MacAddress* mac, UUID* connectionID)
{
    bool found = false;
    auto enumerator = m_ipTable.find(ip);

    if (enumerator != m_ipTable.end())
    {
        // Found IP, return MAC address and/or connection ID
        if (mac)
            *mac = get<0>(enumerator->second);
    
        if (connectionID)
            *connectionID = get<1>(enumerator->second);
        
        // Update last access time for IP address
        get<2>(enumerator->second) = time(nullptr);

        found = true;
    }

    return found;
}

bool PacketProcessor::PendingIPMacRequestExists(const IPAddress& ip)
{
    auto enumerator = m_pendingIPMacRequests.find(ip);
    return (enumerator != m_pendingIPMacRequests.end());
}

void PacketProcessor::CheckActiveIPAddresses()
{
    vector<IPAddress> expiredIPAddresses;
    time_t lastActivity, now;
    string ipTableDump = "\nCurrent IP Table:\n\n";
    double inactiveTime;
    bool hasItems = false;

    // Get current time
    time(&now);

    // Check IP table for expired addresses
    for (auto enumerator = m_ipTable.begin(); enumerator != m_ipTable.end(); ++enumerator)
    {
        IPAddress ip = enumerator->first;
        lastActivity = get<2>(enumerator->second);
        inactiveTime = difftime(now, lastActivity);

        // Add entry to string
        if (m_verboseOutput)
        {
            ipTableDump += 
                "[" + ip.ToString() + "] = " + 
                get<0>(enumerator->second).ToString() + " : "
                "{" + get<1>(enumerator->second).ToString() + "}";
                
            if (inactiveTime > 0)
                ipTableDump += 
                    " inactive for " + 
                    Timer::ToElapsedTimeString(inactiveTime);
            
            ipTableDump += "\n";
        }
        
        // Mark IP addresses that have had no activity within timeout period
        if (inactiveTime > m_inactiveIPTimeout)
            expiredIPAddresses.push_back(ip);
    
        hasItems = true;
    }

    if (hasItems && m_verboseOutput)
        log_info(ipTableDump.c_str());
    
    // Remove expired IP addresses
    if (expiredIPAddresses.size() > 0)
    {
        log_info("Removing %d expired IP addresses from IP table...\n", expiredIPAddresses.size());
    
        for (int i = 0; i < expiredIPAddresses.size(); i++)
            m_ipTable.erase(expiredIPAddresses[i]);
    }
}

void PacketProcessor::CheckPendingIPMacRequests()
{
    vector<IPAddress> expiredRequests;
    time_t lastActivity, now;

    // Get current time
    time(&now);

    // Check for old MAC resolution requests
    for (auto enumerator = m_pendingIPMacRequests.begin(); enumerator != m_pendingIPMacRequests.end(); ++enumerator)
    {
        IPAddress ip = enumerator->first;
        lastActivity = enumerator->second;

        // Mark MAC resolution requests that have had no response within last minute
        if (difftime(now, lastActivity) > m_macResolutionTimeout)
            expiredRequests.push_back(ip);
    }

    // Remove old MAC resolution requests
    if (expiredRequests.size() > 0)
    {
        log_info("Removing %d expired MAC resolution requests from pending table...\n", expiredRequests.size());
    
        for (int i = 0; i < expiredRequests.size(); i++)
            m_pendingIPMacRequests.erase(expiredRequests[i]);
    }
}

void PacketProcessor::IPActivityTimerElapsed(void* state)
{
    PacketProcessor* processor = static_cast<PacketProcessor*>(state);

    if (processor == nullptr)
        throw invalid_argument("PacketProcessor::IPActivityTimerElapsed PacketProcessor instance is null");

    processor->m_checkIPActivity = true;
}

void PacketProcessor::PendingIPMacRequestsTimerElasped(void* state)
{    
    PacketProcessor* processor = static_cast<PacketProcessor*>(state);

    if (processor == nullptr)
        throw invalid_argument("PacketProcessor::PendingIPMacRequestsTimerElasped PacketProcessor instance is null");

    processor->m_checkPendingIPMacRequests = true;
}

// Based on RFC 1071 "C" implementation example
static u_int16_t IPv6CheckSum(ip6_hdr* header, u_int16_t* data, int length)
{
    unsigned long sum = 0;
    int count;
    u_int16_t* address;
    int i;

    address = reinterpret_cast<u_int16_t*>(header);

    for (i = 0; i < 20; i++)
        sum += *address++;

    count = length;
    address = data;

    while (count > 1)
    {
        sum += *(address++);
        count -= 2;
    }

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return static_cast<u_int16_t>(~sum);
}