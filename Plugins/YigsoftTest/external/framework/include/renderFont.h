/// (C) Yigsoft 2023

#pragma once

#include <d3d11.h>

/// Generated using: http://www.angelcode.com/products/bmfont/

namespace app
{
	namespace utils
	{
		class BufferInfo;
	}

	/// Printable font
	class RenderFont
	{
	public:
		RenderFont();
		~RenderFont();

		bool Init( ID3D11Device* device );
		void Draw( ID3D11DeviceContext* deviceContext, const int x, const int y, const unsigned int color, const char* txt ) const;

	private:
		static const int MAX_RENDER_CHARS = 4096;

		static const int MAX_RENDER_VERTICES = 6*MAX_RENDER_CHARS;
		static const int MAX_RENDER_INDICES = 4*MAX_RENDER_CHARS;

		ID3D11Texture2D*			m_texture;
		ID3D11ShaderResourceView*	m_textureView;

		ID3D11PixelShader*			m_pixelShader;
		ID3D11VertexShader*			m_vertexShader;
		ID3D11InputLayout*			m_vertexLayout;

		utils::BufferInfo*			m_indexBuffer;
		utils::BufferInfo*			m_vertexBuffer;

		ID3D11RasterizerState*		m_rasterState;
		ID3D11DepthStencilState*	m_depthState;
		ID3D11BlendState*			m_blendState;

		struct CharInfo
		{
			float x0;
			float y0;
			float x1;
			float y1;
			float width;
			float height;
			float xOffset;
			float yOffset;
			float xAdvance;
		};

#pragma pack(push, 1)
		struct FontVertex
		{
			float x,y;
			float u,v;
			DWORD color;
		};
#pragma pack(pop)

		CharInfo		m_chars[ 256 ];

		bool ParseFontData();
	};

} // app