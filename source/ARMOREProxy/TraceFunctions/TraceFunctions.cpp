//******************************************************************************************************
//  TraceFunctions.cpp - Gbtc
//
//  Copyright © 2015, Grid Protection Alliance.  All Rights Reserved.
//
//  Licensed to the Grid Protection Alliance (GPA) under one or more contributor license agreements. See
//  the NOTICE file distributed with this work for additional information regarding copyright ownership.
//  The GPA licenses this file to you under the MIT License (MIT), the "License"; you may
//  not use this file except in compliance with the License. You may obtain a copy of the License at:
//
//      http://www.opensource.org/licenses/MIT
//
//  Unless agreed to in writing, the subject software distributed under the License is distributed on an
//  "AS-IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. Refer to the
//  License for the specific language governing permissions and limitations.
//
//  Code Modification History:
//  ----------------------------------------------------------------------------------------------------
//  04/23/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "TraceFunctions.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <string>
#include "../MacAddress.h"
#include "../ConsoleLogger/ConsoleLogger.h"

void GetIPv4PacketSummary(const u_int8_t* buffer, char* summary)
{
    struct ip* ipHeader;
    struct icmphdr* icmphdr;
    struct tcphdr* tcphdr;
    struct udphdr* udphdr;
    char srcip[INET_ADDRSTRLEN], dstip[INET_ADDRSTRLEN];
    u_int8_t protocol;
    unsigned short seq;

    // Find position of the IP header
    buffer += sizeof(struct ether_header);
    ipHeader = (struct ip*)buffer;

    strcpy(srcip, inet_ntoa(ipHeader->ip_src));
    strcpy(dstip, inet_ntoa(ipHeader->ip_dst));

    protocol = ipHeader->ip_p;

    // Find position of the transport layer header
    buffer += 4 * ipHeader->ip_hl;

    switch (protocol)
    {
        case IPPROTO_TCP:
            tcphdr = (struct tcphdr*)buffer;
            sprintf(summary, "TCP %s:%d->%s:%d", srcip, ntohs(tcphdr->source), dstip, ntohs(tcphdr->dest));
            break;
        case IPPROTO_UDP:
            udphdr = (struct udphdr*)buffer;
            sprintf(summary, "UDP %s:%d->%s:%d", srcip, ntohs(udphdr->source), dstip, ntohs(udphdr->dest));
            break;
        case IPPROTO_ICMP:
            icmphdr = (struct icmphdr*)buffer;
            memcpy(&seq, (u_char*)icmphdr+6, 2);
            sprintf(summary, "ICMP %s->%s, seq:%d", srcip, dstip, ntohs(seq));
            break;
        default:
            sprintf(summary, "IPPROTO=%d, %s->%s", protocol, srcip, dstip);
            break;
    }            
}

void GetIPv6PacketSummary(const u_int8_t* buffer, char* summary)
{
    struct ip6_hdr* ipv6Header;
    struct icmp6_hdr* icmp6hdr;
    struct tcphdr* tcphdr;
    struct udphdr* udphdr;
    char srcip[INET6_ADDRSTRLEN], dstip[INET6_ADDRSTRLEN];
    u_int8_t protocol;
    unsigned short seq;

    // Find position of the IP header
    buffer += sizeof(struct ether_header);
    ipv6Header = (struct ip6_hdr*)buffer;

    inet_ntop(AF_INET6, &(ipv6Header->ip6_src), srcip, sizeof(srcip));
    inet_ntop(AF_INET6, &(ipv6Header->ip6_dst), dstip, sizeof(dstip));

    protocol = ipv6Header->ip6_nxt;

    // Find position of the transport layer header
    buffer += 40;

    switch (protocol)
    {
        case IPPROTO_TCP:
            tcphdr = (struct tcphdr*)buffer;
            sprintf(summary, "TCP %s:%d->%s:%d", srcip, ntohs(tcphdr->source), dstip, ntohs(tcphdr->dest));
            break;
        case IPPROTO_UDP:
            udphdr = (struct udphdr*)buffer;
            sprintf(summary, "UDP %s:%d->%s:%d", srcip, ntohs(udphdr->source), dstip, ntohs(udphdr->dest));
            break;
        case IPPROTO_ICMPV6:
            icmp6hdr = (struct icmp6_hdr*)buffer;
            memcpy(&seq, &icmp6hdr->icmp6_seq, 2);
            sprintf(summary, "ICMP6 %s->%s, seq:%d", srcip, dstip, ntohs(seq));
            break;
        default:
            sprintf(summary, "IPPROTO=%d, %s->%s", protocol, srcip, dstip);
            break;
    }            
}

void DescribePacket(const u_int8_t* data, const char* io)
{
    struct ether_header* header = (struct ether_header*)data;
    unsigned int type = ntohs(header->ether_type);
    char summary[256];
    MacAddress mac;

    switch (type)
    {
        case ETHERTYPE_IP:
        case ETHERTYPE_ARP:
            if (type == ETHERTYPE_IP)
            {
                GetIPv4PacketSummary(data, summary);
                log_info("%s IPv4: %s", io, summary);
            }
            else
            {
                struct ether_arp* arpreq = (struct ether_arp*)(data + sizeof(struct ether_header));
                mac = MacAddress::CopyFrom(arpreq->arp_sha);
                log_info("%s ARP: op: %d - sender: %s", io, ntohs(arpreq->arp_op), mac.ToString().c_str());
                char srcip[INET_ADDRSTRLEN], dstip[INET_ADDRSTRLEN];
                strcpy(srcip, inet_ntoa(*(in_addr*)arpreq->arp_spa));
                strcpy(dstip, inet_ntoa(*(in_addr*)arpreq->arp_tpa));
                log_info("       For: %s->%s", srcip, dstip);
            }
            break;
        case ETHERTYPE_IPV6:
            GetIPv6PacketSummary(data, summary);
            log_info("%s IPv6: %s", io, summary);
            break;
        default:
            log_info("%s 0x%X packet", io, type);
            break;
    }
    
    mac = MacAddress::CopyFrom(header->ether_shost);
    log_info("    Source: %s", mac.ToString().c_str());
    mac = MacAddress::CopyFrom(header->ether_dhost);
    log_info("    Target: %s", mac.ToString().c_str());

    printf("\n");
}