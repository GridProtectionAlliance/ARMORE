//******************************************************************************************************
//  Timer.h - Gbtc
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

#ifndef _TIMER_H
#define _TIMER_H

#include <time.h>
#include <string>
#include "Thread.h"

typedef void(*TimerCallback)(void* state);

using namespace std;

class Timer
{
public:
    // Constructors    

    /// <summary>Creates a new Timer instance.</summary>
    /// <param name="callback">Function to call when timer expires.</param>
    /// <param name"state">Information to be used by the callback.</param>
    explicit Timer(TimerCallback callback, void* state = nullptr);

    /// <summary>Creates a new Timer instance.</summary>
    /// <param name="callback">Function to call when timer expires.</param>
    /// <param name"state">Information to be used by the callback.</param>
    /// <param name="dueTime">
    /// Specifies the initial expiration of the timer, in seconds and nanoseconds.
    /// Any nonzero value arms the timer.
    /// </param>
    /// <param name="period">
    /// Specifies the seconds and nanoseconds for repeated timer expirations after the
    /// initial expiration, if any. If the <paramref name="dueTime"/> parameter is a
    /// nonzero value and the period is a zero value, the timer will only expire once.
    /// </param>
    Timer(TimerCallback callback, void* state, timespec dueTime, timespec period);
    ~Timer(void);

    // Methods
    
    /// <summary>Changes the Timer interval.</summary>
    /// <param name="dueTime">
    /// Specifies the initial expiration of the timer, in seconds and nanoseconds.
    /// Any nonzero value arms the timer.
    /// </param>
    /// <param name="period">
    /// Specifies the seconds and nanoseconds for repeated timer expirations after the
    /// initial expiration, if any. If the <paramref name="dueTime"/> parameter is a
    /// nonzero value and the period is a zero value, the timer will only expire once.
    /// </param>
    void Change(timespec dueTime, timespec period);

    /// <summary>Stops the Timer.</summary>
    void Stop();
    
    /// <summary>Formats a number of seconds as an elapsed time string.</summary>
    static string ToElapsedTimeString(double seconds, int precision = 2);

private:
    static void TimerPoll(void* arg);

    itimerspec m_timer;
    int m_timerfd;
    Thread* m_timerPollThread;
    TimerCallback m_callback;
    void* m_state;
    volatile bool m_enabled;
};

#endif