#pragma once

#include "Common/Common.hpp"

#include "xrEngine/stdafx.h"

#include "xrEngine/vis_common.h"
#include "xrEngine/Render.h"
#include "xrEngine/IGame_Level.h"

#include "xrParticles/psystem.h"

#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#if __has_include(<dxgi1_3.h>)
#include <dxgi1_3.h>
#define HAS_DXGI1_3
#endif

#if __has_include(<dxgi1_4.h>)
#include <dxgi1_4.h>
#define HAS_DXGI1_4
#endif

#if __has_include(<dxgi1_5.h>)
#include <dxgi1_5.h>
#define HAS_DXGI1_5
#endif

//always enable in dx12 mode
#define CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS 1

#include "DX12/CryDX12.hpp"

#include "Layers/xrRenderDX11/CommonTypes.h"
#include "Layers/xrRenderDX11/dx11HW.h"

#include "Layers/xrRender/Shader.h"
#include "Layers/xrRender/R_Backend.h"
#include "Layers/xrRender/R_Backend_Runtime.h"
#include "Layers/xrRender/Blender.h"
#include "Layers/xrRender/Blender_CLSID.h"

#define R_GL 0
#define R_R1 1
#define R_R2 2
#define R_R3 3
#define R_R4 4
#define R_R5 5
#define RENDER R_R5

#include "Common/_d3d_extensions.h"

#include "Layers/xrRender/ResourceManager.h"
#include "Layers/xrRender/xrRender_console.h"

#include "r2.h"
#include "r5_rendertarget.h"

IC uint32_t NextPower2(uint32_t n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

IC uint32_t IntegerLog2(uint32_t x)
{ 
    DWORD result = 32 ^ 31; // assumes result is unmodified if _BitScanReverse returns 0
    _BitScanReverse(&result, x);
    result ^= 31; // needed because the index is from LSB (whereas all other implementations are from MSB)

    return 31 - result;
}

IC void jitter(CBlender_Compile& C)
{
    //	C.r_Sampler	("jitter0",	JITTER(0), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
    //	C.r_Sampler	("jitter1",	JITTER(1), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
    //	C.r_Sampler	("jitter2",	JITTER(2), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
    //	C.r_Sampler	("jitter3",	JITTER(3), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
    C.r_dx11Texture("jitter0", JITTER(0));
    C.r_dx11Texture("jitter1", JITTER(1));
    C.r_dx11Texture("jitter2", JITTER(2));
    C.r_dx11Texture("jitter3", JITTER(3));
    C.r_dx11Texture("jitter4", JITTER(4));
    C.r_dx11Texture("jitterMipped", r2_jitter_mipped);
    C.r_dx11Sampler("smp_jitter");
}
