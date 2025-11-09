/// (C) Yigsoft 2023

#include "framework.h"
#include "utils.h"

#include <d3dcompiler.h>

#pragma comment( lib, "d3dcompiler.lib" )

namespace app
{
	namespace utils
	{

		ID3D11PixelShader* CompilePixelShader( ID3D11Device* device, const char* code )
		{
			ID3DBlob* dataBlob = nullptr;
			ID3DBlob* errorBlob = nullptr;

			HRESULT hRet = D3DCompile(
				code,					// pSrcData
				strlen(code),			// SrcDataLen
				NULL,					// pFilename
				NULL,					// pDefines
				NULL,					// pInclude
				"ps_main",				// pFunctionNAme
				"ps_5_0",				// pProfile
				0,						// Flags1
				0,						// Flags2
				&dataBlob,				// ppShader
				&errorBlob );			// ppErrorMsg

			if ( FAILED(hRet) || !dataBlob )
			{
				fprintf( stderr, "Pixel shader compilation error:\n");

				if ( errorBlob )
					fprintf( stderr, (char*)errorBlob->GetBufferPointer() );

				SAFE_RELEASE(errorBlob);
				SAFE_RELEASE(dataBlob);

				return nullptr;
			}

			ID3D11PixelShader* pixelShader = nullptr;
			hRet = device->CreatePixelShader( dataBlob->GetBufferPointer(), dataBlob->GetBufferSize(), NULL, &pixelShader );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Unable to create pixel shader\n" );
				return nullptr;
			}

			return pixelShader;
		}

		ID3D11VertexShader* CompileVertexShader( ID3D11Device* device, const char* code, ID3DBlob*& outBlob )
		{
			ID3DBlob* dataBlob = nullptr;
			ID3DBlob* errorBlob = nullptr;

			HRESULT hRet = D3DCompile( 
				code,					// pSrcData
				strlen(code),			// SrcDataLen
				NULL,					// pFilename
				NULL,					// pDefines
				NULL,					// pInclude
				"vs_main",				// pFunctionNAme
				"vs_5_0",				// pProfile
				0,						// Flags1
				0,						// Flags2
				&dataBlob,				// ppShader
				&errorBlob );			// ppErrorMsg

			if ( FAILED(hRet) || !dataBlob )
			{
				fprintf( stderr, "Vertex shader compilation error:\n");

				if ( errorBlob )
					fprintf( stderr, (char*)errorBlob->GetBufferPointer() );

				SAFE_RELEASE(errorBlob);
				SAFE_RELEASE(dataBlob);

				return nullptr;
			}

			ID3D11VertexShader* vertexShader = nullptr;
			hRet = device->CreateVertexShader( dataBlob->GetBufferPointer(), dataBlob->GetBufferSize(), NULL, &vertexShader );
			if ( FAILED(hRet) )
			{
				SAFE_RELEASE(errorBlob);
				SAFE_RELEASE(dataBlob);

				fprintf( stderr, "Unable to create vertex shader\n" );
				return nullptr;
			}

			SAFE_RELEASE(errorBlob);
			outBlob = dataBlob;

			return vertexShader;
		}

		//----

		BufferInfo::BufferInfo( ID3D11Buffer* buffer, ID3D11Buffer* copyBuffer, int size )
			: m_buffer( buffer )
			, m_size( size )
			, m_offset( 0 )
			, m_copyLockOffset( -1 )
			, m_copyLockSize( -1 )
			, m_copyBuffer( copyBuffer )
		{
		}

		BufferInfo::~BufferInfo()
		{
			SAFE_RELEASE( m_buffer );
			SAFE_RELEASE( m_copyBuffer );
		}

		void* BufferInfo::Lock( ID3D11DeviceContext* context, int size, int& outOffset )
		{
			if ( m_offset + size > m_size )
				outOffset = 0;
			else
				outOffset = m_offset;

			m_offset = outOffset + size;

			// copy to temporary buffer
			D3D11_MAPPED_SUBRESOURCE mapData;
			HRESULT hRet = context->Map( m_copyBuffer, 0, D3D11_MAP_WRITE, 0, &mapData );
			assert( SUCCEEDED(hRet) );

			m_copyLockOffset = outOffset;
			m_copyLockSize = size;
			return (char*)mapData.pData + outOffset;
		}

		void BufferInfo::UnlockAndUpload( ID3D11DeviceContext* context )
		{
			assert( m_copyLockSize != -1 );
			assert( m_copyLockOffset != -1 );

			// unmap temp shit buffer
			context->Unmap( m_copyBuffer, 0 );

			// setup copy range
			D3D11_BOX box;
			box.left = m_copyLockOffset;
			box.right = m_copyLockOffset + m_copyLockSize;
			box.top = 0;
			box.bottom = 1;
			box.front = 0;
			box.back = 1;

			// copy to final buffer
			context->CopySubresourceRegion( m_buffer, 0, m_copyLockOffset, 0, 0, m_copyBuffer, 0, &box );

			// reset
			m_copyLockOffset = -1;
			m_copyLockSize = -1;
		}

		//----

		BufferInfo* CreateDynamicIndexBuffer( ID3D11Device* device, const int numIndices )
		{
			ID3D11Buffer* buffer = nullptr;
			ID3D11Buffer* copyBuffer = nullptr;

			D3D11_BUFFER_DESC desc;
			memset( &desc, 0, sizeof(desc) );

			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.ByteWidth = numIndices * sizeof(int);
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_DYNAMIC;

			HRESULT hRet = device->CreateBuffer( &desc, NULL, &buffer );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create dynamic index buffer: 0x%08X\n", hRet );
				return nullptr;
			}

			desc.BindFlags = 0;
			desc.ByteWidth = numIndices * sizeof(int);
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_STAGING;

			hRet = device->CreateBuffer( &desc, NULL, &copyBuffer );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create staging buffer for dynamic index buffer: 0x%08X\n", hRet );
				return nullptr;
			}

			return new BufferInfo( buffer, copyBuffer, desc.ByteWidth );
		}

		BufferInfo* CreateDynamicVertexBuffer( ID3D11Device* device, const int dataSize )
		{
			ID3D11Buffer* buffer = nullptr;
			ID3D11Buffer* copyBuffer = nullptr;

			D3D11_BUFFER_DESC desc;
			memset( &desc, 0, sizeof(desc) );

			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.ByteWidth = dataSize;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_DYNAMIC;

			HRESULT hRet = device->CreateBuffer( &desc, NULL, &buffer );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create dynamic vertex buffer: 0x%08X\n", hRet );
				return nullptr;
			}

			desc.BindFlags = 0;
			desc.ByteWidth = dataSize;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_STAGING;

			hRet = device->CreateBuffer( &desc, NULL, &copyBuffer );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create staging buffer for dynamic vertex buffer: 0x%08X\n", hRet );
				return nullptr;
			}

			return new BufferInfo( buffer, copyBuffer, desc.ByteWidth );
		}

		BufferWriter::BufferWriter( ID3D11DeviceContext* context, BufferInfo* info, int size, int& outOffset )
			: m_context( context )
			, m_buffer( info )
			, m_writePtr( nullptr )
			, m_endPtr( nullptr )
		{
			m_writePtr = (unsigned char*) m_buffer->Lock( context, size, outOffset );
			m_endPtr = m_writePtr + size;

			m_lockOffset = outOffset;
		}

		BufferWriter::~BufferWriter()
		{
			if ( m_buffer != nullptr )
			{
				m_buffer->UnlockAndUpload( m_context );

				m_writePtr = nullptr;
				m_endPtr = nullptr;
			}
		}

	} // utils

} // app