//******************************************************************************************************
//  TraceFunctions.h - Gbtc
//
//  Copyright � 2015, Grid Protection Alliance.  All Rights Reserved.
//
//  Licensed to the Grid Protection Alliance (GPA) under one or more contributor license agreements. See
//  the NOTICE file distributed with this work for additional information regarding copyright ownership.
//  The GPA licenses this file to you under the MIT License (MIT), the "License"; you may
//  not use this file except in compliance with the License. You may obtain a copy of the License at:
//
//      http://www.opensource.org/licenses/MIT
//
//  Unless agreed to in writing, the subject software distributed under the License is distributed on an
//  "AS-IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. Refer to the
//  License for the specific language governing permissions and limitations.
//
//  Code Modification History:
//  ----------------------------------------------------------------------------------------------------
//  04/23/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#ifndef _TRACEFUNCTIONS_H
#define _TRACEFUNCTIONS_H

#include <sys/types.h>

void GetIPv4PacketSummary(const u_int8_t* buffer, char* destination);
void GetIPv6PacketSummary(const u_int8_t* buffer, char* destination);
void DescribePacket(const u_int8_t* data, const char* io);

#endif