/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
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
#include "DX12Device.hpp"
#include "DX12Resource.hpp"

#define DX12_GLOBALHEAP_RESOURCES   (1 << D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
#define DX12_GLOBALHEAP_SAMPLERS    (1 << D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
#define DX12_GLOBALHEAP_TYPES       DX12_GLOBALHEAP_RESOURCES

namespace DX12
{
    Device* Device::Create(IDXGIAdapter* pAdapter, D3D_FEATURE_LEVEL* pFeatureLevel)
    {
        ID3D12Device* pDevice12 = NULL;

#if DX12_ENABLE_DEBUG_LAYER > 0
        ID3D12Debug* debugInterface = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
        {
            debugInterface->EnableDebugLayer();
#if DX12_ENABLE_DEBUG_LAYER > 1
            // Enable DX12 GBV as well
            ID3D12Debug1* debugInterface1 = nullptr;
            if (SUCCEEDED(debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1))))
            {
                debugInterface1->SetEnableGPUBasedValidation(true);
                debugInterface1->SetEnableSynchronizedCommandQueueValidation(true);
                debugInterface1->Release();
            }
#endif
            debugInterface->Release();
        }
#endif

        D3D_FEATURE_LEVEL level;
        HRESULT hr =
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            //      (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_11_3, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            //      (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_11_2, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_10_1, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_10_0, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_9_3, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_9_2, IID_PPV_ARGS(&pDevice12)) == S_OK) ||
            (D3D12CreateDevice(pAdapter, level = D3D_FEATURE_LEVEL_9_1, IID_PPV_ARGS(&pDevice12)) == S_OK) ? S_OK : S_FALSE;

        if (hr != S_OK)
        {
            DX12_ASSERT(0, "Failed to create D3D12 Device!");
            return NULL;
        }

#if DX12_USE_DXC == 1
        D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_0};
        if (FAILED(pDevice12->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
        {
            DX12_ASSERT(0, "Shader Model 6.0 is not supported!");
        }
#endif // DX12_USE_DXC

        if (pFeatureLevel)
        {
            *pFeatureLevel = level;
        }
     
        Device* result = new Device(pDevice12);
        pDevice12->Release();

        return DX12::PassAddRef(result);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Device::Device(ID3D12Device* d3d12Device)
        : m_Device(d3d12Device)
        , m_SamplerCache(this)
        , m_ShaderResourceDescriptorCache(this)
        , m_UnorderedAccessDescriptorCache(this)
        , m_DepthStencilDescriptorCache(this)
        , m_RenderTargetDescriptorCache(this)
        , m_GlobalDescriptorHeaps
        {
#ifdef __d3d12_x_h__
            { this },
#endif
            { this },
            { this },
            { this },
            { this }
        }
        , m_FrameCounter(0)
        , m_CalibratedCpuTimestamp{}
        , m_CalibratedCpuTimestampFrequency{}
        , m_CalibratedGpuTimestamp{}
        , m_CalibratedGpuTimestampFrequency{}
        , m_NullSRV{}
        , m_NullUAV{}
        , m_NullSampler{}
    {
        // Anniversary Update
#if NTDDI_WIN10_RS1 && (WDK_NTDDI_VERSION >= NTDDI_WIN10_RS1)
        ID3D12Device1* pDevice1 = nullptr;
        m_Device->QueryInterface(__uuidof(ID3D12Device1), (void**)&pDevice1);
        if (m_Device1 = pDevice1)
            pDevice1->Release();
#endif
            // Creator's Update
#if NTDDI_WIN10_RS2 && (WDK_NTDDI_VERSION >= NTDDI_WIN10_RS2)
        ID3D12Device2* pDevice2 = nullptr;
        m_Device->QueryInterface(__uuidof(ID3D12Device2), (void**)&pDevice2);
        if (m_Device2 = pDevice2)
            pDevice2->Release();
#endif

        // These objects are reference counted, but didn't heap allocate them. We assume ownership.
        {
            m_SamplerCache.AddRef();
            m_ShaderResourceDescriptorCache.AddRef();
            m_UnorderedAccessDescriptorCache.AddRef();
            m_DepthStencilDescriptorCache.AddRef();
            m_RenderTargetDescriptorCache.AddRef();

            for (u32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
            {
                m_GlobalDescriptorHeaps[i].AddRef();
            }
        }

        m_PipelineStateCache.Init(this);
        m_RootSignatureCache.Init(this);

        // init sampler cache
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc;
            ZeroMemory(&desc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

            desc.NumDescriptors = 2048;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            m_SamplerCache.Init(desc);
        }

        // init shader resource descriptor cache
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 65535;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            m_ShaderResourceDescriptorCache.Init(desc);
        }

        // init unordered access descriptor cache
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 4096;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            m_UnorderedAccessDescriptorCache.Init(desc);
        }

        // init depth stencil descriptor cache
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 256;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            m_DepthStencilDescriptorCache.Init(desc);
        }

        // init render target descriptor cache
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 1024;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            m_RenderTargetDescriptorCache.Init(desc);
        }

        // init global descriptor heaps

        static u32 globalHeapSizes[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
        {
            1000000, // D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            1024, // D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
            256,  // D3D12_DESCRIPTOR_HEAP_TYPE_RTV
            256   // D3D12_DESCRIPTOR_HEAP_TYPE_DSV
        };

        for (D3D12_DESCRIPTOR_HEAP_TYPE eType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; eType < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; eType = D3D12_DESCRIPTOR_HEAP_TYPE(eType + 1))
        {
            if (DX12_GLOBALHEAP_TYPES & (1 << eType))
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.NumDescriptors = globalHeapSizes[eType];
                desc.Type = eType;
                desc.Flags = (eType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || eType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

                m_GlobalDescriptorHeaps[eType].Init(desc);
            }
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = DXGI_FORMAT_R32_UINT;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        m_NullSRV = CacheShaderResourceView(&SRVDesc, nullptr);

        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.Format = DXGI_FORMAT_R32_UINT;
        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        m_NullUAV = CacheUnorderedAccessView(&UAVDesc, nullptr);

        D3D12_SAMPLER_DESC SamplerDesc = {};
       
        SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;

        SamplerDesc.MinLOD = 0;
        SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

        m_NullSampler = CacheSampler(&SamplerDesc);
    }

    Device::~Device()
    {
        // Free all resources in the m_ReleaseHeap and m_RecycleHeap
        FlushReleaseHeap(ResourceReleasePolicy::Immediate);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Device::CacheSampler(const D3D12_SAMPLER_DESC* pDesc)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        DX12_ASSERT(m_SamplerCache.GetCursor() < m_SamplerCache.GetCapacity());

        D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = m_SamplerCache.GetHandleOffsetCPU(0);
        GetD3D12Device()->CreateSampler(pDesc, dstHandle);
        m_SamplerCache.IncrementCursor();

        return dstHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Device::CacheShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D12Resource* pResource)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        bool useCached = false;
        D3D12_CPU_DESCRIPTOR_HANDLE dstHandle;
        if (m_DescriptorPools[DescriptorPoolSRV].size())
        {
            auto it = m_DescriptorPools[DescriptorPoolSRV].front();
            if (m_FrameCounter > (it.first + AllowedGPUFramesLatency))
            {
                useCached = true;
                m_DescriptorPools[DescriptorPoolSRV].pop_front();
                dstHandle = m_ShaderResourceDescriptorCache.GetHandleOffsetCPU_R(it.second);
            }
        }
        if(!useCached)
        {
            DX12_ASSERT(m_ShaderResourceDescriptorCache.GetCursor() < m_ShaderResourceDescriptorCache.GetCapacity());
            dstHandle = m_ShaderResourceDescriptorCache.GetHandleOffsetCPU(0);
            m_ShaderResourceDescriptorCache.IncrementCursor();
        }

        GetD3D12Device()->CreateShaderResourceView(pResource, pDesc, dstHandle);
        

        return dstHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Device::CacheUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D12Resource* pResource)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        bool useCached = false;
        D3D12_CPU_DESCRIPTOR_HANDLE dstHandle;
        if (m_DescriptorPools[DescriptorPoolUAV].size())
        {
            auto it = m_DescriptorPools[DescriptorPoolUAV].front();
            if (m_FrameCounter > (it.first + AllowedGPUFramesLatency))
            {
                useCached = true;
                m_DescriptorPools[DescriptorPoolUAV].pop_front();
                dstHandle = m_UnorderedAccessDescriptorCache.GetHandleOffsetCPU_R(it.second);
            }
        }
        if (!useCached)
        {
            DX12_ASSERT(m_UnorderedAccessDescriptorCache.GetCursor() < m_UnorderedAccessDescriptorCache.GetCapacity());
            dstHandle = m_UnorderedAccessDescriptorCache.GetHandleOffsetCPU(0);
            m_UnorderedAccessDescriptorCache.IncrementCursor();
        }
        GetD3D12Device()->CreateUnorderedAccessView(pResource, nullptr, pDesc, dstHandle);
        
        return dstHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Device::CacheDepthStencilView(const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D12Resource* pResource)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        DX12_ASSERT(m_DepthStencilDescriptorCache.GetCursor() < m_DepthStencilDescriptorCache.GetCapacity());

        D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = m_DepthStencilDescriptorCache.GetHandleOffsetCPU(0);
        GetD3D12Device()->CreateDepthStencilView(pResource, pDesc, dstHandle);
        m_DepthStencilDescriptorCache.IncrementCursor();

        return dstHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Device::CacheRenderTargetView(const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, ID3D12Resource* pResource)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        DX12_ASSERT(m_RenderTargetDescriptorCache.GetCursor() < m_RenderTargetDescriptorCache.GetCapacity());

        D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = m_RenderTargetDescriptorCache.GetHandleOffsetCPU(0);
        GetD3D12Device()->CreateRenderTargetView(pResource, pDesc, dstHandle);
        m_RenderTargetDescriptorCache.IncrementCursor();

        return dstHandle;
    }

    void Device::RecycleDescriptorHandle(DescriptorPoolType poolType, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        u32 offset;
        switch (poolType)
        {
        case DescriptorPoolSRV:
            offset = m_ShaderResourceDescriptorCache.GetOffsetFromCPUHandle(handle);
            break;
        case DescriptorPoolUAV:
            offset = m_UnorderedAccessDescriptorCache.GetOffsetFromCPUHandle(handle);
            break;
        default:
            R_ASSERT2(0, "DescriptorPoolType not supported!");
        }
        
        m_DescriptorPools[poolType].push_back(std::pair<u32, u32>(m_FrameCounter, offset));
    }

    void Device::FinishFrame()
    {
        m_FrameCounter++;
    }

    HRESULT STDMETHODCALLTYPE Device::CreateOrReuseCommittedResource(
        _In_  const D3D12_HEAP_PROPERTIES* pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        _In_  const D3D12_RESOURCE_DESC* pResourceDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riidResource,
        _COM_Outptr_opt_  void** ppvResource,
        ResourceStates& resourceStates)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);
       
        struct
        {
            D3D12_HEAP_FLAGS sHeapFlags;
            D3D12_HEAP_PROPERTIES sHeapProperties;
            D3D12_RESOURCE_DESC sResourceDesc;
        }
        hashableBlob;

        hashableBlob.sHeapProperties = *pHeapProperties;
        hashableBlob.sResourceDesc = *pResourceDesc;
        hashableBlob.sHeapFlags = HeapFlags;

        // Clear spaces from alignment of members
        void* ptr1 = ((char*)&hashableBlob.sResourceDesc.Dimension) + sizeof(hashableBlob.sResourceDesc.Dimension);
        ZeroMemory(ptr1, offsetof(D3D12_RESOURCE_DESC, Alignment) - sizeof(hashableBlob.sResourceDesc.Dimension));
        void* ptr2 = ((char*)&hashableBlob.sResourceDesc.Flags) + sizeof(hashableBlob.sResourceDesc.Flags);
        ZeroMemory(ptr2, sizeof(hashableBlob.sResourceDesc) - offsetof(D3D12_RESOURCE_DESC, Flags) - sizeof(hashableBlob.sResourceDesc.Flags));

        THash hHash = ComputeSmallHash<sizeof(hashableBlob)>(&hashableBlob);
        
        auto result = m_RecycleHeap.find(hHash);
        if (result != m_RecycleHeap.end())
        {
            if (ppvResource)
            {
                *ppvResource = result->second.resource;
                resourceStates = result->second.resourceStates;
                m_RecycleHeap.erase(result);
            }
            
            return S_OK;
        }

        auto res = GetD3D12Device()->CreateCommittedResource(
                pHeapProperties, HeapFlags, pResourceDesc, InitialResourceState,
                pOptimizedClearValue, riidResource, ppvResource);
        resourceStates.m_CurrentState = InitialResourceState;
        resourceStates.m_AnnouncedState = static_cast<D3D12_RESOURCE_STATES>(-1);

        return res;
    }

    void Device::FlushReleaseHeap(ResourceReleasePolicy releasePolicy)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        const u32 RESOURCE_RECYCLE_LATENCY = 8;
        const u32 RESOURCE_RELEASE_LATENCY = 32;
        {
            auto it = m_ReleaseHeap.begin();
            while (it != m_ReleaseHeap.end())
            {
                u32 resourceFrameNumber = it->second.frameNumber;
                ID3D12Resource* resource = it->second.resource;
                if (m_FrameCounter > (resourceFrameNumber + RESOURCE_RECYCLE_LATENCY) || releasePolicy == ResourceReleasePolicy::Immediate)
                {
                    m_RecycleHeap.insert(*it);
                    it = m_ReleaseHeap.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
        {
            auto it = m_RecycleHeap.begin();
            while (it != m_RecycleHeap.end())
            {
                u32 resourceFrameNumber = it->second.frameNumber;
                if (m_FrameCounter > (resourceFrameNumber + RESOURCE_RELEASE_LATENCY) || releasePolicy == ResourceReleasePolicy::Immediate)
                {
                    it->second.resource->Release();
                    it = m_RecycleHeap.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
    }

    void Device::ReleaseLater(ID3D12Resource* object, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES announcedState)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        if (object)
        {
            struct
            {
                D3D12_HEAP_FLAGS sHeapFlags;
                D3D12_HEAP_PROPERTIES sHeapProperties;
                D3D12_RESOURCE_DESC sResourceDesc;
            }
            hashableBlob;

            object->GetHeapProperties(&hashableBlob.sHeapProperties, &hashableBlob.sHeapFlags);
            hashableBlob.sResourceDesc = object->GetDesc();
            // When creating a committed resource, D3D12_HEAP_FLAGS must not have either D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES,
            // D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES, nor D3D12_HEAP_FLAG_DENY_BUFFERS set. These flags will be set automatically
            // to correspond with the committed resource type.
            hashableBlob.sHeapFlags = D3D12_HEAP_FLAGS(hashableBlob.sHeapFlags & ~(D3D12_HEAP_FLAG_DENY_BUFFERS + D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES + D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES));

            // Clear spaces from alignment of members
            void* ptr1 = ((char*)&hashableBlob.sResourceDesc.Dimension) + sizeof(hashableBlob.sResourceDesc.Dimension);
            ZeroMemory(ptr1, offsetof(D3D12_RESOURCE_DESC, Alignment) - sizeof(hashableBlob.sResourceDesc.Dimension));
            void* ptr2 = ((char*)&hashableBlob.sResourceDesc.Flags) + sizeof(hashableBlob.sResourceDesc.Flags);
            ZeroMemory(ptr2, sizeof(hashableBlob.sResourceDesc) - offsetof(D3D12_RESOURCE_DESC, Flags) - sizeof(hashableBlob.sResourceDesc.Flags));

            ReleaseInfo releaseInfo;

            THash hash = ComputeSmallHash<sizeof(hashableBlob)>(&hashableBlob);

            releaseInfo.resource = object;
            releaseInfo.resourceStates.m_CurrentState = currentState;
            releaseInfo.resourceStates.m_AnnouncedState = announcedState;
            releaseInfo.frameNumber = m_FrameCounter;

            m_ReleaseHeap.insert(std::make_pair(hash, std::move(releaseInfo)));
        }
    }

    DescriptorBlock Device::GetGlobalDescriptorBlock(D3D12_DESCRIPTOR_HEAP_TYPE eType, u32 size)
    {
        static CryCriticalSectionNonRecursive csThreadSafeScope;
        CryAutoLock<CryCriticalSectionNonRecursive> lThreadSafeScope(csThreadSafeScope);

        if (DX12_GLOBALHEAP_TYPES & (1 << eType))
        {
            R_ASSERT2(m_GlobalDescriptorHeaps[eType].GetCapacity() - m_GlobalDescriptorHeaps[eType].GetCursor() >= size, "Exceeded capacity");
            DescriptorBlock result(&m_GlobalDescriptorHeaps[eType], m_GlobalDescriptorHeaps[eType].GetCursor(), size);
            m_GlobalDescriptorHeaps[eType].IncrementCursor(size);
            return result;
        }

        DescriptorHeap* pResourceHeap = DX12::PassAddRef(new DescriptorHeap(this));

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = size;
        desc.Type = eType;
        desc.Flags = (eType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || eType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        pResourceHeap->Init(desc);

        DescriptorBlock result(pResourceHeap, pResourceHeap->GetCursor(), size);
        pResourceHeap->Release();
        return result;
    }

    void Device::CalibrateClocks(ID3D12CommandQueue* presentQueue)
    {
        presentQueue->GetClockCalibration(&m_CalibratedGpuTimestamp, &m_CalibratedCpuTimestamp);
        presentQueue->GetTimestampFrequency(&m_CalibratedGpuTimestampFrequency);

        QueryPerformanceFrequency((LARGE_INTEGER*)&m_CalibratedCpuTimestampFrequency);
    }

    u64 Device::MakeCpuTimestamp(u64 gpuTimestamp) const
    {
        INT64 timestampDelta = ((INT64)gpuTimestamp - (INT64)m_CalibratedGpuTimestamp);
        INT64 calibratedTimestampDelta = (timestampDelta * (INT64)m_CalibratedCpuTimestampFrequency) / (INT64)m_CalibratedGpuTimestampFrequency;
        return m_CalibratedCpuTimestamp + calibratedTimestampDelta;
    }

    u64 Device::MakeCpuTimestampMicroseconds(u64 gpuTimestamp) const
    {
        return (MakeCpuTimestamp(gpuTimestamp) * 1000) / (m_CalibratedCpuTimestampFrequency / 1000);
    }
}
