/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates, or 
* a third party where indicated.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,  
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
*
*/

// Original file Copyright Crytek GMBH or its affiliates, used under license.
#include "StdAfx.h"
#include "DX12SamplerState.hpp"

namespace DX12
{
    //---------------------------------------------------------------------------------------------------------------------
    SamplerState::SamplerState()
        : ReferenceCounted()
        , m_DescriptorHandle(INVALID_CPU_DESCRIPTOR_HANDLE)
    {
        // clear before use
        memset(&m_unSamplerDesc, 0, sizeof(m_unSamplerDesc));
    }

    //---------------------------------------------------------------------------------------------------------------------
    SamplerState::~SamplerState()
    {
    }
}
