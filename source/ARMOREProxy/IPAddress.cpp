//******************************************************************************************************
//  IPAddress.cpp - Gbtc
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
//  07/23/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "IPAddress.h"
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <stdexcept>

using namespace std;

IPAddress::IPAddress()
{
    m_ipVersion = 4;
    m_ipv4Value.s_addr = 0;
    memset(m_ipv6Value.s6_addr, 0, sizeof(in6_addr));
    m_hashCode = 0;
}

IPAddress::IPAddress(const in_addr& address)
{
    m_ipVersion = 4;
    m_ipv4Value = address;
    memset(m_ipv6Value.s6_addr, 0, sizeof(in6_addr));
    m_hashCode = hash<u_int32_t>()(address.s_addr);
}

IPAddress::IPAddress(const in6_addr& address)
{
    m_ipVersion = 6;
    m_ipv4Value.s_addr = 0;
    m_ipv6Value = address;
    m_hashCode = hash<string>()(ToString());
}

IPAddress::~IPAddress()
{
}
    
std::size_t IPAddress::GetHashCode() const
{
    return m_hashCode;
}

inline bool IPAddress::IsIPv4() const
{
    return m_ipVersion == 4;
}

inline bool IPAddress::IsIPv6() const
{
    return m_ipVersion == 6;
}

bool IPAddress::operator<(const IPAddress& other) const
{
    if (IsIPv6())
        return memcmp(m_ipv6Value.s6_addr, other.m_ipv6Value.s6_addr, 16) < 0;
        
   return m_ipv4Value.s_addr < other.m_ipv4Value.s_addr;
}

bool IPAddress::operator==(const IPAddress& other) const
{
    if (IsIPv6())
        return memcmp(m_ipv6Value.s6_addr, other.m_ipv6Value.s6_addr, 16) == 0;

    return m_ipv4Value.s_addr == other.m_ipv4Value.s_addr;
}

bool IPAddress::IsValid(const IPAddress& netmask) const
{
    if (IsIPv6())
        return true;
    
    if (netmask.IsIPv6())
        throw runtime_error("Network mask validation is only applicable for IPv4 addresses.");
    
    in_addr_t mask = ~netmask.m_ipv4Value.s_addr;
    in_addr_t result = m_ipv4Value.s_addr & mask;
    
    return result > 0 && result < mask;
}

bool IPAddress::IsMulticast() const
{
    if (IsIPv6())
        return m_ipv6Value.s6_addr[0] == 0xFF;
    
    int octet = m_ipv4Value.s_addr & 0xFF;  
    return octet > 223 && octet < 240;
}

string IPAddress::ToString() const
{
    if (IsIPv6())
    {
        char address[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &m_ipv6Value, address, sizeof(address));
        return address;
    }
    
    char address[INET_ADDRSTRLEN];
    strcpy(address, inet_ntoa(m_ipv4Value));
    return address;
}

const u_int8_t* IPAddress::GetValue() const
{
    if (IsIPv6())
        return m_ipv6Value.s6_addr;
    
    return reinterpret_cast<const u_int8_t*>(&m_ipv4Value.s_addr);
}

IPAddress IPAddress::Parse(const string address, bool isIPv6)
{
    if (isIPv6)
    {
        in6_addr ipv6Value;
    
        if (inet_pton(AF_INET6, address.c_str(), &ipv6Value) != 1)
            throw out_of_range("Failed to parse IPv6 address from string - the preferred format is x:x:x:x:x:x:x:x");

        return IPAddress(ipv6Value);
    }
    
    in_addr ipv4Value;
    
    if (inet_pton(AF_INET, address.c_str(), &ipv4Value) != 1)
        throw out_of_range("Failed to parse IPv4 address from string - the format is ddd.ddd.ddd.ddd");
    
    return IPAddress(ipv4Value);
}

ostream& operator<<(ostream& strm, IPAddress& ip)
{
    return strm << ip.ToString();
}
