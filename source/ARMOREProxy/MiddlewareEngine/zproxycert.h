//******************************************************************************************************
//  zproxycert.h - Gbtc
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
//  09/11/2015 - J. Ritchie Carroll
//       Created new actor based on https://github.com/zeromq/czmq/blob/master/src/zproxy.c
//
//******************************************************************************************************

#ifndef _ZPROXYCERT_H_
#define _ZPROXYCERT_H_

#ifdef __cplusplus
extern "C" {
#endif

// Extends zproxy actor functionality by allowing ability to specify curve security certificate
// that will get applied to frontend socket. Since this is a custom actor for this project, any
// unused functionality of original zproxy actor was removed.
void zproxycert(zsock_t* pipe, void* certificate);
    
#ifdef __cplusplus
}
#endif

#endif