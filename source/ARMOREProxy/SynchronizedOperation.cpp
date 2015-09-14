//******************************************************************************************************
//  SynchronizedOperation.cpp - Gbtc
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
//  07/16/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "SynchronizedOperation.h"
#include "Thread.h"
#include <stdexcept>

using namespace std;

#define NotRunning  0
#define Running     1
#define Pending     2

SynchronizedOperation::SynchronizedOperation(Action action, void* arg)
{
    m_action = action;
    m_arg = arg;
    m_state = NotRunning;
}

SynchronizedOperation::~SynchronizedOperation()
{
}

bool SynchronizedOperation::IsRunning()
{
    return __sync_val_compare_and_swap(&m_state, NotRunning, NotRunning) != NotRunning;
}

bool SynchronizedOperation::IsPending()
{
    return __sync_val_compare_and_swap(&m_state, NotRunning, NotRunning) == Pending;
}

void SynchronizedOperation::Run()
{
    if (__sync_val_compare_and_swap(&m_state, Running, Pending) == NotRunning)
        TryRun();
}

void SynchronizedOperation::TryRun()
{
    if (__sync_val_compare_and_swap(&m_state, NotRunning, Running) == NotRunning)
    {
        while (ExecuteAction())
        {
        }
    }
}

void SynchronizedOperation::RunOnce()
{
    if (__sync_val_compare_and_swap(&m_state, Running, Pending) == NotRunning)
        TryRunOnce();
}

void SynchronizedOperation::RunOnceAsync()
{
    if (__sync_val_compare_and_swap(&m_state, Running, Pending) == NotRunning)
        TryRunOnceAsync();
}

void SynchronizedOperation::TryRunOnce()
{
    if (__sync_val_compare_and_swap(&m_state, NotRunning, Running) == NotRunning)
    {
        if (ExecuteAction())
            ExecuteActionAsync();
    }
}

void SynchronizedOperation::TryRunOnceAsync()
{
    if (__sync_val_compare_and_swap(&m_state, NotRunning, Running) == NotRunning)
        ExecuteActionAsync();
}

bool SynchronizedOperation::ExecuteAction()
{
    m_action(m_arg);
    
    if (__sync_val_compare_and_swap(&m_state, Running, NotRunning) == Pending)
    {
        // There is no race condition here because if m_state is Pending,
        // then it cannot be changed by any other line of code except this one
        __sync_lock_test_and_set(&m_state, Running);
        return true;
    }

    return false;
}

void SynchronizedOperation::ExecuteActionAsync()
{
    Thread* thread = new Thread(&AsyncThreadProc);
    thread->SetArgument(new pair<SynchronizedOperation*, Thread*>(this, thread));
    thread->Start();
}

void SynchronizedOperation::AsyncThreadProc(void* state)
{
    pair<SynchronizedOperation*, Thread*>* parameters = static_cast<pair<SynchronizedOperation*, Thread*>*>(state);

    if (parameters == nullptr)
        throw invalid_argument("SynchronizedOperation::AsyncThreadProc pair based parameters instance is null");
        
    SynchronizedOperation* operation = parameters->first;
    delete parameters->second;
    delete parameters;
    
    while (operation->ExecuteAction())
    {        
    }
}