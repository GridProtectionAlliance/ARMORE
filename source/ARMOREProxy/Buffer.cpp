//******************************************************************************************************
//  Buffer.cpp - Gbtc
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
//  06/15/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include "Buffer.h"
#include <string.h>

const Buffer Buffer::Empty;

Buffer::Buffer()
{
    value = nullptr;
    length = 0;
}

Buffer::Buffer(u_int8_t* buffer, u_int32_t size)
{
    value = buffer;
    length = size;
}

bool Buffer::IsEmpty() const
{
    return length == 0 || value == nullptr;
}

Buffer Buffer::Copy(const Buffer& source)
{
    if (source.length > 0 && source.value)
    {
        u_int8_t* value = new u_int8_t[source.length];
        memcpy(value, source.value, source.length);
        return Buffer(value, source.length);
    }

    return Empty;
}

Buffer Buffer::Create(const u_int32_t length)
{
    // Note that "()" suffix on new expression zeros out memory per C++ 11 standard
    if (length > 0)
        return Buffer(new u_int8_t[length](), length);

    return Empty;
}

bool Buffer::operator<(const Buffer& other) const
{
    if (length < other.length)
        return true;

    if (length > other.length)
        return false;

    return memcmp(value, other.value, length) < 0;
}

bool Buffer::operator==(const Buffer& other) const
{
    if (length == other.length)
        return memcmp(value, other.value, length) == 0;

    return false;
}
