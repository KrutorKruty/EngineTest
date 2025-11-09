/// (C) Yigsoft 2023

#pragma once

#include <d3d11.h>

namespace app
{
	namespace utils
	{
		class BufferInfo;
	}

	/// Lines rendering
	class RenderLines
	{
	public:
		RenderLines();
		~RenderLines();

		bool Init( ID3D11Device* device );
		void Draw( ID3D11DeviceContext* deviceContext, const RenderVertex* vertice, const int numVertices );

	private:
		static const int MAX_RENDER_VERTICES = 1024*1024;

		ID3D11PixelShader*			m_pixelShader;
		ID3D11VertexShader*			m_vertexShader;
		ID3D11InputLayout*			m_vertexLayout;

		utils::BufferInfo*			m_vertexBuffer;

		ID3D11RasterizerState*		m_rasterState;
		ID3D11DepthStencilState*	m_depthState;
		ID3D11BlendState*			m_blendState;
	};

} // app