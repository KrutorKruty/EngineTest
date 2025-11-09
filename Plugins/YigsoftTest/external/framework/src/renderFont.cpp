/// (C) Yigsoft 2023

#include "framework.h"
#include "renderFont.h"
#include "utils.h"

#include "fontData.inl"

namespace app
{

	const char* fontPS = ""
		"Texture2D fontTex : register( s0 );\n"
		"SamplerState fontTexSampler { Filter = MIN_MAG_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };\n"
		"float4 ps_main( float4 pos : SV_Position, float2 uv : TEXCOORD0, float4 color : COLOR0 ) : SV_Target\n"
		"{ return fontTex.Sample(fontTexSampler, uv).wwww * float4(color.xyz,1); }\n";

	const char* fontVS = ""
		"struct VS_INPUT { float2 pos : POSITION; float2 uv : TEXCOORD0; float4 color : COLOR0; };\n"
		"struct VS_OUTPUT { float4 pos : SV_Position; float2 uv : TEXCOORD0; float4 color : COLOR0; };\n"
		"void vs_main( VS_INPUT d, out VS_OUTPUT o )\n"
		"{ o.pos = float4((d.pos.xy / float2(800.0f,-450.0f)) + float2(-1.0f,1.0f),0.5f,1); o.uv = d.uv; o.color = d.color; }\n";

	D3D11_INPUT_ELEMENT_DESC fontDecl[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RenderFont::RenderFont()
		: m_texture( nullptr ) 
		, m_pixelShader( nullptr )
		, m_vertexShader( nullptr )
		, m_textureView( nullptr )
		, m_vertexLayout( nullptr )
		, m_indexBuffer( nullptr )
		, m_vertexBuffer( nullptr )
		, m_rasterState( nullptr )
		, m_depthState( nullptr )
		, m_blendState( nullptr )

	{
		memset( m_chars, 0, sizeof(m_chars) );
	}

	RenderFont::~RenderFont()
	{
		SAFE_RELEASE( m_texture );
		SAFE_RELEASE( m_pixelShader );
		SAFE_RELEASE( m_vertexShader );
		SAFE_RELEASE( m_textureView );
		SAFE_RELEASE( m_vertexLayout );
		SAFE_RELEASE( m_rasterState );
		SAFE_RELEASE( m_depthState );
		SAFE_RELEASE( m_blendState );

		delete m_indexBuffer;
		delete m_vertexBuffer;
	}

	bool RenderFont::Init( ID3D11Device* device )
	{
		m_pixelShader = app::utils::CompilePixelShader( device, fontPS );
		if ( !m_pixelShader )
			return false;

		ID3DBlob* vertexDataBlob = nullptr;
		m_vertexShader = app::utils::CompileVertexShader( device, fontVS, /*out*/ vertexDataBlob );
		if ( !m_vertexShader )
			return false;

		HRESULT hRet = device->CreateInputLayout( fontDecl, ARRAYSIZE(fontDecl), vertexDataBlob->GetBufferPointer(), vertexDataBlob->GetBufferSize(), &m_vertexLayout );
		if ( FAILED(hRet) )
		{
			fprintf( stderr, "Failed to create font vertex layout: 0x%08X\n", hRet );
			return false;
		}

		// setup texture
		{
			// convert font image (hacky)
			unsigned char fontDataTemp[ sizeof(app::font::TextureData) / 3 ];
			for (int i=0; i<sizeof(fontDataTemp); ++i )
				fontDataTemp[i] = app::font::TextureData[i*3] << 2;
		
			D3D11_TEXTURE2D_DESC texDesc;
			memset( &texDesc, 0, sizeof(texDesc) );
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.Format = DXGI_FORMAT_A8_UNORM;	
			texDesc.Width = 256;
			texDesc.Height = 128;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA texData;
			texData.pSysMem = &fontDataTemp;
			texData.SysMemPitch = 256;

			HRESULT hRet = device->CreateTexture2D( &texDesc, &texData, &m_texture );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create font texture: 0x%08X\n", hRet );
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			memset( &viewDesc, 0, sizeof(viewDesc) );
			viewDesc.Texture2D.MipLevels = 1;
			viewDesc.Texture2D.MostDetailedMip = 0;
			viewDesc.Format = texDesc.Format;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

			hRet = device->CreateShaderResourceView( m_texture, &viewDesc, &m_textureView );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create font texture view: 0x%08X\n", hRet );
 				return false;
			}
		}

		// setup index buffer
		m_indexBuffer = utils::CreateDynamicIndexBuffer( device, MAX_RENDER_INDICES );

		// setup vertex buffer
		m_vertexBuffer = utils::CreateDynamicVertexBuffer( device, MAX_RENDER_VERTICES * sizeof(FontVertex) );

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
			desc.RenderTarget[0].BlendEnable = true; // for now
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].RenderTargetWriteMask = 0xF;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;			
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

			HRESULT hRet = device->CreateBlendState( &desc, &m_blendState );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create blend state: 0x%08X\n", hRet );
				return false;
			}
		}

		// parse the binary font data
		if ( !ParseFontData() )
		{
			fprintf( stderr, "Font internal data error\n" );
			return false;
		}

		// font is ready for drawing
		return true;
	}

	namespace helper
	{
#pragma pack(push, 1)
		struct FontChunkInfo
		{
			unsigned char id;
			int size;
		};

		struct FontCharInfo
		{
			unsigned int id;
			unsigned short x;
			unsigned short y;
			unsigned short width;
			unsigned short height;
			short xOffset;
			short yOffset;
			short xAdvance;
			char page;
			char chnl;
		};
#pragma pack(pop)

	} // helper

	// http://www.angelcode.com/products/bmfont/doc/file_format.html#bin
	bool RenderFont::ParseFontData()
	{
		const unsigned char* data = &app::font::FontData[0];
		const int size = sizeof(app::font::FontData);

		if ( data[0] != 'B' || data[1] != 'M' || data[2] != 'F' || data[3] != 3 ) 
		{
			fprintf( stderr, "Font data invalid!\n" );
			return false;
		}

		const float texInvWidth = 1.0f / 256.0f;
		const float texInvHeight = 1.0f / 128.0f;

		int pos = 4; // past header
		while ( pos < size )
		{
			const auto* chunk = (const helper::FontChunkInfo*)( data + pos );

			if ( chunk->id == 4 /* chars*/ )
			{
				assert( (chunk->size % sizeof(helper::FontCharInfo)) == 0 );
				const int numChars = chunk->size / sizeof(helper::FontCharInfo);

				// process the char data
				const auto* charData = (const helper::FontCharInfo*)( data + pos + sizeof(helper::FontChunkInfo));
				for ( int i=0; i<numChars; ++i, ++charData )
				{
					assert( charData->id < 256 );
					auto& charInfo = m_chars[ charData->id ];

					charInfo.x0 = (float) charData->x * texInvWidth;
					charInfo.y0 = (float) charData->y * texInvHeight;
					charInfo.x1 = (float)( charData->x + charData->width ) * texInvWidth;
					charInfo.y1 = (float)( charData->y + charData->height ) * texInvHeight;
					charInfo.width = (float) charData->width;
					charInfo.height = (float) charData->height;
					charInfo.xAdvance = (float) charData->xAdvance;
					charInfo.xOffset  = (float) charData->xOffset;
					charInfo.yOffset = (float) charData->yOffset;
				}
			}

			pos += sizeof(helper::FontChunkInfo) + chunk->size;
		}

		// done
		return true;
	}

	void RenderFont::Draw( ID3D11DeviceContext* deviceContext, const int x, const int y, const unsigned int color, const char* txt ) const
	{
		int numChars = (int)strlen(txt);
		if ( numChars > MAX_RENDER_CHARS )
			numChars = MAX_RENDER_CHARS;

		int indexAllocOffset = 0;
		int vertexAllocOffset = 0;

		int numVertices = 0;
		int numIndices = 0;

		{
			utils::BufferWriter indexWriter( deviceContext, m_indexBuffer, 6*sizeof(int)*numChars, indexAllocOffset );
			utils::BufferWriter vertexWriter( deviceContext, m_vertexBuffer, 4*sizeof(FontVertex)*numChars, vertexAllocOffset );

			float curX = (float)x;
			float curY = (float)y;

			for ( int i = 0; i<numChars; ++i )
			{
				const auto& charData = m_chars[ (unsigned char)txt[i] ];
				if ( charData.xAdvance )
				{
					if ( txt[i] > 32 )
					{
						{
							FontVertex v;
							v.color = color;
							v.x = curX + charData.xOffset;
							v.y = curY + charData.yOffset;
							v.u = charData.x0;
							v.v = charData.y0;
							vertexWriter.Write(v);
						}

						{
							FontVertex v;
							v.color = color;
							v.x = curX + charData.xOffset + charData.width;
							v.y = curY + charData.yOffset;
							v.u = charData.x1;
							v.v = charData.y0;
							vertexWriter.Write(v);
						}

						{
							FontVertex v;
							v.color = color;
							v.x = curX + charData.xOffset + charData.width;
							v.y = curY + charData.yOffset + charData.height;
							v.u = charData.x1;
							v.v = charData.y1;
							vertexWriter.Write(v);
						}


						{
							FontVertex v;
							v.color = color;
							v.x = curX + charData.xOffset;
							v.y = curY + charData.yOffset + charData.height;
							v.u = charData.x0;
							v.v = charData.y1;
							vertexWriter.Write(v);
						}

						indexWriter.Write<int>( numVertices + 0 );
						indexWriter.Write<int>( numVertices + 1 );
						indexWriter.Write<int>( numVertices + 2 );

						indexWriter.Write<int>( numVertices + 0 );
						indexWriter.Write<int>( numVertices + 2 );
						indexWriter.Write<int>( numVertices + 3 );

						numVertices += 4;
						numIndices += 6;
					}

					curX += charData.xAdvance;
				}
			}
		}

		UINT stride = sizeof(FontVertex);
		UINT offsets = vertexAllocOffset;
		deviceContext->IASetInputLayout( m_vertexLayout );
		deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		deviceContext->IASetVertexBuffers( 0, 1, m_vertexBuffer->GetBufferPtr(), &stride, &offsets );
		deviceContext->IASetIndexBuffer( m_indexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, indexAllocOffset );

		deviceContext->PSSetShaderResources( 0, 1, &m_textureView );
		deviceContext->PSSetShader( m_pixelShader, NULL, 0 );
		deviceContext->VSSetShader( m_vertexShader, NULL, 0 );
		deviceContext->RSSetState( m_rasterState );

		FLOAT blendFactor[] = {0,0,0,0};
		deviceContext->OMSetBlendState( m_blendState, blendFactor, 0xFFFFFFFF );
		deviceContext->OMSetDepthStencilState( m_depthState, 0 );

		deviceContext->DrawIndexed( numIndices, 0, 0 );
	}

} // app