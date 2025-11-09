/// (C) Yigsoft 2023

#pragma once

#include <d3d11.h>

/// Generated using: http://www.angelcode.com/products/bmfont/

namespace app
{
	namespace utils
	{

		class BufferInfo
		{
		public:
			BufferInfo( ID3D11Buffer* buffer, ID3D11Buffer* copyBuffer, int size );
			~BufferInfo();

			inline ID3D11Buffer* GetBuffer() { return m_buffer; }
			inline ID3D11Buffer** GetBufferPtr() { return &m_buffer; }

			void* Lock( ID3D11DeviceContext* context, int size, int& outOffset );
			void UnlockAndUpload( ID3D11DeviceContext* context );

		public:
			ID3D11Buffer*	m_buffer;
			int				m_size;
			int				m_offset;

			int				m_copyLockOffset;
			int				m_copyLockSize;
			ID3D11Buffer*	m_copyBuffer;
		};

		/// mapped writes
		class BufferWriter
		{
		public:
			BufferWriter( ID3D11DeviceContext* context, BufferInfo* info, int size, int& outOffset );
			~BufferWriter();

			template< typename T >
			inline void Write( const T& data )
			{
				assert( m_writePtr + sizeof(data) < m_endPtr );
				*(T*) m_writePtr = data;
				m_writePtr += sizeof(data);
			}

			inline void* GetData() const
			{
				return  m_writePtr;
			}

		private:
			ID3D11DeviceContext*	m_context;
			BufferInfo*				m_buffer;

			unsigned char*			m_writePtr;
			unsigned char*			m_endPtr;

			int						m_lockOffset;
		};



		/// helper function to compile pixel shader
		ID3D11PixelShader* CompilePixelShader( ID3D11Device* device, const char* code );

		/// helper function to compile vertex shader
		ID3D11VertexShader* CompileVertexShader( ID3D11Device* device, const char* code, ID3DBlob*& outBlob );

		/// create dynamic index buffer, CPU writable
		BufferInfo* CreateDynamicIndexBuffer( ID3D11Device* device, const int numIndices );

		/// create dynamic vertex buffer, CPU writable
		BufferInfo* CreateDynamicVertexBuffer( ID3D11Device* device, const int dataSize );

	} // utils
} // app