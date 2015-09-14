//******************************************************************************************************
//  UUID.cpp - Gbtc
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

#include "UUID.h"
#include <uuid/uuid.h>
#include <string>

const UUID UUID::Empty = UUID();

UUID::UUID()
{
    uuid_clear(value);
}

UUID::UUID(const UUID& other)
{
    uuid_copy(value, other.value);
}

UUID::UUID(const uuid_t other)
{
    uuid_copy(value, other);
}

UUID::UUID(const string in)
{
    uuid_parse(in.c_str(), value);
}

void UUID::CopyTo(uuid_t dst)
{
    uuid_copy(dst, value);
}

bool UUID::IsEmpty() const
{
    return uuid_is_null(value) == 1;
}

string UUID::ToString() const
{
    char image[37];
    uuid_unparse(value, image);
    return image;
}

UUID UUID::NewUUID()
{
    UUID value;
    uuid_generate(value.value);
    return value;
}

UUID& UUID::operator=(const UUID& other)
{
    uuid_copy(value, other.value);
    return *this;
}

bool UUID::operator<(const UUID& other) const
{
    return uuid_compare(value, other.value) < 0;
}

bool UUID::operator==(const UUID& other) const
{
    return uuid_compare(value, other.value) == 0;
}

ostream& operator << (ostream& strm, UUID& id)
{
    return strm << id.ToString();
}
