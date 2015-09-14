//******************************************************************************************************
//  SpinLock.h - Gbtc
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

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include <pthread.h>

// Represents a synchronization primitive based on a spinning lock used to allow synchronized
// access to resource.
class SpinLock
{
public:
    // Constructors
    explicit SpinLock();
    ~SpinLock();

    // Methods
    void Acquire();
    bool TryAcquire(int* err = nullptr);
    void Release();

private:
    pthread_spinlock_t m_lock;
};

// Represents a reference to a shared SpinLock instance, with a normal use case of
// local stack allocation, for RAII purposes that will make sure any acquired
// lock is released even when an exception is thrown.
class LocalSpinLock
{
public:
    explicit LocalSpinLock(SpinLock* lock);
    LocalSpinLock(const LocalSpinLock&) = delete;
    ~LocalSpinLock();

    void Acquire();
    bool TryAcquire();
    void Release();
    
private:
    SpinLock* m_lock;
    volatile bool m_acquired;
};

#ifndef _LABELCAT
#define _LABELCAT(x,y) x##y
#endif
#ifndef LABELCAT
#define LABELCAT(x,y) _LABELCAT(x,y)
#endif
#ifndef UNIQUELABEL
#define UNIQUELABEL(x) LABELCAT(x,__LINE__)
#endif

// Define handy lock macros for wrapping critical section with a SpinLock instance.
// The SpinLock will be constrained within the context of the wrapped code unless there
// is an exception, in which case the LocalSpinLock destructor will manage the release.
#define spinlock(name, ...)                     \
    LocalSpinLock UNIQUELABEL(_lock)(&name);    \
    UNIQUELABEL(_lock).Acquire();           \
    __VA_ARGS__                             \
    UNIQUELABEL(_lock).Release()

#define tryspinlock(name, ...)                  \
    LocalSpinLock UNIQUELABEL(_lock)(&name);    \
    if (UNIQUELABEL(_lock).TryAcquire()) {  \
        __VA_ARGS__                         \
        UNIQUELABEL(_lock).Release();       \
    }

#endif