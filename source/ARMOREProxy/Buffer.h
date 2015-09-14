//******************************************************************************************************
//  Buffer.h - Gbtc
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
//  06/15/2015 - Ritchie
//       Generated original version of source code.
//
//******************************************************************************************************

#ifndef _BUFFER_H
#define _BUFFER_H

#include <sys/types.h>

// Represents a memory reference and length.
class Buffer
{
public:
    // Fields
    u_int8_t* value;
    u_int32_t length;

    // Constructors
    Buffer();
    Buffer(u_int8_t* buffer, u_int32_t size);

    // Methods
    bool IsEmpty() const;

    // Operators

    // Simple comparisons based on length then memcmp
    bool operator<(const Buffer& other) const;  
    bool operator==(const Buffer& other) const;

    // Static

    // Creates an allocated copy of source buffer - it is consumer's responsibility
    // to release the allocated buffer, i.e., "deletebuffer" (see macro below)
    static Buffer Copy(const Buffer& source);

    // Creates an allocated buffer of specified size - it is consumer's responsibility
    // to release the allocated buffer, i.e., "deletebuffer" (see macro below)
    static Buffer Create(const u_int32_t length);

    // Represents an empty buffer, that is, value is null and length is zero
    const static Buffer Empty;
};

#define deletebuffer(name)              \
    if (name.length > 0 && name.value)  \
        delete name.value

#endif