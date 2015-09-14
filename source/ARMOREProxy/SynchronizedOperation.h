//******************************************************************************************************
//  SynchronizedOperation.h - Gbtc
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

#ifndef _SYNCHRONIZEDOPERATION_H
#define _SYNCHRONIZEDOPERATION_H

typedef void(*Action)(void* arg);

class SynchronizedOperation
{
public:
    // Constructors
    SynchronizedOperation(Action action, void* arg = nullptr);
    ~SynchronizedOperation();
    
    // Methods
    
    // Gets a value to indicate whether the synchronized operation
    // is currently executing its action.
    bool IsRunning();
    
    // Gets a value to indicate whether the synchronized operation
    // has an additional operation that is pending execution after
    // the currently running action has completed.
    bool IsPending();
    
    // Executes the action on this thread or marks the operation
    // as pending if the operation is already running.
    void Run();
    
    // Attempts to execute the action on this thread. Does nothing
    // if the operation is already running.
    void TryRun();
    
    // Executes the action on this thread or marks the operation
    // as pending if the operation is already running.
    void RunOnce();
    
    // Executes the action on another thread or marks the operation
    // as pending if the operation is already running.
    void RunOnceAsync();
    
    // Attempts to execute the action on this thread. Does nothing
    // if the operation is already running.
    void TryRunOnce();
    
    // Attempts to execute the action on another thread. Does
    // nothing if the operation is already running.
    void TryRunOnceAsync();

private:
    bool ExecuteAction();
    void ExecuteActionAsync();
    static void AsyncThreadProc(void* state);

    Action m_action;
    int m_state;
    void* m_arg;
};

#endif