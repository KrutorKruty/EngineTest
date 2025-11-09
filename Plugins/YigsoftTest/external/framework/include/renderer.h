/// (C) Yigsoft 2023

#pragma once

#include "framework.h"

namespace app
{
	class RenderFont;
	class RenderLines;

	/// basic DX11 "renderer"
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		inline ID3D11Device* GetDevice() const { return m_device; }
		inline ID3D11DeviceContext*	GetDeviceContext() const { return m_deviceContext; }

		bool Init( HWND hWnd );
		void Render( const RenderFrame& frame );		

	private:
		IDXGISwapChain*		m_swapchain;

		ID3D11Device*			m_device;
		ID3D11DeviceContext*	m_deviceContext;

		ID3D11RenderTargetView*		m_backBufferView;

		ID3D11Texture2D*			m_depthStencil;
		ID3D11DepthStencilView*		m_depthStencilView;

		RenderFont*			m_fontRenderer;
		RenderLines*		m_linesRenderer;
	};

} // app
	