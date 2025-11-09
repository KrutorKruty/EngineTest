/// (C) Yigsoft 2023

#include "framework.h"
#include "app.h"
#include "renderer.h"
#include "renderFont.h"
#include "renderLines.h"
#include "utils.h"

#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

namespace app
{



	Renderer::Renderer()
		: m_swapchain( nullptr )
		, m_device( nullptr )
		, m_deviceContext ( nullptr )
		, m_backBufferView( nullptr )
		, m_depthStencil( nullptr )
		, m_depthStencilView( nullptr )
		, m_fontRenderer( nullptr )
		, m_linesRenderer( nullptr )
	{
	}

	Renderer::~Renderer()
	{
		delete m_fontRenderer;
		delete m_linesRenderer;
		SAFE_RELEASE( m_backBufferView );
		SAFE_RELEASE( m_deviceContext );
		SAFE_RELEASE( m_depthStencil );
		SAFE_RELEASE( m_depthStencilView );
		SAFE_RELEASE( m_device );
	}

	bool Renderer::Init( HWND hWnd )
	{
		IDXGIFactory* factory = nullptr;

		HRESULT hRet = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		if (FAILED(hRet) )
		{
			fprintf( stderr, "Error creating DXGI: 0x%08X\n", hRet );
			return false;
		}

		// set regular 32-bit surface for the back buffer.
		DXGI_SWAP_CHAIN_DESC swd;
		ZeroMemory(&swd, sizeof(swd));
		swd.BufferCount = 1;
		swd.BufferDesc.Width = (int) Resolution::WIDTH;
		swd.BufferDesc.Height = (int) Resolution::HEIGHT;
		swd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swd.BufferDesc.RefreshRate.Numerator = 0;
		swd.BufferDesc.RefreshRate.Denominator = 1;
		swd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swd.OutputWindow = hWnd;
		swd.SampleDesc.Count = 1;
		swd.SampleDesc.Quality = 0;
		swd.Windowed = true;
		swd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swd.Flags = 0;

		// create device
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		hRet = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swd, &m_swapchain, &m_device, NULL, &m_deviceContext );
		if ( FAILED(hRet) )
		{
			fprintf( stderr, "Error creating device: 0x%08X\n", hRet );
			return false;
		}

		// get the pointer to the back buffer
		ID3D11Texture2D* backBuffer = nullptr;
		hRet = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		if (FAILED(hRet))
		{
			fprintf( stderr, "Error getting swap chain: 0x%08X\n", hRet );
			return false;
		}

		// create the render target view with the back buffer pointer.
		hRet = m_device->CreateRenderTargetView( backBuffer, NULL, &m_backBufferView );
		backBuffer->Release();
		if (FAILED(hRet))
		{
			fprintf( stderr, "Error creating render target: 0x%08X\n", hRet );
			return false;				
		}

		// create depth surface
		{
			D3D11_TEXTURE2D_DESC texDesc;
			memset( &texDesc, 0, sizeof(texDesc) );
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	
			texDesc.Width = swd.BufferDesc.Width;
			texDesc.Height = swd.BufferDesc.Height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;

			HRESULT hRet = m_device->CreateTexture2D( &texDesc, NULL, &m_depthStencil );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create depth stencil texture: 0x%08X\n", hRet );
				return false;
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
			memset( &viewDesc, 0, sizeof(viewDesc) );
			viewDesc.Flags = 0;
			viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			viewDesc.Format = texDesc.Format;
			viewDesc.Texture2D.MipSlice = 0;

			hRet = m_device->CreateDepthStencilView( m_depthStencil, &viewDesc, &m_depthStencilView );
			if ( FAILED(hRet) )
			{
				fprintf( stderr, "Failed to create depth stencil view: 0x%08X\n", hRet );
				return false;
			}
		}

		// initialize font renderer
		m_fontRenderer = new RenderFont();
		if ( !m_fontRenderer->Init( m_device ) )
		{
			fprintf( stderr, "Error creating font renderer\n" );
			return false;
		}

		// initialize line renderer
		m_linesRenderer = new RenderLines();
		if ( !m_linesRenderer->Init( m_device ) )
		{
			fprintf( stderr, "Error creating line renderer\n" );
			return false;
		}

		return true;
	}

	void Renderer::Render( const RenderFrame& frame )
	{
		const FLOAT clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
		m_deviceContext->ClearRenderTargetView( m_backBufferView, clearColor );

		{
			m_deviceContext->OMSetRenderTargets( 1, &m_backBufferView, m_depthStencilView );

			// setup viewport
			{
				D3D11_VIEWPORT viewport;
				viewport.Width = (int) Resolution::WIDTH;
				viewport.Height = (int) Resolution::HEIGHT;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;
				m_deviceContext->RSSetViewports( 1, &viewport );
			}

			// render lines
			m_linesRenderer->Draw( m_deviceContext, frame.m_vertices, frame.GetNumVertices() );

			// render strings (on top)
			for ( const auto& str : frame.m_strings )
				m_fontRenderer->Draw( m_deviceContext, str.x, str.y, str.color, str.text.c_str() );
		}

		m_swapchain->Present( 0, 0 );
	}

	//-----
}


