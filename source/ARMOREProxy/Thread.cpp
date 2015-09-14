//******************************************************************************************************
//  Thread.cpp - Gbtc
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
//  06/12/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "Thread.h"
#include <string.h>
#include <signal.h>
#include <stdexcept>
#include "ConsoleLogger/ConsoleLogger.h"

using namespace std;

Thread::Thread(ThreadStart worker, void* arg, bool autoStart, bool cancelOnDestruct)
{
    m_thread = 0;
    m_worker = worker;
    m_arg = arg;
    m_cancelOnDestruct = cancelOnDestruct;
    
    if (autoStart)
        Start();
}

Thread::~Thread(void)
{
    if (m_cancelOnDestruct)
        Cancel();
}

void Thread::Start()
{
    if (m_thread)
        throw runtime_error("Thread already started.");
    
    // Create thread
    int err = pthread_create(&m_thread, nullptr, &ThreadProc, this);

    if (err != 0)
        throw runtime_error("Failed to create Thread: " + string(strerror(err)));
}

void Thread::Cancel()       
{
    if (m_thread)
    {
        pthread_cancel(m_thread);
        m_thread = 0;
    }
}

void* Thread::GetArgument()
{
    return m_arg;
}

void Thread::SetArgument(void* arg)
{
    m_arg = arg;
}

void* Thread::ThreadProc(void* arg)
{
    Thread* thread = static_cast<Thread*>(arg);

    if (thread == nullptr)
        throw invalid_argument("Thread::ThreadProc Thread instance is null");

    try
    {
        // Call user worker function with any provided parameter
        thread->m_worker(thread->m_arg);
    }
    catch (exception& ex)
    {
        log_error("Thread::ThreadProc exception: %s\n", ex.what());
        raise(SIGTERM);
    }

    return nullptr;
}