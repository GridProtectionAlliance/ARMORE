//******************************************************************************************************
//  CaptureEngine.cpp - Gbtc
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
//  04/24/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "CaptureEngine.h"
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <stdexcept>

static in_addr IPv4NetMaskInterfaceLookup(const string interfaceName);

CaptureEngine::CaptureEngine(const string interfaceName) :
    InterfaceName(interfaceName),
    LocalMACAddress(MacAddress::InterfaceLookup(interfaceName)),
    LocalIPv4NetMask(IPv4NetMaskInterfaceLookup(interfaceName))
{
}

CaptureEngine::~CaptureEngine()
{   
}

Buffer CaptureEngine::GetNextPacket() const
{
    timeval ts;
    return GetNextPacket(ts);
}

static in_addr IPv4NetMaskInterfaceLookup(const string interfaceName)
{
    struct ifreq ifr;
    
    // Open an IP based socket
    int sfd = socket(PF_INET, SOCK_DGRAM, 0);

    if (sfd == -1)
        throw runtime_error("Failed to open IP socket needed to lookup interface IPv4 net mask");

    // Initialize interface request structure with desired interface name
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    
    // Net mask is only applicable for IPv4
    ifr.ifr_addr.sa_family = AF_INET;
    
    // Request I/O control operation to receive net mask
    if (ioctl(sfd, SIOCGIFNETMASK, &ifr) == -1)
    {
        close(sfd);
        throw runtime_error("Failed to lookup interface IPv4 net mask");
    }

    // Close the socket
    close(sfd);

    return reinterpret_cast<sockaddr_in*>(&ifr.ifr_netmask)->sin_addr;
}