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
#pragma once
#ifndef __DX12SWAPCHAIN__
#define __DX12SWAPCHAIN__

#include "DX12.hpp"
#include "DX12Device.hpp"
#include "DX12CommandList.hpp"
#include "DX12AsyncCommandQueue.hpp"

#include <vector>

namespace DX12
{
class SwapChain : public ReferenceCounted
{
public:

    static SwapChain* Create(CommandList* pDevice, IDXGIFactory4* factory, DXGI_SWAP_CHAIN_DESC* pDesc);
    static SwapChain* CreateForHwnd(CommandList* pDevice, IDXGIFactory4* factory, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput);
    static SwapChain* CreateForCoreWindow(CommandList* pDevice, IDXGIFactory4* factory, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc, IDXGIOutput* pRestrictToOutput);

protected:
    SwapChain(CommandList* pDevice, IDXGISwapChain3* dxgiSwapChain, DXGI_SWAP_CHAIN_DESC* pDesc);

    virtual ~SwapChain();

public:
    inline IDXGISwapChain3* GetDXGISwapChain() const
    { 
        return m_NativeSwapChain;
    }

    inline Resource& GetBackBuffer(u32 index)
    {
        Resource* resource = m_BackBuffers[index];
        R_ASSERT2(resource, "null backbuffer");
        return *resource;
    }

    inline Resource& GetCurrentBackBuffer()
    {
        Resource* resource = m_BackBuffers[m_NativeSwapChain->GetCurrentBackBufferIndex()];
        R_ASSERT2(resource, "Resource is null");
        return *resource;
    }

    inline ResourceView& GetBackBufferView(u32 index)
    {
        return m_BackBufferViews[index];
    }

    inline ResourceView& GetCurrentBackBufferView()
    {
        return m_BackBufferViews[m_NativeSwapChain->GetCurrentBackBufferIndex()];
    }

    inline u32 GetCurrentBackBufferIndex() const
    {
        return m_NativeSwapChain->GetCurrentBackBufferIndex();
    }

    inline const DXGI_SWAP_CHAIN_DESC& GetDesc() const 
    {
        return m_Desc; 
    }

    HRESULT Present(u32 SyncInterval, u32 Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters = NULL);

    IC HRESULT SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
    {
        m_Desc.Windowed = !Fullscreen;
        return m_NativeSwapChain->SetFullscreenState(Fullscreen, pTarget);
    }

    HANDLE GetFrameLatencyWaitableObject();

    HRESULT ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters);
    HRESULT ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags, const UINT* pCreationNodeMask = NULL, IUnknown* const* ppPresentQueue = NULL);

private:
    friend class CCryDX12SwapChain;
    void AcquireBuffers(std::vector<Resource*>&& resource);
    void ForfeitBuffers();

    AsyncCommandQueue& m_AsyncQueue;
    CommandList* m_CommandList;

    DXGI_SWAP_CHAIN_DESC m_Desc;

    _smart_ptr<IDXGISwapChain3> m_NativeSwapChain;

    std::vector<Resource*> m_BackBuffers;
    std::vector<ResourceView> m_BackBufferViews;
};
} 
#endif // __DX12SWAPCHAIN__
