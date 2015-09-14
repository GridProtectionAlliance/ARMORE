//******************************************************************************************************
//  PcapEngine.cpp - Gbtc
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

#include "PcapEngine.h"
#include <stdexcept>

PcapEngine::PcapEngine(const string interfaceName) : CaptureEngine(interfaceName)
{
    m_pcap = nullptr;
}

PcapEngine::~PcapEngine()
{
    if (m_pcap != nullptr)
        pcap_close(m_pcap);
}

void PcapEngine::Open()
{
    if (m_pcap != nullptr)
        throw runtime_error("PCAP session already open");

    char pcap_errbuf[PCAP_ERRBUF_SIZE];

    // Open a PCAP packet capture session for the specified interface
    pcap_errbuf[0] = '\0';
    
    m_pcap = pcap_open_live(InterfaceName.c_str(), BUFSIZ, 1, 5, pcap_errbuf);
    
    if (pcap_errbuf[0] != '\0')
        throw runtime_error("Failed to open PCAP session: " + string(pcap_errbuf));
}

void PcapEngine::Close()
{
    if (m_pcap != nullptr)
    {
        pcap_close(m_pcap);
        m_pcap = nullptr;
    }
}

int PcapEngine::GetFileDescriptor() const
{
    if (m_pcap == nullptr)
        return -1;

    return pcap_get_selectable_fd(m_pcap);
}

Buffer PcapEngine::GetNextPacket(timeval& ts) const
{
    if (m_pcap == nullptr)
        return Buffer::Empty;

    pcap_pkthdr *header;
    const u_char *data;

    int result = pcap_next_ex(m_pcap, &header, &data);

    if (result != 1)
        return Buffer::Empty;

    Buffer buffer((u_int8_t*)data, header->len);

    ts = header->ts;

    return buffer;
}

void PcapEngine::InjectPacket(const Buffer& buffer) const
{
    if (pcap_inject(m_pcap, buffer.value, buffer.length) == -1)
        throw runtime_error("Failed to inject PCAP buffer: " + string(pcap_geterr(m_pcap)));
}
