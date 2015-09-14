//******************************************************************************************************
//  CaptureEngine.h - Gbtc
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

#ifndef _CAPTUREENGINE_H
#define _CAPTUREENGINE_H

#include <string>
#include "../Buffer.h"
#include "../MacAddress.h"
#include "../IPAddress.h"

using namespace std;

enum class CaptureEngineOption : int
{
    Netmap,
    PCAP,
    DPDK,
    Default = Netmap
};

class CaptureEngine
{
public:
    // Fields
    const string InterfaceName;
    const MacAddress LocalMACAddress;
    const IPAddress LocalIPv4NetMask;

    // Constructors
    explicit CaptureEngine(const string interfaceName);
    virtual ~CaptureEngine();
    
    // Methods

    // Opens the capture engine
    virtual void Open() = 0;

    // Closes the capture engine
    virtual void Close() = 0;

    // Gets a file descriptor for the capture engine for polling use
    virtual int GetFileDescriptor() const = 0;

    // Gets next packet
    Buffer GetNextPacket() const;

    // Gets next packet and provides timestamp through out parameter
    virtual Buffer GetNextPacket(timeval& ts) const = 0;

    // Injects a packet onto the captured interface
    // Consumers expect runtime_error to be thrown on injection failure
    virtual void InjectPacket(const Buffer& buffer) const = 0;
};

#endif