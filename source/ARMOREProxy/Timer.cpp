//******************************************************************************************************
//  Timer.cpp - Gbtc
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

#include "Timer.h"

#include <sys/timerfd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "ConsoleLogger/ConsoleLogger.h"

using namespace std;

Timer::Timer(TimerCallback callback, void* state)
{
    m_callback = callback;
    m_state = state;
    m_timerPollThread = nullptr;

    // Create timer
    m_timerfd = timerfd_create(CLOCK_MONOTONIC, 0);

    // Make sure timer is in blocking mode
    int flags = fcntl(m_timerfd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(m_timerfd, F_SETFL, flags);

    if (m_timerfd == -1)
        throw runtime_error("Failed to create Timer");
}

Timer::Timer(TimerCallback callback, void* state, timespec dueTime, timespec period) : 
    Timer(callback, state)
{
    if (dueTime.tv_sec != 0 || dueTime.tv_nsec != 0 || period.tv_sec != 0 || period.tv_nsec != 0)
        Change(dueTime, period);
}

Timer::~Timer(void)
{
    Stop();
}

void Timer::Change(timespec dueTime, timespec period)
{
    int err;

    // Set timer periods
    m_timer.it_value = dueTime;
    m_timer.it_interval = period;

    err = timerfd_settime(m_timerfd, 0, &m_timer, nullptr);

    if (err != 0)
        throw runtime_error("Failed to set Timer interval: " + string(strerror(err)));

    if (dueTime.tv_sec == 0 && dueTime.tv_nsec == 0 && period.tv_sec == 0 && period.tv_nsec == 0)
    {
        m_enabled = false;

        // Terminate any existing timer polling thread
        if (m_timerPollThread)
        {
            delete m_timerPollThread;
            m_timerPollThread = nullptr;
        }
    }
    else
    {
        // Create timer polling thread
        m_timerPollThread = new Thread(&TimerPoll, this, true);    
        m_enabled = true;
    }
}

void Timer::Stop()
{
    timespec dueTime, period;

    // A value of zero for dueTime and period will stop timer
    memset(&dueTime, 0, sizeof(timespec));
    memset(&period, 0, sizeof(timespec));

    Change(dueTime, period);
}

void Timer::TimerPoll(void* arg)
{
    Timer* timer = static_cast<Timer*>(arg);

    if (timer == nullptr)
        throw invalid_argument("Timer::TimerPoll Timer instance is null");

    u_int64_t expirations = 0;

    while (timer->m_enabled)
    {
        if (read(timer->m_timerfd, &expirations, sizeof(u_int64_t)) > 0 && expirations > 0)
        {
            try
            {
                timer->m_callback(timer->m_state);
            }
            catch (exception& ex)
            {
                log_error("Timer::TimerPoll exception: %s\n", ex.what());
                raise(SIGTERM);
            }
        }
    }
}

// This would be better associated with a date/time class...
string Timer::ToElapsedTimeString(double seconds, int precision)
{
    stringstream fmtstr;
                
    if (seconds < 60.0)
    {
        fmtstr << setprecision(precision) << seconds;
        return fmtstr.str() + " seconds";
    }
    
    if (seconds < 3600.0)
    {
        fmtstr << setprecision(precision) << seconds / 60.0;
        return fmtstr.str() + " minutes";
    }

    if (seconds < 86400.0)
    {
        fmtstr << setprecision(precision) << seconds / 3600.0;
        return fmtstr.str() + " hours";    
    }
    
    fmtstr << setprecision(precision) << seconds / 86400.0;
    return fmtstr.str() + " days";   
}