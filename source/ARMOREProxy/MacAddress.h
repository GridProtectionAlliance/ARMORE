//******************************************************************************************************
//  MacAddress.h - Gbtc
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
//  04/23/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#ifndef _MACADDRESS_H
#define _MACADDRESS_H

#include <sys/types.h>
#include <iosfwd>

using namespace std;

struct MacAddress
{
    // Fields
    u_int8_t value[8];

    // Constructors
    MacAddress();
    ~MacAddress();
    
    // Methods
    u_int64_t ToULong() const;
    void CopyTo(u_int8_t* address) const;
    string ToString() const;

    // Operators
    bool operator<(const MacAddress& other) const;
    bool operator==(const MacAddress& other) const;
    bool operator!=(const MacAddress& other) const;

    // Static
    static MacAddress InterfaceLookup(const string interfaceName);
    static MacAddress Parse(const string address);
    static MacAddress CopyFrom(const u_int8_t* address);
    static MacAddress FromULong(u_int64_t address);

    const static MacAddress Broadcast;
    const static MacAddress Zero;
};

ostream& operator<< (ostream&, const MacAddress&);

#endif