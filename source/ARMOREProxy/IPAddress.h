//******************************************************************************************************
//  IPAddress.h - Gbtc
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

#ifndef _IPADDRESS_H
#define _IPADDRESS_H

#include <sys/types.h>
#include <netinet/in.h>
#include <string>

using namespace std;

class IPAddress
{
public:
    // Constructors
    IPAddress();
    IPAddress(const in_addr& address);
    IPAddress(const in6_addr& address);
    ~IPAddress();
    
    // Methods
    std::size_t GetHashCode() const;
    bool IsIPv4() const;
    bool IsIPv6() const;
    bool IsValid(const IPAddress& netmask) const;
    bool IsMulticast() const;
    string ToString() const;
    const u_int8_t* GetValue() const;
    
    // Operators
    bool operator<(const IPAddress& other) const;
    bool operator==(const IPAddress& other) const;

    // Static
    static IPAddress Parse(const string address, bool isIPv6 = false);
    
private:
    int m_ipVersion;
    in_addr m_ipv4Value;
    in6_addr m_ipv6Value;
    std::size_t m_hashCode;
};

ostream& operator<<(ostream&, const IPAddress&);

// Create hash function for IPAddress
namespace std
{
    template<> struct hash<IPAddress>
    {
        size_t operator()(const IPAddress& address) const
        {
            return address.GetHashCode();
        }
    };
}

#endif

