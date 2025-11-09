/// (C) Yigsoft 2023

#include "framework.h"
#include "renderLines.h"
#include "utils.h"

namespace app
{

	const char* linePS = ""
		"float4 ps_main( float4 pos : SV_Position, float4 color : COLOR0 ) : SV_Target\n"
		"{ return float4(color.xyz,1); }\n";

	const char* lineVS = ""
		"struct VS_INPUT { float2 pos : POSITION; float4 color : COLOR0; };\n"
		"struct VS_OUTPUT { float4 pos : SV_Position; float4 color : COLOR0; };\n"
		"void vs_main( VS_INPUT d, out VS_OUTPUT o )\n"
		"{ o.pos = float4((d.pos.xy / float2(800.0f,-450.0f)) + float2(-1.0f,1.0f),0.5f,1); o.color = d.color; }\n";

	D3D11_INPUT_ELEMENT_DESC linesDecl[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RenderLines::RenderLines()
		: m_pixelShader( nullptr )
		, m_vertexShader( nullptr )
		, m_vertexLayout( nullptr )
		, m_vertexBuffer( nullptr )
		, m_rasterState( nullptr )
		, m_depthState( nullptr )
		, m_blendState( nullptr )

	{
	}

	RenderLines::~RenderLines()
	{
		SAFE_RELEASE( m_pixelShader );
		SAFE_RELEASE( m_vertexShader );
		SAFE_RELEASE( m_vertexLayout );
		SAFE_RELEASE( m_rasterState );
		SAFE_RELEASE( m_depthState );
		SAFE_RELEASE( m_blendState );
		delete m_vertexBuffer;
	}

	bool RenderLines::Init( ID3D11Device* device )
	{
		m_pixelShader = app::utils::CompilePixelShader( device, linePS );
		if ( !m_pixelShader )
			return false;

		ID3DBlob* vertexDataBlob = nullptr;
		m_vertexShader = app::utils::CompileVertexShader( device, lineVS, /*out*/ vertexDataBlob );
		if ( !m_vertexShader )
			return false;

		HRESULT hRet = device->CreateInputLayout( linesDecl, ARRAYSIZE(linesDecl), vertexDataBlob->GetBufferPointer(), vertexDataBlob->GetBufferSize(), &m_vertexLayout );
		SAFE_RELEASE( vertexDataBlob );

		if ( FAILED(hRet) )
		{
			fprintf( stderr, "Failed to create lines vertex layout: 0x%08X\n", hRet );
			return false;
		}

		// setup vertex buffer
		m_vertexBuffer = utils::CreateDynamicVertexBuffer( device, MAX_RENDER_VERTICES * sizeof(RenderVertex) );

		// create rendering setup
		{
			D3D11_RASTERIZER_DESC desc;

			memset( &desc, 0, sizeof(desc) );
			desc.CullMode = D3D11_CULL_NONE;
			desc.FillMode = D3D11_FILL_SOLID;

			HRESULT hRet = device->CreateRasterizerState( &desc, &m_rasterState );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create raster state: 0x%08X\n", hRet );
				return false;
			}
		}

		// create depth state
		{
			D3D11_DEPTH_STENCIL_DESC desc;

			memset( &desc, 0, sizeof(desc) );
			desc.DepthEnable = false;

			HRESULT hRet = device->CreateDepthStencilState( &desc, &m_depthState );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create depth state: 0x%08X\n", hRet );
				return false;
			}
		}

		// create blend state
		{
			D3D11_BLEND_DESC desc;

			memset( &desc, 0, sizeof(desc) );
			desc.RenderTarget[0].RenderTargetWriteMask = 0xF;

			HRESULT hRet = device->CreateBlendState( &desc, &m_blendState );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create blend state: 0x%08X\n", hRet );
				return false;
			}
		}

		// font is ready for drawing
		return true;
	}

	void RenderLines::Draw( ID3D11DeviceContext* deviceContext, const RenderVertex* vertices, const int numVertices ) 
	{
		int vertexAllocOffset = 0;

		{
			utils::BufferWriter vertexWriter( deviceContext, m_vertexBuffer, sizeof(RenderVertex)*numVertices, vertexAllocOffset );
			memcpy( vertexWriter.GetData(), vertices, sizeof(RenderVertex)*numVertices );
		}

		UINT stride = sizeof(RenderVertex);
		UINT offsets = vertexAllocOffset;
		deviceContext->IASetInputLayout( m_vertexLayout );
		deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
		deviceContext->IASetVertexBuffers( 0, 1, m_vertexBuffer->GetBufferPtr(), &stride, &offsets );

		deviceContext->PSSetShader( m_pixelShader, NULL, 0 );
		deviceContext->VSSetShader( m_vertexShader, NULL, 0 );
		deviceContext->RSSetState( m_rasterState );

		FLOAT blendFactor[] = {0,0,0,0};
		deviceContext->OMSetBlendState( m_blendState, blendFactor, 0xFFFFFFFF );
		deviceContext->OMSetDepthStencilState( m_depthState, 0 );

		deviceContext->Draw( numVertices, 0 );
	}

} // app