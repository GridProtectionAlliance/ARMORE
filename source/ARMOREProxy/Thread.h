//******************************************************************************************************
//  Thread.h - Gbtc
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

#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>

typedef void(*ThreadStart)(void* arg);

class Thread
{
public:
    // Constructors

    /// <summary>Creates a new Thread instance.</summary>
    /// <param name="worker">Worker function to call when starting new thread.</param>
    /// <param name="arg">Any thread argument to be passed to worker function.</param>
    /// <param name="autoStart">Determines if thread should be automatically started on class construction.</param>
    /// <param name="cancelOnDestruct">Determines if thread should be canceled on class destruction.</param>
    explicit Thread(ThreadStart worker, void* arg = nullptr, bool autoStart = false, bool cancelOnDestruct = false);

    ~Thread(void);
    
    // Methods
    void Start();
    void Cancel();
    
    void* GetArgument();
    void SetArgument(void* arg);

private:
    static void* ThreadProc(void* arg);

    ThreadStart m_worker;
    pthread_t m_thread;
    void* m_arg;
    bool m_cancelOnDestruct;
};

#endif