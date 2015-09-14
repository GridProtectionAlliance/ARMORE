//******************************************************************************************************
//  SpinLock.cpp - Gbtc
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
//  07/17/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "SpinLock.h"
#include <string.h>
#include <stdexcept>

using namespace std;

SpinLock::SpinLock()
{
    int err = pthread_spin_init(&m_lock, 0);

    if (err != 0)
        throw runtime_error("Failed to create spinning lock: " + string(strerror(err)));
}

SpinLock::~SpinLock()
{
    pthread_spin_destroy(&m_lock);
}

void SpinLock::Acquire()
{
    int err = pthread_spin_lock(&m_lock);

    if (err != 0)
        throw runtime_error("Failed to acquire spinning lock: " + string(strerror(err)));
}

bool SpinLock::TryAcquire(int* errout)
{
    int err = pthread_spin_trylock(&m_lock);

    if (errout != nullptr)
        *errout = err;

    return err == 0;
}

void SpinLock::Release()
{
    int err = pthread_spin_unlock(&m_lock);

    if (err != 0)
        throw runtime_error("Failed to release spinning lock: " + string(strerror(err)));
}

//******************************************************************************************************
//
//  LocalSpinLock Implementation
//
//******************************************************************************************************

LocalSpinLock::LocalSpinLock(SpinLock* lock)
{
    m_acquired = false;
    m_lock = lock;
}

LocalSpinLock::~LocalSpinLock()
{
    if (m_acquired)
        Release();
}

void LocalSpinLock::Acquire()
{
    // Use case of LocalSpinLock is a simple acquire and release, recursive
    // lock using this RAII safety wrapper is a miss-use of class
    if (m_acquired)
        throw logic_error("LocalSpinLock already acquired - cannot reacquire.");

    try
    {
        m_lock->Acquire();
        m_acquired = true;
    }
    catch (...)
    {
        m_acquired = false;
        throw;
    }
}

bool LocalSpinLock::TryAcquire()
{
    // Use case of LocalSpinLock is a simple acquire and release, recursive
    // lock using this RAII safety wrapper is a miss-use of class
    if (m_acquired)
        throw logic_error("LocalSpinLock already acquired - cannot reacquire.");

    m_acquired = m_lock->TryAcquire();
    return m_acquired;
}

void LocalSpinLock::Release()
{
    if (!m_acquired)
        throw logic_error("LocalSpinLock not acquired - cannot release.");

    m_acquired = false;
    m_lock->Release();
}
    