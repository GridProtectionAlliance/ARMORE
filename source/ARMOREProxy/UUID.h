//******************************************************************************************************
//  UUID.h - Gbtc
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
//  06/11/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#ifndef _UUID_H
#define _UUID_H

#include <string>
#include <uuid/uuid.h>

using namespace std;

struct UUID
{
    //Fields
    uuid_t value;

    // Constructors
    UUID();
    UUID(const UUID& other);
    UUID(const uuid_t other);
    UUID(const string in);
    
    // Methods
    void CopyTo(uuid_t dst);
    bool IsEmpty() const;
    string ToString() const;

    // Operators
    UUID& operator=(const UUID& other);
    bool operator<(const UUID& other) const;  
    bool operator==(const UUID& other) const;

    // Static
    static UUID NewUUID();
    const static UUID Empty;
};

ostream& operator << (ostream&, const UUID&);

// Create hash function for UUID
namespace std
{
    template<> struct hash<UUID>
    {
        size_t operator()(const UUID& id) const
        {
            size_t seed = 0;
        
            for (int i = 0; i < 16; ++i)
                seed ^= static_cast<size_t>(id.value[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}

#endif