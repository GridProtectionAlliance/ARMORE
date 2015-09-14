//******************************************************************************************************
//  ZeroMQEngine.h - Gbtc
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
//  06/11/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#ifndef _ZEROMQENGINE_H
#define _ZEROMQENGINE_H

#include "MiddlewareEngine.h"

using namespace std;

class ZeroMQEngine : public MiddlewareEngine
{
public:
    // Constructors
    ZeroMQEngine(BufferReceivedCallback callback, bool serverMode, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput);
    ~ZeroMQEngine();
    
    // Methods
    void Initialize(const string endPoint, const string instanceName, const string instanceID) override;
    void Shutdown() override;
    bool SendBuffer(const Buffer& buffer) override;
    bool SendBuffer(const UUID& connectionID, const Buffer& buffer) override;
    
private:    
    // Fields
    MiddlewareEngine* m_instance;
    BufferReceivedCallback m_bufferReceivedCallback;
};

#endif