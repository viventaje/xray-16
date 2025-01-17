#pragma once

class dx11State;

using D3D_SAMPLER_DESC              = D3D11_SAMPLER_DESC;
using D3D_RASTERIZER_DESC           = D3D11_RASTERIZER_DESC;
using D3D_DEPTH_STENCIL_DESC        = D3D11_DEPTH_STENCIL_DESC;
using D3D_BLEND_DESC                = D3D11_BLEND_DESC;

using D3D_SUBRESOURCE_DATA          = D3D11_SUBRESOURCE_DATA;

using D3D_TEXTURE1D_DESC            = D3D11_TEXTURE1D_DESC;
using D3D_TEXTURE2D_DESC            = D3D11_TEXTURE2D_DESC;
using D3D_TEXTURE3D_DESC            = D3D11_TEXTURE3D_DESC;
using D3D_BUFFER_DESC               = D3D11_BUFFER_DESC;
using D3D_QUERY_DESC                = D3D11_QUERY_DESC;
using D3D_SHADER_RESOURCE_VIEW_DESC = D3D11_SHADER_RESOURCE_VIEW_DESC;
using D3D_RENDER_TARGET_VIEW_DESC   = D3D11_RENDER_TARGET_VIEW_DESC;
using D3D_DEPTH_STENCIL_VIEW_DESC   = D3D11_DEPTH_STENCIL_VIEW_DESC;
using D3D_MAPPED_TEXTURE2D          = D3D11_MAPPED_SUBRESOURCE;
using D3D_MAPPED_TEXTURE3D          = D3D11_MAPPED_SUBRESOURCE;
using D3D_INPUT_ELEMENT_DESC        = D3D11_INPUT_ELEMENT_DESC;

using D3D_RECT                      = D3D11_RECT;

#if USE_DX12_CRY_TYPES
using ID3DRasterizerState           = ID3D11RasterizerState;
using ID3DDepthStencilState         = ID3D11DepthStencilState;
using ID3DBlendState                = ID3D11BlendState;
using ID3DSamplerState              = ID3D11SamplerState;
using ID3DBuffer                    = ID3D11Buffer; //CryDX12Buffer
using ID3DVertexBuffer              = ID3D11Buffer; //CryDX12Buffer
using ID3DIndexBuffer               = ID3D11Buffer; //CryDX12Buffer
using ID3DInputLayout               = ID3D11InputLayout;
using ID3DVertexShader              = ID3D11VertexShader;
using ID3DGeometryShader            = ID3D11GeometryShader;
using ID3DPixelShader               = ID3D11PixelShader;
using ID3DTexture1D                 = ID3D11Texture1D;
using ID3DTexture2D                 = ID3D11Texture2D;
using ID3DTexture3D                 = ID3D11Texture3D;
using ID3DBaseTexture               = ID3D11Resource;
using ID3DResource                  = ID3D11Resource;
using ID3DRenderTargetView          = ID3D11RenderTargetView; //CryDX12RenderTargetView
using ID3DDepthStencilView          = ID3D11DepthStencilView; //CryDX12DepthStencilView
using ID3DShaderResourceView        = ID3D11ShaderResourceView; //CryDX12ShaderResourceView
using ID3DQuery                     = ID3D11Query; //CryDX12Query
using ID3DDevice                    = CCryDX12Device;
using ID3DDeviceContext             = CCryDX12DeviceContext;
#else
using ID3DRasterizerState           = ID3D11RasterizerState;
using ID3DDepthStencilState         = ID3D11DepthStencilState;
using ID3DBlendState                = ID3D11BlendState;
using ID3DSamplerState              = ID3D11SamplerState;
using ID3DBuffer                    = ID3D11Buffer;
using ID3DVertexBuffer              = ID3D11Buffer;
using ID3DIndexBuffer               = ID3D11Buffer;
using ID3DInputLayout               = ID3D11InputLayout;
using ID3DVertexShader              = ID3D11VertexShader;
using ID3DGeometryShader            = ID3D11GeometryShader;
using ID3DPixelShader               = ID3D11PixelShader;
using ID3DTexture1D                 = ID3D11Texture1D;
using ID3DTexture2D                 = ID3D11Texture2D;
using ID3DTexture3D                 = ID3D11Texture3D;
using ID3DBaseTexture               = ID3D11Resource;
using ID3DResource                  = ID3D11Resource;
using ID3DRenderTargetView          = ID3D11RenderTargetView;
using ID3DDepthStencilView          = ID3D11DepthStencilView;
using ID3DShaderResourceView        = ID3D11ShaderResourceView;
using ID3DQuery                     = ID3D11Query;
#if USE_DX12
using ID3DDevice                    = ID3D11Device1;
using ID3DDeviceContext             = ID3D11DeviceContext1;
#else
using ID3DDevice                    = ID3D11Device;
using ID3DDeviceContext             = ID3D11DeviceContext;
#endif
#endif

constexpr auto D3D_COMMONSHADER_SAMPLER_SLOT_COUNT = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;

constexpr auto D3D_REQ_TEXTURE2D_U_OR_V_DIMENSION = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

constexpr auto D3D_STANDARD_MULTISAMPLE_PATTERN = D3D11_STANDARD_MULTISAMPLE_PATTERN;

using          D3D_USAGE                        = D3D11_USAGE;
constexpr auto D3D_USAGE_DEFAULT                = D3D11_USAGE_DEFAULT;
constexpr auto D3D_USAGE_DYNAMIC                = D3D11_USAGE_DYNAMIC;
constexpr auto D3D_USAGE_STAGING                = D3D11_USAGE_STAGING;
constexpr auto D3D_USAGE_IMMUTABLE              = D3D11_USAGE_IMMUTABLE;

using          D3D_CULL_MODE                    = D3D11_CULL_MODE;
constexpr auto D3D_CULL_NONE                    = D3D11_CULL_NONE;
constexpr auto D3D_CULL_FRONT                   = D3D11_CULL_FRONT;
constexpr auto D3D_CULL_BACK                    = D3D11_CULL_BACK;

constexpr auto D3D_CREATE_DEVICE_DEBUG          = D3D11_CREATE_DEVICE_DEBUG;

using          D3D_COMPARISON_FUNC              = D3D11_COMPARISON_FUNC;
constexpr auto D3D_COMPARISON_NEVER             = D3D11_COMPARISON_NEVER;
constexpr auto D3D_COMPARISON_LESS              = D3D11_COMPARISON_LESS;
constexpr auto D3D_COMPARISON_EQUAL             = D3D11_COMPARISON_EQUAL;
constexpr auto D3D_COMPARISON_LESS_EQUAL        = D3D11_COMPARISON_LESS_EQUAL;
constexpr auto D3D_COMPARISON_GREATER           = D3D11_COMPARISON_GREATER;
constexpr auto D3D_COMPARISON_NOT_EQUAL         = D3D11_COMPARISON_NOT_EQUAL;
constexpr auto D3D_COMPARISON_GREATER_EQUAL     = D3D11_COMPARISON_GREATER_EQUAL;
constexpr auto D3D_COMPARISON_ALWAYS            = D3D11_COMPARISON_ALWAYS;

using          D3D_STENCIL_OP                   = D3D11_STENCIL_OP;
constexpr auto D3D_STENCIL_OP_KEEP              = D3D11_STENCIL_OP_KEEP;
constexpr auto D3D_STENCIL_OP_ZERO              = D3D11_STENCIL_OP_ZERO;
constexpr auto D3D_STENCIL_OP_REPLACE           = D3D11_STENCIL_OP_REPLACE;
constexpr auto D3D_STENCIL_OP_INCR_SAT          = D3D11_STENCIL_OP_INCR_SAT;
constexpr auto D3D_STENCIL_OP_DECR_SAT          = D3D11_STENCIL_OP_DECR_SAT;
constexpr auto D3D_STENCIL_OP_INVERT            = D3D11_STENCIL_OP_INVERT;
constexpr auto D3D_STENCIL_OP_INCR              = D3D11_STENCIL_OP_INCR;
constexpr auto D3D_STENCIL_OP_DECR              = D3D11_STENCIL_OP_DECR;

using          D3D_BLEND                        = D3D11_BLEND;
constexpr auto D3D_BLEND_ZERO                   = D3D11_BLEND_ZERO;
constexpr auto D3D_BLEND_ONE                    = D3D11_BLEND_ONE;
constexpr auto D3D_BLEND_SRC_COLOR              = D3D11_BLEND_SRC_COLOR;
constexpr auto D3D_BLEND_INV_SRC_COLOR          = D3D11_BLEND_INV_SRC_COLOR;
constexpr auto D3D_BLEND_SRC_ALPHA              = D3D11_BLEND_SRC_ALPHA;
constexpr auto D3D_BLEND_INV_SRC_ALPHA          = D3D11_BLEND_INV_SRC_ALPHA;
constexpr auto D3D_BLEND_DEST_ALPHA             = D3D11_BLEND_DEST_ALPHA;
constexpr auto D3D_BLEND_INV_DEST_ALPHA         = D3D11_BLEND_INV_DEST_ALPHA;
constexpr auto D3D_BLEND_DEST_COLOR             = D3D11_BLEND_DEST_COLOR;
constexpr auto D3D_BLEND_INV_DEST_COLOR         = D3D11_BLEND_INV_DEST_COLOR;
constexpr auto D3D_BLEND_SRC_ALPHA_SAT          = D3D11_BLEND_SRC_ALPHA_SAT;
constexpr auto D3D_BLEND_BLEND_FACTOR           = D3D11_BLEND_BLEND_FACTOR;
constexpr auto D3D_BLEND_INV_BLEND_FACTOR       = D3D11_BLEND_INV_BLEND_FACTOR;
constexpr auto D3D_BLEND_SRC1_COLOR             = D3D11_BLEND_SRC1_COLOR;
constexpr auto D3D_BLEND_INV_SRC1_COLOR         = D3D11_BLEND_INV_SRC1_COLOR;
constexpr auto D3D_BLEND_SRC1_ALPHA             = D3D11_BLEND_SRC1_ALPHA;
constexpr auto D3D_BLEND_INV_SRC1_ALPHA         = D3D11_BLEND_INV_SRC1_ALPHA;

using          D3D_BLEND_OP                     = D3D11_BLEND_OP;
constexpr auto D3D_BLEND_OP_ADD                 = D3D11_BLEND_OP_ADD;
constexpr auto D3D_BLEND_OP_SUBTRACT            = D3D11_BLEND_OP_SUBTRACT;
constexpr auto D3D_BLEND_OP_REV_SUBTRACT        = D3D11_BLEND_OP_REV_SUBTRACT;
constexpr auto D3D_BLEND_OP_MIN                 = D3D11_BLEND_OP_MIN;
constexpr auto D3D_BLEND_OP_MAX                 = D3D11_BLEND_OP_MAX;

using          D3D_TEXTURE_ADDRESS_MODE         = D3D11_TEXTURE_ADDRESS_MODE;
constexpr auto D3D_TEXTURE_ADDRESS_WRAP         = D3D11_TEXTURE_ADDRESS_WRAP;
constexpr auto D3D_TEXTURE_ADDRESS_MIRROR       = D3D11_TEXTURE_ADDRESS_MIRROR;
constexpr auto D3D_TEXTURE_ADDRESS_CLAMP        = D3D11_TEXTURE_ADDRESS_CLAMP;
constexpr auto D3D_TEXTURE_ADDRESS_BORDER       = D3D11_TEXTURE_ADDRESS_BORDER;
constexpr auto D3D_TEXTURE_ADDRESS_MIRROR_ONCE  = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;

constexpr auto D3D_DEPTH_WRITE_MASK_ALL         = D3D11_DEPTH_WRITE_MASK_ALL;
constexpr auto D3D_DEPTH_WRITE_MASK_ZERO        = D3D11_DEPTH_WRITE_MASK_ZERO;
constexpr auto D3D_COLOR_WRITE_ENABLE_ALL       = D3D11_COLOR_WRITE_ENABLE_ALL;

using          D3D_FILTER                       = D3D11_FILTER;
constexpr auto D3D_FILTER_ANISOTROPIC           = D3D11_FILTER_ANISOTROPIC;
constexpr auto D3D_FILTER_MIN_MAG_MIP_LINEAR    = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
constexpr auto D3D_FILTER_COMPARISON_ANISOTROPIC = D3D11_FILTER_COMPARISON_ANISOTROPIC;

using          D3D_FILL_MODE                    = D3D11_FILL_MODE;
constexpr auto D3D_FILL_SOLID                   = D3D11_FILL_SOLID;
constexpr auto D3D_FILL_WIREFRAME               = D3D11_FILL_WIREFRAME;

using          D3D_FORMAT_SUPPORT               = D3D11_FORMAT_SUPPORT;
constexpr auto D3D_FORMAT_SUPPORT_TEXTURE2D     = D3D11_FORMAT_SUPPORT_TEXTURE2D;
constexpr auto D3D_FORMAT_SUPPORT_RENDER_TARGET = D3D11_FORMAT_SUPPORT_RENDER_TARGET;
constexpr auto D3D_FORMAT_SUPPORT_DEPTH_STENCIL = D3D11_FORMAT_SUPPORT_DEPTH_STENCIL;
constexpr auto D3D_FORMAT_SUPPORT_DISPLAY       = D3D11_FORMAT_SUPPORT_DISPLAY;
constexpr auto D3D_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW = D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW;

constexpr auto D3D_INPUT_PER_VERTEX_DATA        = D3D11_INPUT_PER_VERTEX_DATA;

using          D3D_BIND_FLAG                    = D3D11_BIND_FLAG;
constexpr auto D3D_BIND_INDEX_BUFFER            = D3D11_BIND_INDEX_BUFFER;
constexpr auto D3D_BIND_VERTEX_BUFFER           = D3D11_BIND_VERTEX_BUFFER;
constexpr auto D3D_BIND_CONSTANT_BUFFER         = D3D11_BIND_CONSTANT_BUFFER;
constexpr auto D3D_BIND_SHADER_RESOURCE         = D3D11_BIND_SHADER_RESOURCE;
constexpr auto D3D_BIND_DEPTH_STENCIL           = D3D11_BIND_DEPTH_STENCIL;
constexpr auto D3D_BIND_STREAM_OUTPUT           = D3D11_BIND_STREAM_OUTPUT;
constexpr auto D3D_BIND_RENDER_TARGET           = D3D11_BIND_RENDER_TARGET;
constexpr auto D3D_BIND_UNORDERED_ACCESS        = D3D11_BIND_UNORDERED_ACCESS;

constexpr auto D3D_CPU_ACCESS_WRITE             = D3D11_CPU_ACCESS_WRITE;
constexpr auto D3D_CPU_ACCESS_READ              = D3D11_CPU_ACCESS_READ;

using          D3D_MAP                          = D3D11_MAP;
constexpr auto D3D_MAP_READ                     = D3D11_MAP_READ;
constexpr auto D3D_MAP_WRITE                    = D3D11_MAP_WRITE;
constexpr auto D3D_MAP_READ_WRITE               = D3D11_MAP_READ_WRITE;
constexpr auto D3D_MAP_WRITE_DISCARD            = D3D11_MAP_WRITE_DISCARD;
constexpr auto D3D_MAP_WRITE_NO_OVERWRITE       = D3D11_MAP_WRITE_NO_OVERWRITE;

constexpr auto D3D_RESOURCE_MISC_GENERATE_MIPS  = D3D11_RESOURCE_MISC_GENERATE_MIPS;
constexpr auto D3D_RESOURCE_MISC_SHARED         = D3D11_RESOURCE_MISC_SHARED;
constexpr auto D3D_RESOURCE_MISC_TEXTURECUBE    = D3D11_RESOURCE_MISC_TEXTURECUBE;
constexpr auto D3D_RESOURCE_MISC_DRAWINDIRECT_ARGS = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
constexpr auto D3D_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
constexpr auto D3D_RESOURCE_MISC_BUFFER_STRUCTURED = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
constexpr auto D3D_RESOURCE_MISC_RESOURCE_CLAMP = D3D11_RESOURCE_MISC_RESOURCE_CLAMP;
constexpr auto D3D_RESOURCE_MISC_SHARED_KEYEDMUTEX = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
constexpr auto D3D_RESOURCE_MISC_GDI_COMPATIBLE = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

constexpr auto D3D_CLEAR_DEPTH                  = D3D11_CLEAR_DEPTH;
constexpr auto D3D_CLEAR_STENCIL                = D3D11_CLEAR_STENCIL;

using          D3D_QUERY                        = D3D11_QUERY;
constexpr auto D3D_QUERY_EVENT                  = D3D11_QUERY_EVENT;
constexpr auto D3D_QUERY_OCCLUSION              = D3D11_QUERY_OCCLUSION;
constexpr auto D3D_QUERY_TIMESTAMP              = D3D11_QUERY_TIMESTAMP;
constexpr auto D3D_QUERY_TIMESTAMP_DISJOINT     = D3D11_QUERY_TIMESTAMP_DISJOINT;
constexpr auto D3D_QUERY_PIPELINE_STATISTICS    = D3D11_QUERY_PIPELINE_STATISTICS;
constexpr auto D3D_QUERY_OCCLUSION_PREDICATE    = D3D11_QUERY_OCCLUSION_PREDICATE;
constexpr auto D3D_QUERY_SO_STATISTICS          = D3D11_QUERY_SO_STATISTICS;
constexpr auto D3D_QUERY_SO_OVERFLOW_PREDICATE  = D3D11_QUERY_SO_OVERFLOW_PREDICATE;

constexpr auto D3D_QUERY_SO_STATISTICS_STREAM0  = D3D11_QUERY_SO_STATISTICS_STREAM0;
constexpr auto D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM0 = D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM0;

constexpr auto D3D_QUERY_SO_STATISTICS_STREAM1  = D3D11_QUERY_SO_STATISTICS_STREAM1;
constexpr auto D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM1 = D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM1;

constexpr auto D3D_QUERY_SO_STATISTICS_STREAM2  = D3D11_QUERY_SO_STATISTICS_STREAM2;
constexpr auto D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM2 = D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM2;

constexpr auto D3D_QUERY_SO_STATISTICS_STREAM3  = D3D11_QUERY_SO_STATISTICS_STREAM3;
constexpr auto D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM3 = D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM3;

using          D3D_RESOURCE_DIMENSION           = D3D11_RESOURCE_DIMENSION;
constexpr auto D3D_RESOURCE_DIMENSION_UNKNOWN   = D3D11_RESOURCE_DIMENSION_UNKNOWN;
constexpr auto D3D_RESOURCE_DIMENSION_BUFFER    = D3D11_RESOURCE_DIMENSION_BUFFER;
constexpr auto D3D_RESOURCE_DIMENSION_TEXTURE1D = D3D11_RESOURCE_DIMENSION_TEXTURE1D;
constexpr auto D3D_RESOURCE_DIMENSION_TEXTURE2D = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
constexpr auto D3D_RESOURCE_DIMENSION_TEXTURE3D = D3D11_RESOURCE_DIMENSION_TEXTURE3D;


using          D3D_DSV_DIMENSION                = D3D11_DSV_DIMENSION;
constexpr auto D3D_DSV_DIMENSION_UNKNOWN        = D3D11_DSV_DIMENSION_UNKNOWN;
constexpr auto D3D_DSV_DIMENSION_TEXTURE1D      = D3D11_DSV_DIMENSION_TEXTURE1D;
constexpr auto D3D_DSV_DIMENSION_TEXTURE1DARRAY = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;

constexpr auto D3D_DSV_DIMENSION_TEXTURE2D      = D3D11_DSV_DIMENSION_TEXTURE2D;
constexpr auto D3D_DSV_DIMENSION_TEXTURE2DARRAY = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;

using          D3D_RTV_DIMENSION                = D3D11_RTV_DIMENSION;
constexpr auto D3D_DSV_DIMENSION_TEXTURE2DMS    = D3D11_DSV_DIMENSION_TEXTURE2DMS;
constexpr auto D3D_DSV_DIMENSION_TEXTURE2DMSARRAY = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;

constexpr auto D3D_RTV_DIMENSION_UNKNOWN        = D3D11_RTV_DIMENSION_UNKNOWN;
constexpr auto D3D_RTV_DIMENSION_BUFFER         = D3D11_RTV_DIMENSION_BUFFER;

constexpr auto D3D_RTV_DIMENSION_TEXTURE1D      = D3D11_RTV_DIMENSION_TEXTURE1D;
constexpr auto D3D_RTV_DIMENSION_TEXTURE1DARRAY = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;

constexpr auto D3D_RTV_DIMENSION_TEXTURE2D      = D3D11_RTV_DIMENSION_TEXTURE2D;
constexpr auto D3D_RTV_DIMENSION_TEXTURE2DARRAY = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;

constexpr auto D3D_RTV_DIMENSION_TEXTURE2DMS    = D3D11_RTV_DIMENSION_TEXTURE2DMS;
constexpr auto D3D_RTV_DIMENSION_TEXTURE2DMSARRAY = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;

constexpr auto D3D_RTV_DIMENSION_TEXTURE3D      = D3D11_RTV_DIMENSION_TEXTURE3D;

using          D3D_UAV_DIMENSION                = D3D11_UAV_DIMENSION;
constexpr auto D3D_UAV_DIMENSION_UNKNOWN        = D3D11_UAV_DIMENSION_UNKNOWN;
constexpr auto D3D_UAV_DIMENSION_BUFFER         = D3D11_UAV_DIMENSION_BUFFER;
constexpr auto D3D_UAV_DIMENSION_TEXTURE1D      = D3D11_UAV_DIMENSION_TEXTURE1D;
constexpr auto D3D_UAV_DIMENSION_TEXTURE1DARRAY = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
constexpr auto D3D_UAV_DIMENSION_TEXTURE2D      = D3D11_UAV_DIMENSION_TEXTURE2D;
constexpr auto D3D_UAV_DIMENSION_TEXTURE2DARRAY = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
constexpr auto D3D_UAV_DIMENSION_TEXTURE3D      = D3D11_UAV_DIMENSION_TEXTURE3D;

#if USE_DX12
using D3D_SHADER_DESC                           = D3D12_SHADER_DESC;
using D3D_SHADER_BUFFER_DESC                    = D3D12_SHADER_BUFFER_DESC;
using D3D_SHADER_VARIABLE_DESC                  = D3D12_SHADER_VARIABLE_DESC;
using D3D_SHADER_INPUT_BIND_DESC                = D3D12_SHADER_INPUT_BIND_DESC;
using D3D_SHADER_TYPE_DESC                      = D3D12_SHADER_TYPE_DESC;

using ID3DShaderReflection                      = ID3D12ShaderReflection;
using ID3DShaderReflectionConstantBuffer        = ID3D12ShaderReflectionConstantBuffer;
using ID3DShaderReflectionVariable              = ID3D12ShaderReflectionVariable;
using ID3DShaderReflectionType                  = ID3D12ShaderReflectionType;

#define IID_ID3DShaderReflection IID_ID3D12ShaderReflection
#else
using D3D_SHADER_DESC = D3D11_SHADER_DESC;
using D3D_SHADER_BUFFER_DESC = D3D11_SHADER_BUFFER_DESC;
using D3D_SHADER_VARIABLE_DESC = D3D11_SHADER_VARIABLE_DESC;
using D3D_SHADER_INPUT_BIND_DESC = D3D11_SHADER_INPUT_BIND_DESC;
using D3D_SHADER_TYPE_DESC = D3D11_SHADER_TYPE_DESC;

using ID3DShaderReflection = ID3D11ShaderReflection;
using ID3DShaderReflectionConstantBuffer = ID3D11ShaderReflectionConstantBuffer;
using ID3DShaderReflectionVariable = ID3D11ShaderReflectionVariable;
using ID3DShaderReflectionType = ID3D11ShaderReflectionType;

#define IID_ID3DShaderReflection IID_ID3D11ShaderReflection
#endif

using ID3DState = dx11State;
#define DX11_ONLY(expr) expr

struct D3D_VIEWPORT : D3D11_VIEWPORT
{
    using D3D11_VIEWPORT::D3D11_VIEWPORT;

    // Needed to suppress warnings
    template <typename TopLeftCoords, typename Dimensions>
    D3D_VIEWPORT(TopLeftCoords x, TopLeftCoords y, Dimensions w, Dimensions h, float minZ, float maxZ)
        : D3D11_VIEWPORT{
            static_cast<float>(x), static_cast<float>(y),
            static_cast<float>(w), static_cast<float>(h),
            minZ, maxZ
        }
    {}
};

#if USE_DX12_CRY_TYPES
using VertexBufferHandle    = ID3D11Buffer*;
using IndexBufferHandle     = ID3D11Buffer*;
using ConstantBufferHandle  = ID3D11Buffer*;
using HostBufferHandle      = void*;
#else
using VertexBufferHandle    = ID3D11Buffer*;
using IndexBufferHandle     = ID3D11Buffer*;
using ConstantBufferHandle  = ID3D11Buffer*;
using HostBufferHandle      = void*;
#endif

using VertexElement         = D3DVERTEXELEMENT9;
using InputElementDesc      = D3D11_INPUT_ELEMENT_DESC;
