#include "stdafx.h"

#include "dx11HW.h"

#include "StateManager/dx11SamplerStateCache.h"
#include "dx11TextureUtils.h"

#include <SDL_syswm.h>

CHW HW;

#if USE_DX12
#define FRAME_LATENCY_WAITABLE_OBJECT 0
#endif

CHW::CHW()
{
    if (!ThisInstanceIsGlobal())
        return;

    Device.seqAppActivate.Add(this);
    Device.seqAppDeactivate.Add(this);
}

CHW::~CHW()
{
    if (!ThisInstanceIsGlobal())
        return;

    Device.seqAppActivate.Remove(this);
    Device.seqAppDeactivate.Remove(this);
}

void CHW::OnAppActivate()
{
    if (m_pSwapChain && !m_ChainDesc.Windowed)
    {
        ShowWindow(m_ChainDesc.OutputWindow, SW_RESTORE);
        m_pSwapChain->SetFullscreenState(
            ThisInstanceIsGlobal() ? psDeviceMode.WindowStyle == rsFullscreen : false, NULL);
#if defined(USE_DX12)
        m_pSwapChain->ResizeTarget(&m_ChainDesc.BufferDesc);
#endif
    }
}

void CHW::OnAppDeactivate()
{
    if (m_pSwapChain && !m_ChainDesc.Windowed)
    {
        m_pSwapChain->SetFullscreenState(FALSE, NULL);
        if (psDeviceMode.WindowStyle == rsFullscreen || psDeviceMode.WindowStyle == rsFullscreenBorderless)
            ShowWindow(m_ChainDesc.OutputWindow, SW_MINIMIZE);
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CHW::CreateD3D()
{
#if defined(USE_DX11)
    
    hDXGI = XRay::LoadModule("dxgi");
    hD3D = XRay::LoadModule("d3d11");

    if (!hD3D->IsLoaded() || !hDXGI->IsLoaded())
    {
        Valid = false;
        return;
    }
#endif // defined(USE_DX11)

#if defined(USE_DX12)
    // Минимально поддерживаемая версия Windows => Windows Vista SP2 или Windows 7.
    DX12CreateDXGIFactory(__uuidof(IDXGIFactory4), (void**)(&m_pFactory));
#else
    // Минимально поддерживаемая версия Windows => Windows Vista SP2 или Windows 7.
    const auto createDXGIFactory =
        static_cast<decltype(&CreateDXGIFactory1)>(hDXGI->GetProcAddress("CreateDXGIFactory1"));
    if (createDXGIFactory)
        createDXGIFactory(__uuidof(IDXGIFactory1), (void**)(&m_pFactory));
#endif

    if (m_pFactory)
        m_pFactory->EnumAdapters1(0, &m_pAdapter);

    Valid = m_pAdapter;
}

#include <dxgidebug.h>

void CHW::DestroyD3D()
{
    _SHOW_REF("refCount:m_pAdapter", m_pAdapter);
    _RELEASE(m_pAdapter);

    _SHOW_REF("refCount:m_pFactory", m_pFactory);
    _RELEASE(m_pFactory);

#if defined(USE_DX11)
    // Manually close and unload additional DLLs
    // To make it work with DXVK, etc.
    hD3D->Close();
    hDXGI->Close();

    if (auto hModule = GetModuleHandleA("d3d11.dll"))
        FreeLibrary(hModule);
    if (auto hModule = GetModuleHandleA("dxgi.dll"))
        FreeLibrary(hModule);
#endif

#if DEBUG
#if USE_DX12
    typedef HRESULT(__stdcall * fPtr)(const IID&, void**);
    HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
    fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");
    IDXGIDebug* pDxgiDebug;
    DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDxgiDebug);
    pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
#endif
}

void CHW::CreateDevice(SDL_Window* sdlWnd)
{
    CreateD3D();
    if (!Valid)
        return;

    m_DriverType = Caps.bForceGPU_REF ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_HARDWARE;

    // Display the name of video board
    DXGI_ADAPTER_DESC1 Desc{};
    if (FAILED(m_pAdapter->GetDesc1(&Desc)))
        Msg("! [%s] failed to retrieve adapter description", __FUNCTION__);
    //  Warning: Desc.Description is wide string
    Msg("* GPU [vendor:%X]-[device:%X]: %S", Desc.VendorId, Desc.DeviceId, Desc.Description);

    Caps.id_vendor = Desc.VendorId;
    Caps.id_device = Desc.DeviceId;

    u32 createDeviceFlags = 0;

#ifdef DEBUG
    if (xrDebug::DebuggerIsPresent())
        createDeviceFlags |= D3D_CREATE_DEVICE_DEBUG;
#endif

    HRESULT R;

#if defined(USE_DX12)
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
#else
    D3D_FEATURE_LEVEL featureLevels[] = {
#ifdef HAS_DX11_3
        D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0,
#endif
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

    D3D_FEATURE_LEVEL featureLevels2[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

    D3D_FEATURE_LEVEL featureLevels3[] = {D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
#endif
    auto& pContext = d3d_contexts_pool[CHW::IMM_CTX_ID];
#if defined(USE_DX12)
    const auto createDevice = [&](const D3D_FEATURE_LEVEL* level, const u32 levels) {
        return DX12CreateDevice(m_pAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, level, levels,
            D3D12_SDK_VERSION, reinterpret_cast<ID3D11Device **>(&pDevice), &FeatureLevel, reinterpret_cast<ID3D11DeviceContext**>(&pContext));
    };
    R = createDevice(featureLevels, std::size(featureLevels));
#else
    const auto createDevice = [&](const D3D_FEATURE_LEVEL* level, const u32 levels) {
        static const auto d3d11CreateDevice =
            static_cast<PFN_D3D11_CREATE_DEVICE>(hD3D->GetProcAddress("D3D11CreateDevice"));
        return d3d11CreateDevice(m_pAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, level, levels,
            D3D11_SDK_VERSION, &pDevice, &FeatureLevel, &pContext);
    };

    if (DX10Only)
        R = createDevice(featureLevels3, std::size(featureLevels3));
    else
    {
        R = createDevice(featureLevels, std::size(featureLevels));
        if (FAILED(R))
            R = createDevice(featureLevels2, std::size(featureLevels2));
    }
#endif

    if (SUCCEEDED(R))
    {
#if defined(USE_DX12)
        D3DCompile = &::D3DCompile;
        ComputeShadersSupported = true;

        D3D11_FEATURE_DATA_D3D11_OPTIONS options;
        pDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &options, sizeof(options));

        D3D11_FEATURE_DATA_DOUBLES doubles;
        pDevice->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &doubles, sizeof(doubles));

        DoublePrecisionFloatShaderOps = doubles.DoublePrecisionFloatShaderOps;
        SAD4ShaderInstructions = options.SAD4ShaderInstructions;
        ExtendedDoublesShaderInstructions = options.ExtendedDoublesShaderInstructions;
#else
        pContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&pContext1));
#ifdef HAS_DX11_3
        pDevice->QueryInterface(__uuidof(ID3D11Device3), reinterpret_cast<void**>(&pDevice3));
#endif
        if (FeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            D3DCompile = &::D3DCompile;
            ComputeShadersSupported = true;
        }
        else
        {
            if (ClearSkyMode)
            {
                hD3DCompiler = XRay::LoadModule("d3dcompiler_37");
                D3DCompile = static_cast<D3DCompileFunc>(hD3DCompiler->GetProcAddress("D3DCompileFromMemory"));
            }
            else
            {
                D3DCompile = &::D3DCompile;
            }

            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS data;
            pDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &data, sizeof(data));
            ComputeShadersSupported = data.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x;
        }
        D3D11_FEATURE_DATA_D3D11_OPTIONS options;
        pDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &options, sizeof(options));

        D3D11_FEATURE_DATA_DOUBLES doubles;
        pDevice->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &doubles, sizeof(doubles));

        DoublePrecisionFloatShaderOps = doubles.DoublePrecisionFloatShaderOps;
        SAD4ShaderInstructions = options.SAD4ShaderInstructions;
        ExtendedDoublesShaderInstructions = options.ExtendedDoublesShaderInstructions;
#endif
    }

    if (FAILED(R))
    {
        Valid = false;
        if (!ThisInstanceIsGlobal())
            return;
        // Fatal error! Cannot create rendering device AT STARTUP !!!
        Msg("Failed to initialize graphics hardware.\n"
            "Please try to restart the game.\n"
            "CreateDevice returned 0x%08x",
            R);
        xrDebug::DoExit("Failed to initialize graphics hardware.\nPlease try to restart the game.");
    }

    _SHOW_REF("* CREATE: DeviceREF:", pDevice);

    // Create deferred contexts
    for (int id = 0; id < R__NUM_PARALLEL_CONTEXTS; ++id)
    {
#if defined(USE_DX12)
#if DX12_DEFERRED_CONTEXT
        R = pDevice->CreateDeferredContext1(0, &d3d_contexts_pool[id]);
#endif
#else
        R = pDevice->CreateDeferredContext(0, &d3d_contexts_pool[id]);
#endif
        VERIFY(SUCCEEDED(R));
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (!SDL_GetWindowWMInfo(sdlWnd, &info))
    {
        Msg("! Failed to retrieve SDL window handle: %s", SDL_GetError());
        Valid = false;
        return;
    }

    const HWND hwnd = info.info.win.window;

#if defined(USE_DX12)
    if (!CreateSwapChainOnDX12(hwnd))
    {
        if (!CreateSwapChain(hwnd))
            Valid = false;
    }
#else
    if (!CreateSwapChainOnDX11_2(hwnd))
    {
        if (!CreateSwapChain(hwnd))
            Valid = false;
    }
#endif

    // Select depth-stencil format
    constexpr DXGI_FORMAT formats[] = {
        // DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
    };  
    const DXGI_FORMAT selectedFormat = SelectFormat(D3D_FORMAT_SUPPORT_DEPTH_STENCIL, formats);
    if (selectedFormat == DXGI_FORMAT_UNKNOWN)
    {
        Valid = false;
        if (!ThisInstanceIsGlobal())
            return;
        Log("Failed to initialize graphics hardware: "
            "failed to select depth-stencil format.\n"
            "Please try to restart the game.");
        xrDebug::DoExit("Failed to initialize graphics hardware.\nPlease try to restart the game.");
    }  
    Caps.fDepth = dx11TextureUtils::ConvertTextureFormat(selectedFormat);

#if CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS
    m_constant_allocator.Initialize();
    for (u32 q = 0; q < PoolConfig::POOL_FRAME_QUERY_COUNT; q++)
    {
        CreateFence(m_frameQuery[q]);
    }
#endif

    const auto memory = Desc.DedicatedVideoMemory;
    Msg("*   Texture memory: %d M", memory / (1024 * 1024));
}

bool CHW::CreateSwapChain(HWND hwnd)
{
    // Set up the presentation parameters
    DXGI_SWAP_CHAIN_DESC& sd = m_ChainDesc;
    ZeroMemory(&sd, sizeof(sd));

    // Back buffer
    sd.BufferDesc.Width = Device.dwWidth;
    sd.BufferDesc.Height = Device.dwHeight;

    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;

    //  TODO: DX11: implement dynamic format selection
    constexpr DXGI_FORMAT formats[] = {
        // DXGI_FORMAT_R16G16B16A16_FLOAT, // Do we even need this?
        // DXGI_FORMAT_R10G10B10A2_UNORM, // D3DX11SaveTextureToMemory fails on this format
        DXGI_FORMAT_R8G8B8A8_UNORM,
    };

    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Select back-buffer format
    sd.BufferDesc.Format = SelectFormat(D3D_FORMAT_SUPPORT_DISPLAY, formats);
    Caps.fTarget = dx11TextureUtils::ConvertTextureFormat(sd.BufferDesc.Format);

    // Buffering
#if defined(USE_DX12)
    BackBufferCount = 2;
#else 
    BackBufferCount = 1;
#endif

    sd.BufferCount = BackBufferCount;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Multisample
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;

    // Windoze
    /* XXX:
       Probably the reason of weird tearing
       glitches reported by Shoker in windowed
       mode with VSync enabled.
       XXX: Fix this windoze stuff!!!
    */
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;

    //  Additional set up
#if FRAME_LATENCY_WAITABLE_OBJECT == 1
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#else 
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif 

#if defined(USE_DX11)
    const auto hr = m_pFactory->CreateSwapChain(pDevice, &sd, &m_pSwapChain);
#else 
    const auto hr = m_pFactory->CreateSwapChain(pDevice, &sd, reinterpret_cast<IDXGISwapChain**>(&m_pSwapChain));
#endif 

#if FRAME_LATENCY_WAITABLE_OBJECT == 1
    if (sd.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
    {
        m_pSwapChain->SetMaximumFrameLatency(BackBufferCount - 1);
    }
#endif

    return SUCCEEDED(hr);
}

#if defined(USE_DX11)
bool CHW::CreateSwapChainOnDX11_2(HWND hwnd)
{
    if (strstr(Core.Params, "-no_dx11_2"))
        return false;

#ifdef HAS_DX11_2
    IDXGIFactory2* pFactory2{};
    m_pAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pFactory2);
    if (!pFactory2)
        return false;

    // Set up the presentation parameters
    DXGI_SWAP_CHAIN_DESC1 desc{};

    // Back buffer
    desc.Width = Device.dwWidth;
    desc.Height = Device.dwHeight;

    constexpr DXGI_FORMAT formats[] = {
        // DXGI_FORMAT_R16G16B16A16_FLOAT,
        // DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM,
    };

    // Select back-buffer format
    desc.Format = SelectFormat(D3D11_FORMAT_SUPPORT_DISPLAY, formats);
    Caps.fTarget = dx11TextureUtils::ConvertTextureFormat(desc.Format);

    // Buffering
    BackBufferCount = 1; // For DXGI_SWAP_EFFECT_FLIP_DISCARD we need at least two
    desc.BufferCount = BackBufferCount;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Multisample
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    // Windoze
    // desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // XXX: tearing glitches with flip presentation model
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fulldesc{};
    fulldesc.Windowed = TRUE;

    // Additional setup
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain1* swapchain{};
    const HRESULT result = pFactory2->CreateSwapChainForHwnd(
        pDevice, hwnd, &desc, fulldesc.Windowed ? nullptr : &fulldesc, nullptr, &swapchain);
    _RELEASE(pFactory2);

    if (FAILED(result))
        return false;

    if (FAILED(swapchain->GetDesc(&m_ChainDesc)))
    {
        _RELEASE(swapchain);
        return false;
    }
    m_pSwapChain = swapchain;

    m_pSwapChain->QueryInterface(__uuidof(IDXGISwapChain2), reinterpret_cast<void**>(&m_pSwapChain2));

    if (m_pSwapChain2 && ThisInstanceIsGlobal())
        Device.PresentationFinished = m_pSwapChain2->GetFrameLatencyWaitableObject();

    return true;
#else // #ifdef HAS_DX11_2
    UNUSED(hwnd);
#endif

    return false;
}
#else
bool CHW::CreateSwapChainOnDX12(HWND hwnd) 
{
    // Set up the presentation parameters
    DXGI_SWAP_CHAIN_DESC1 desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC1));

    // Back buffer
    desc.Width = Device.dwWidth;
    desc.Height = Device.dwHeight;

    constexpr DXGI_FORMAT formats[] = {
        // DXGI_FORMAT_R16G16B16A16_FLOAT,
        // DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM,
    };

    // Select back-buffer format
    desc.Format = SelectFormat(D3D11_FORMAT_SUPPORT_DISPLAY, formats);
    Caps.fTarget = dx11TextureUtils::ConvertTextureFormat(desc.Format);

    // Buffering
    BackBufferCount = 2; // For DXGI_SWAP_EFFECT_FLIP_DISCARD we need at least two
    
    desc.BufferCount = BackBufferCount;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Multisample
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    // Windoze
    // desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // XXX: tearing glitches with flip presentation model
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fulldesc;
    ZeroMemory(&fulldesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));

    fulldesc.Windowed = TRUE;

    // Additional setup
#if FRAME_LATENCY_WAITABLE_OBJECT == 1
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#else
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif 

    const HRESULT result = m_pFactory->CreateSwapChainForHwnd(
        pDevice, hwnd, &desc, fulldesc.Windowed ? nullptr : &fulldesc, nullptr, reinterpret_cast<IDXGISwapChain1**>(&m_pSwapChain));

    if (FAILED(result))
        return false;

    if (FAILED(m_pSwapChain->GetDesc(&m_ChainDesc))) {
        _RELEASE(m_pSwapChain);
        return false;
    }
  
#if FRAME_LATENCY_WAITABLE_OBJECT == 1
    if (desc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
    {
        m_pSwapChain->SetMaximumFrameLatency(BackBufferCount - 1);
    }
#endif
    
    return true;
}
#endif

bool CHW::ThisInstanceIsGlobal() const { return this == &HW; }

void CHW::DestroyDevice()
{
    if (ThisInstanceIsGlobal()) // only if we are global HW
    {
        RSManager.ClearStateArray();
        DSSManager.ClearStateArray();
        BSManager.ClearStateArray();
        SSManager.ClearStateArray();
    }
    //  Must switch to windowed mode to release swap chain
    if (!m_ChainDesc.Windowed && m_pSwapChain)
        m_pSwapChain->SetFullscreenState(FALSE, NULL);
#if defined(USE_DX11)
#ifdef HAS_DX11_2
    _RELEASE(m_pSwapChain2);
#endif
#endif

    _SHOW_REF("refCount:m_pSwapChain", m_pSwapChain);
    _RELEASE(m_pSwapChain);
  
#if defined(USE_DX11)
    _RELEASE(pContext1);
#endif

    for (int id = 0; id < R__NUM_CONTEXTS; ++id)
    {
        if (d3d_contexts_pool[id])
        {
            _SHOW_REF("refCount:pContext", d3d_contexts_pool[id]);
            _RELEASE(d3d_contexts_pool[id]);
        }
    }

#if defined(USE_DX11)
#ifdef HAS_DX11_3
    _RELEASE(pDevice3);
#endif
#endif
   
#if CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS
    m_constant_allocator.Shutdown();
    for (u32 q = 0; q < PoolConfig::POOL_FRAME_QUERY_COUNT; q++)
    {
        if (ReleaseFence(m_frameQuery[q]) != S_OK)
        {
            Msg("Initialize: could not releasefence");
        }
        m_frameQuery[q] = u64();
    }
#endif

    _SHOW_REF("refCount:pDevice:", pDevice);
    _RELEASE(pDevice);

    DestroyD3D();
}

//////////////////////////////////////////////////////////////////////
// Resetting device
//////////////////////////////////////////////////////////////////////
void CHW::Reset()
{
    DXGI_SWAP_CHAIN_DESC& cd = m_ChainDesc;
    const bool bWindowed = ThisInstanceIsGlobal() ? psDeviceMode.WindowStyle != rsFullscreen : true;
    cd.Windowed = bWindowed;
    m_pSwapChain->SetFullscreenState(!bWindowed, NULL);
    DXGI_MODE_DESC& desc = m_ChainDesc.BufferDesc;
    desc.Width = Device.dwWidth;
    desc.Height = Device.dwHeight;
    CHK_DX(m_pSwapChain->ResizeTarget(&desc));
    CHK_DX(m_pSwapChain->ResizeBuffers(cd.BufferCount, desc.Width, desc.Height, desc.Format, cd.Flags));
}

void CHW::SetPrimaryAttributes(u32& /*windowFlags*/) {}

bool CHW::CheckFormatSupport(const DXGI_FORMAT format, const u32 feature) const
{
    u32 supports;

    if (SUCCEEDED(pDevice->CheckFormatSupport(format, &supports)))
    {
        if (supports & feature)
            return true;
    }

    return false;
}

DXGI_FORMAT CHW::SelectFormat(D3D_FORMAT_SUPPORT feature, const DXGI_FORMAT formats[], size_t count) const
{
    for (size_t i = 0; i < count; ++i)
        if (CheckFormatSupport(formats[i], feature))
            return formats[i];

    return DXGI_FORMAT_UNKNOWN;
}

bool CHW::UsingFlipPresentationModel() const
{
    return m_ChainDesc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
#ifdef HAS_DXGI1_4
        || m_ChainDesc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD
#endif
        ;
}

std::pair<u32, u32> CHW::GetSurfaceSize() const
{
    return {m_ChainDesc.BufferDesc.Width, m_ChainDesc.BufferDesc.Height};
}

void CHW::BeginScene()
{
#if CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS
    m_constant_allocator.Update(m_frame_id, m_frameQuery[m_frame_id]);
#endif
}

void CHW::EndScene() 
{
#if CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS
    IssueFence(m_frameQuery[m_frame_id]);
#endif
}

void CHW::Present()
{
    const bool bUseVSync = psDeviceMode.WindowStyle == rsFullscreen &&
        psDeviceFlags.test(rsVSync); // xxx: weird tearing glitches when VSync turned on for windowed mode in DX11

    switch (m_pSwapChain->Present(bUseVSync ? 1 : 0, 0))
    {
    case DXGI_STATUS_OCCLUDED:
    case DXGI_ERROR_DEVICE_REMOVED:
        doPresentTest = true;
        break;
    }

#if defined(USE_DX11)
#ifdef HAS_DX11_2
    if (m_pSwapChain2 && UsingFlipPresentationModel())
    {
        const float fps = Device.GetStats().fFPS;
        if (fps < 30)
            m_pSwapChain2->SetSourceSize(UINT(Device.dwWidth * 0.85f), UINT(Device.dwHeight * 0.85f));
        else if (fps < 15)
            m_pSwapChain2->SetSourceSize(UINT(Device.dwWidth * 0.7f), UINT(Device.dwHeight * 0.7f));
    }
#endif
#else   
    if (UsingFlipPresentationModel())
    {
        const float fps = Device.GetStats().fFPS;
        if (fps < 30)
            m_pSwapChain->SetSourceSize(UINT(Device.dwWidth * 0.85f), UINT(Device.dwHeight * 0.85f));
        else if (fps < 15)
            m_pSwapChain->SetSourceSize(UINT(Device.dwWidth * 0.7f), UINT(Device.dwHeight * 0.7f));
    }
#endif

#if CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS
    m_constant_allocator.ReleaseEmptyBanks();
    m_frame_id = (m_frame_id + 1) % POOL_FRAME_QUERY_COUNT;
#endif
    
#if !defined(USE_DX12)
    CurrentBackBuffer = (CurrentBackBuffer + 1) % BackBufferCount;
#endif 
}

DeviceState CHW::GetDeviceState()
{
    if (doPresentTest)
    {
        switch (m_pSwapChain->Present(0, DXGI_PRESENT_TEST))
        {
        case S_OK:
            doPresentTest = false;

            break;

        case DXGI_STATUS_OCCLUDED:
            // Do not render until we become visible again
            return DeviceState::Lost;

        case DXGI_ERROR_DEVICE_RESET:
            return DeviceState::NeedReset;

        case DXGI_ERROR_DEVICE_REMOVED:

            FATAL("Graphics driver was updated or GPU was physically removed from computer.\n"
                  "Please, restart the game.");

            break;
        }
    }

    return DeviceState::Normal;
}
