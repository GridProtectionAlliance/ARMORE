//******************************************************************************************************
//  NetmapEngine.h - Gbtc
//
//  Copyright � 2015, Grid Protection Alliance.  All Rights Reserved.
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

#ifndef _NETMAPENGINE_H
#define _NETMAPENGINE_H

#define NETMAP_WITH_LIBS
#include "netmap_user.h"
#include "CaptureEngine.h"

using namespace std;

class NetmapEngine : public CaptureEngine
{
public:
    // Constructors
    explicit NetmapEngine(const string interfaceName);
    ~NetmapEngine() override;

    // Methods
    void Open() override;
    void Close() override;
    int GetFileDescriptor() const override;
    Buffer GetNextPacket(timeval& ts) const override;
    void InjectPacket(const Buffer& buffer) const override;

private:
    struct nm_desc* m_nmd;
};

#endif