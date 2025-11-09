/// (C) Yigsoft 2023

#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <assert.h>

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

#define FRAMEWORK_SAFE_RELEASE(x) if (x) { (x)->Release(); x = NULL; }

namespace app
{
	class RenderFont;

	enum class Resolution : int
	{
		WIDTH = 1600,
		HEIGHT = 900,
	};

	/// timing provider
	class Timer
	{
	public:
		typedef unsigned __int64 TTicks;

		Timer();

		TTicks GetNow();
		double ToSeconds( TTicks time );
		double ToSecondsIntv( TTicks time );

		static inline Timer& GetInstance()
		{
			return st_instance;
		}

	private:
		TTicks		m_base;
		TTicks		m_freq;

		static Timer st_instance;
	};

	/// scoped timing block
	class ScopedTimer
	{
	public:
		inline ScopedTimer()
			: m_startTime( Timer::GetInstance().GetNow() )
		{}

		inline double GetElaspedTime() const
		{
			const auto delta = (Timer::GetInstance().GetNow() - m_startTime);
			return Timer::GetInstance().ToSecondsIntv( delta );
		}

	private:
		Timer::TTicks		m_startTime;
	};

	/// basic listener for actions done with the window
	class WindowListener
	{
	public:
		~WindowListener() {};

		virtual void OnClose() = 0;
		virtual void OnKeyPress( const int keyCode ) = 0;
	};

	typedef std::unique_ptr<WindowListener> WindowListenerPtr;

	/// basic window implementation using WinAPI
	class Window
	{
	public:
		Window();
		~Window();

		inline HWND GetHandle() const { return m_hwnd; }

		void SetListener( WindowListenerPtr listener );

	private:
		void RegisterWndClass();
		void CreateWnd();
		void DestroyWnd();

		static LRESULT CALLBACK StaticWndProc(HWND, UINT, WPARAM, LPARAM);
		LRESULT CALLBACK WndProc(UINT, WPARAM, LPARAM);

		static bool			st_classRegistered;

		static const wchar_t*	st_className;

		WindowListenerPtr		m_listener;
		HWND					m_hwnd;
	};

	/// additional app initialization data (needed for DirectCompute)
	struct AppInitContext
	{
		DWORD					m_width;
		DWORD					m_height;

		ID3D11Device*			m_device;
		ID3D11DeviceContext*	m_deviceContext;
	};

	/// renderable vertex
	struct RenderVertex
	{
		float x,y;
		DWORD color;
	};

	/// renderable string
	struct RenderString
	{
		int x,y;
		DWORD color;
		std::string text;
	};

	/// collections of data to render that represent a single frame
	class RenderFrame
	{
	public:
		RenderFrame();

		void Reset();

		inline void SetColor( const DWORD color )
		{
			m_currentColor = color;
		}

		inline void AddLine( const float x0, const float y0, const float x1, const float y1 )
		{
			if ( m_writePtr < m_endPtr )
			{
				m_writePtr->color = m_currentColor;
				m_writePtr->x = x0;
				m_writePtr->y = y0;
				++m_writePtr;

				m_writePtr->color = m_currentColor;
				m_writePtr->x = x1;
				m_writePtr->y = y1;
				++m_writePtr;
			}
		}

		void AddString( const int x, const int y, const DWORD color, const char* txt, ... );

		friend class Renderer;

	private:
		static const int MAX_VERTICES = 8 * 100000; // good for around 100k of objects

		DWORD			m_currentColor;

		RenderVertex	m_vertices[ MAX_VERTICES ];
		RenderVertex*	m_writePtr;
		RenderVertex*	m_endPtr;

		inline int GetNumVertices() const
		{
			return (int)( m_writePtr - m_vertices);
		}

		std::vector< RenderString >	m_strings;
	};

	class Renderer;
	class IApp;

	/// framework interface
	class Framework
	{
	public:
		Framework();
		~Framework();

		void RegisterApp( IApp* app );

		bool Init();
		void Loop();

		void BufferInput( const int pressedKey );
		void RequestExit();

	private:
		void Tick( const float timeDelta );
		void Render();

		void ResetAverages();
		void ProcessAverage();

		void ProcessInput();
		void ProcessInputKey( const int pressedKey );

		void ConnectWindowHook();
		void PumpMessages();

		void RenderStats( RenderFrame& frame );

		std::atomic<bool>		m_done;

		int						m_currentApp;
		std::vector< IApp* >	m_apps;

		double			m_lastAppTickTime;
		double			m_lastAppRenderTime;

		double			m_lastAvgAppTickTime;
		double			m_lastAvgAppRenderTime;

		double			m_avgAppTickTime;
		double			m_avgAppRenderTime;
		int				m_numAvgFrames;

		Window*			m_window;
		Renderer*		m_renderer;
		RenderFrame*	m_frame;

		std::mutex			m_inputBufferLock;
		std::vector< int >	m_inputBuffer;
	};


} // app
