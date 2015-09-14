//******************************************************************************************************
//  Lock.cpp - Gbtc
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

#include "Lock.h"
#include <string.h>
#include <stdexcept>

using namespace std;

Lock::Lock(bool recursive)
{
    pthread_mutexattr_t attributes;
    pthread_mutexattr_init(&attributes);

    // Set a recursive mutex if requested (default)
    if (recursive)
        pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
    else
        pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_ERRORCHECK);

    int err = pthread_mutex_init(&m_lock, &attributes);

    if (err != 0)
        throw runtime_error("Failed to create mutex for Lock: " + string(strerror(err)));
}

Lock::~Lock()
{
    pthread_mutex_destroy(&m_lock);
}

void Lock::Acquire()
{
    int err = pthread_mutex_lock(&m_lock);

    if (err != 0)
        throw runtime_error("Failed to acquire mutex for Lock: " + string(strerror(err)));
}

bool Lock::TryAcquire(int* errout)
{
    int err = pthread_mutex_trylock(&m_lock);

    if (errout != nullptr)
        *errout = err;

    return err == 0;
}

void Lock::Release()
{
    int err = pthread_mutex_unlock(&m_lock);

    if (err != 0)
        throw runtime_error("Failed to release mutex for Lock: " + string(strerror(err)));
}

//******************************************************************************************************
//
//  LocalLock Implementation
//
//******************************************************************************************************

LocalLock::LocalLock(Lock* lock)
{
    m_acquired = false;
    m_lock = lock;
}

LocalLock::~LocalLock()
{
    if (m_acquired)
        Release();
}

void LocalLock::Acquire()
{
    // Use case of LocalLock is a simple acquire and release, recursive
    // lock using this RAII safety wrapper is a miss-use of class
    if (m_acquired)
        throw logic_error("LocalLock already acquired - cannot reacquire.");

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

bool LocalLock::TryAcquire()
{
    // Use case of LocalLock is a simple acquire and release, recursive
    // lock using this RAII safety wrapper is a miss-use of class
    if (m_acquired)
        throw logic_error("LocalLock already acquired - cannot reacquire.");

    m_acquired = m_lock->TryAcquire();
    return m_acquired;
}

void LocalLock::Release()
{
    if (!m_acquired)
        throw logic_error("LocalLock not acquired - cannot release.");

    m_acquired = false;
    m_lock->Release();
}
    