//******************************************************************************************************
//  MacAddress.cpp - Gbtc
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
//  04/23/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "MacAddress.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <stdexcept>

const MacAddress MacAddress::Broadcast = FromULong(0xFFFFFFFFFFFFFFFF);
const MacAddress MacAddress::Zero      = FromULong(0x0000000000000000);

MacAddress::MacAddress()
{
    memset(value, 0, 8);
}

MacAddress::~MacAddress()
{
}

u_int64_t MacAddress::ToULong() const
{
    return *(u_int64_t*)value;
}

void MacAddress::CopyTo(u_int8_t* address) const
{
    memcpy(address, value, ETH_ALEN);
}

string MacAddress::ToString() const
{
    char address[32] = {0};
    int length = 0;

    for (int i = 0; i < 6; i++)
        length += sprintf(address + length, i ? ":%02X" : "%02X", value[i]);

    return address;
}

bool MacAddress::operator<(const MacAddress& other) const
{
    return memcmp(value, other.value, 8) < 0;
}

bool MacAddress::operator==(const MacAddress& other) const
{
    return memcmp(value, other.value, 8) == 0;
}

bool MacAddress::operator!=(const MacAddress& other) const
{
    return memcmp(value, other.value, 8) != 0;
}

ostream& operator<<(ostream& strm, MacAddress& mac)
{
    return strm << mac.ToString();
}

MacAddress MacAddress::InterfaceLookup(const string interfaceName)
{
    MacAddress mac;
    struct ifreq ifr;
    
    // Open an IP based socket
    int sfd = socket(PF_INET, SOCK_DGRAM, 0);

    if (sfd == -1)
        throw runtime_error("Failed to open IP socket needed to lookup interface MAC address");

    // Initialize interface request structure with desired interface name
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);

    // Request I/O control operation to receive MAC address
    if (ioctl(sfd, SIOCGIFHWADDR, &ifr) == -1)
    {
        close(sfd);
        throw runtime_error("Failed to lookup interface MAC address");
    }
        
    // Copy the MAC address
    memcpy(mac.value, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    // Close the socket
    close(sfd);

    return mac;
}

MacAddress MacAddress::Parse(const string address)
{
    MacAddress mac;
    
    if (sscanf(address.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac.value[0], &mac.value[1], &mac.value[2], &mac.value[3], &mac.value[4], &mac.value[5]) != ETH_ALEN)
        throw out_of_range("Failed to parse MAC address from string - expected 6 colon separated hex values, e.g.:  0A:1B:2C:3D:4E:5F");
    
    return mac;
}

MacAddress MacAddress::CopyFrom(const u_int8_t* address)
{
    MacAddress mac;

    memcpy(mac.value, address, ETH_ALEN);

    return mac;
}

MacAddress MacAddress::FromULong(u_int64_t address)
{
    MacAddress mac;

    *reinterpret_cast<u_int64_t*>(mac.value) = address;

    return mac;
}
