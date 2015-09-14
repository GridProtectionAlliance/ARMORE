//******************************************************************************************************
//  NetmapEngine.cpp - Gbtc
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

#include "NetmapEngine.h"
#include <stdexcept>

NetmapEngine::NetmapEngine(const string interfaceName) : CaptureEngine(interfaceName)
{
    m_nmd = nullptr;
}

NetmapEngine::~NetmapEngine()
{
    if (m_nmd != nullptr)
        nm_close(m_nmd);
}

void NetmapEngine::Open()
{
    if (m_nmd != nullptr)
        throw runtime_error("netmap session already open");

    string binding = "netmap:" + InterfaceName;

    m_nmd = nm_open(binding.c_str(), nullptr, 0, nullptr);

    if (m_nmd == nullptr)
        throw runtime_error("Failed to open netmap session: " + string(strerror(errno)));
}

void NetmapEngine::Close()
{
    if (m_nmd != nullptr)
    {
        nm_close(m_nmd);
        m_nmd = nullptr;
    }
}

int NetmapEngine::GetFileDescriptor() const
{
    if (m_nmd == nullptr)
        return -1;

    return NETMAP_FD(m_nmd);
}

Buffer NetmapEngine::GetNextPacket(timeval& ts) const
{
    if (m_nmd == nullptr)
        return Buffer::Empty;

    nm_pkthdr header;

    u_int8_t* packet = reinterpret_cast<u_int8_t*>(nm_nextpkt(m_nmd, &header));

    if (packet == nullptr)
        return Buffer::Empty;

    Buffer buffer(packet, header.len);

    ts = header.ts;

    return buffer;
}

void NetmapEngine::InjectPacket(const Buffer& buffer) const
{
    if (nm_inject(m_nmd, buffer.value, buffer.length) == -1)
        throw runtime_error("Failed to inject netmap packet");
}
