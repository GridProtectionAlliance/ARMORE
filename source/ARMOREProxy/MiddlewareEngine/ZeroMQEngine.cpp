//******************************************************************************************************
//  ZeroMQEngine.cpp - Gbtc
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
//  06/11/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "ZeroMQEngine.h"
#include "ZeroMQServer.h"
#include "ZeroMQClient.h"

#include <stdexcept>
#include "../ConsoleLogger/ConsoleLogger.h"

ZeroMQEngine::ZeroMQEngine(BufferReceivedCallback callback, bool serverMode, bool securityEnabled, int inactiveClientTimeout, bool verboseOutput) : 
    MiddlewareEngine(callback, serverMode, securityEnabled, inactiveClientTimeout, verboseOutput)
{
    m_instance = nullptr;
    m_bufferReceivedCallback = callback;
}

ZeroMQEngine::~ZeroMQEngine()
{
    if (m_instance)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

void ZeroMQEngine::Initialize(const string endPoint, const string instanceName, const string instanceID)
{
    zcert_t* localCertificate = nullptr;

    // Handle common security initialization for server and client instances
    if (SecurityEnabled())
    {
        if (!zsys_has_curve())
            throw runtime_error("Failed to locate needed curve security libraries, cannot initialize ZeroMQ security");
        
        string localCertDirectory = CURVECERTLOCALDIR "/";
        string publicCertFileName;
        string privateCertFileName;
        
        // Make sure certificate store directories exist
        if (zsys_dir_create(CURVECERTLOCALDIR) != 0)
            throw runtime_error("Failed to create local curve security store, cannot initialize ZeroMQ security");

        if (zsys_dir_create(CURVECERTREMOTEDIR) != 0)
            throw runtime_error("Failed to create remote curve security store, cannot initialize ZeroMQ security");
        
        // Derive certificate file names based on engine mode
        if (ServerMode())
        {
            publicCertFileName = localCertDirectory + SVRPUBCERTFILENAME;
            privateCertFileName = localCertDirectory + SVRPVTCERTFILENAME;
        }
        else
        {
            publicCertFileName = localCertDirectory + instanceName + PUBCERTFILENAMEEXT;
            privateCertFileName = localCertDirectory + instanceName + PVTCERTFILENAMEEXT;
        }
        
        // See if private certificate already exists
        if (zsys_file_exists(privateCertFileName.c_str()))
        {
            // Load existing local certificate
            localCertificate = zcert_load(privateCertFileName.c_str());
            
            if (localCertificate == nullptr)
                throw runtime_error("Failed to load local curve certificate, cannot initialize ZeroMQ security");
        }
        else
        {
            // Create a new full certificate (public + private)
            localCertificate = zcert_new();
            
            if (localCertificate == nullptr)
                throw runtime_error("Failed to create local curve certificate, cannot initialize ZeroMQ security");

            zcert_set_meta(localCertificate, "name", instanceName.c_str());
            zcert_set_meta(localCertificate, "id", instanceID.c_str());
            zcert_set_meta(localCertificate, "type", ServerMode() ? "server" : "client");
            
            // Persist certificates for future runs
            if (zcert_save_public(localCertificate, publicCertFileName.c_str()) != 0)
                throw runtime_error("Failed to save local curve public certificate, cannot initialize ZeroMQ security");
            
            if (zcert_save_secret(localCertificate, privateCertFileName.c_str()) != 0)
                throw runtime_error("Failed to save local curve private certificate, cannot initialize ZeroMQ security");
        }        
    }
    
    // Create new ZeroMQ engine instance
    if (ServerMode())
        m_instance = new ZeroMQServer(m_bufferReceivedCallback, SecurityEnabled(), InactiveClientTimeout(), VerboseOutput(), ConnectionID(), localCertificate);
    else
        m_instance = new ZeroMQClient(m_bufferReceivedCallback, SecurityEnabled(), InactiveClientTimeout(), VerboseOutput(), ConnectionID(), localCertificate);

    m_instance->Initialize(endPoint, instanceName, instanceID);

    log_info("\nEstablished ZeroMQ socket [%s]\n", ConnectionID().ToString().c_str());
}

void ZeroMQEngine::Shutdown()
{
    if (m_instance)
        m_instance->Shutdown();
}

bool ZeroMQEngine::SendBuffer(const Buffer& buffer)
{
    if (m_instance)
        return m_instance->SendBuffer(buffer);

    return false;
}

bool ZeroMQEngine::SendBuffer(const UUID& connectionID, const Buffer& buffer)
{
    if (m_instance)
        return m_instance->SendBuffer(connectionID, buffer);

    return false;
}
