/// (C) Yigsoft 2023

#include "framework.h"
#include "app.h"
#include "renderer.h"

namespace app
{

	app::Framework* app::Framework::st_globalFrameworkInstance = nullptr;


	Framework& Framework::GetGlobalInstance()
	{
		if (st_globalFrameworkInstance == nullptr)
		{
			st_globalFrameworkInstance = new Framework();
		}
		return *st_globalFrameworkInstance;
	}


	const RenderFrame& Framework::GetRenderFrame() const
	{
		return *m_frame;
	}



	Framework::Framework()
		: m_window(nullptr)
		, m_renderer(nullptr)
		, m_frame(nullptr)
		, m_done(false)
		, m_currentApp(0)
		, m_lastAppTickTime(0.0)
		, m_lastAppRenderTime(0.0)
		, m_avgAppTickTime(0.0)
		, m_avgAppRenderTime(0.0)
		, m_lastAvgAppTickTime(0.0)
		, m_lastAvgAppRenderTime(0.0)
		, m_numAvgFrames(0)
	{
	}

	Framework::~Framework()
	{
		delete m_frame;
		delete m_renderer;
		delete m_window;

		for (auto* ptr : m_apps)
			delete ptr;
	}

	void Framework::RegisterApp(IApp* app)
	{
		if (app)
			m_apps.push_back(app);
	}

	bool Framework::Init()
	{
		// no apps
		if (m_apps.empty())
			return false;

		// create app window
		m_window = new Window();

		// install hook for the viewport
		ConnectWindowHook();

		// create renderer
		m_renderer = new Renderer();
		if (!m_renderer->Init(m_window->GetHandle()))
			return false;

		// create renderable frame
		m_frame = new RenderFrame();

		AppInitContext initContext;
		initContext.m_width = (DWORD)Resolution::WIDTH;
		initContext.m_height = (DWORD)Resolution::HEIGHT;
		initContext.m_device = m_renderer->GetDevice();
		initContext.m_deviceContext = m_renderer->GetDeviceContext();

		// initialize user applications
		for (auto* ptr : m_apps)
			if (!ptr->OnInit(initContext))
				return false;

		// framework initialized
		return true;
	}

	void Framework::Loop()
	{
		auto prev = Timer::GetInstance().GetNow();

		while (!m_done)
		{
			auto cur = Timer::GetInstance().GetNow();
			auto delta = Timer::GetInstance().ToSecondsIntv(cur - prev);
			prev = cur;

			// pump windows messages
			PumpMessages();

			// process buffered input
			ProcessInput();

			// tick app
			{
				auto timeDelta = (float)delta;
				if (timeDelta > 0.01f)
					timeDelta = 0.01f;

				Tick(timeDelta);
			}

			// render app
			Render();

			// process counted frame averages
			ProcessAverage();
		}
	}

	void Framework::PumpMessages()
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
			{
				RequestExit();
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Framework::BufferInput(const int pressedKey)
	{
		if (pressedKey)
		{
			std::lock_guard< std::mutex > lock(m_inputBufferLock);
			m_inputBuffer.push_back(pressedKey);
		}
	}

	void Framework::RequestExit()
	{
		m_done = true;
	}

	void Framework::ProcessInput()
	{
		std::vector< int > grabbedKeys;

		{
			std::lock_guard< std::mutex > lock(m_inputBufferLock);
			grabbedKeys = std::move(m_inputBuffer);
		}

		for (const int key : grabbedKeys)
			ProcessInputKey(key);
	}

	void Framework::ProcessInputKey(const int pressedKey)
	{
		// app selection
		if (pressedKey >= '1' && pressedKey <= '9')
		{
			const int appIndex = pressedKey - '1';
			if (appIndex < (int)m_apps.size())
			{
				if (m_currentApp != appIndex)
				{
					auto* prevApp = m_apps[m_currentApp];
					m_currentApp = appIndex;

					// sync app state
					m_apps[m_currentApp]->OnAppSwitched(prevApp);

					// reset timing
					ResetAverages();
				}
			}
		}

		// close 
		else if (pressedKey == VK_ESCAPE)
		{
			RequestExit();
		}

		// pass to app
		else
		{
			auto* app = m_apps[m_currentApp];
			app->OnKeyPressed(pressedKey);
		}
	}

	void Framework::Tick(const float timeDelta)
	{
		ScopedTimer timer; // for timing user implementation

		auto* app = m_apps[m_currentApp];
		app->OnTick(timeDelta);

		m_lastAppTickTime = timer.GetElaspedTime();
		m_avgAppTickTime += m_lastAppTickTime;
		m_numAvgFrames += 1;
	}

	void Framework::Render()
	{
		m_frame->Reset();

		{
			ScopedTimer timer; // for timing user implementation

			auto* app = m_apps[m_currentApp];
			app->OnRender(*m_frame);

			m_lastAppRenderTime = timer.GetElaspedTime();
			m_avgAppRenderTime += m_lastAppRenderTime;
		}

		// stats (not coutned in user section)
		RenderStats(*m_frame);

		// present
		m_renderer->Render(*m_frame);
	}

	void Framework::RenderStats(RenderFrame& frame)
	{
		frame.AddString(10, 10, RGB(255, 255, 255), "Render: %6.2f ms  (avg: %6.3fms)", 1000.0 * m_lastAppRenderTime, 1000.0 * m_lastAvgAppRenderTime);
		frame.AddString(10, 30, RGB(255, 255, 255), "Tick: %6.2f ms  (avg: %6.3fms)", 1000.0 * m_lastAppTickTime, 1000.0 * m_lastAvgAppTickTime);
		frame.AddString(10, 50, RGB(190, 190, 190), "Press 1-9 to switch between apps");
	}

	namespace helper
	{
		class FrameworkWindowHook : public WindowListener
		{
		public:
			FrameworkWindowHook(Framework* framework)
				: m_framework(framework)
			{
			}

			virtual void FrameworkWindowHook::OnClose() override
			{
				m_framework->RequestExit();
			}

			virtual void FrameworkWindowHook::OnKeyPress(const int keyCode)
			{
				m_framework->BufferInput(keyCode);
			}

		private:
			Framework* m_framework;
		};
	}

	void Framework::ConnectWindowHook()
	{
		WindowListenerPtr ptr(new helper::FrameworkWindowHook(this));
		m_window->SetListener(std::move(ptr));
	}

	void Framework::ResetAverages()
	{
		m_lastAvgAppRenderTime = 0.0;
		m_lastAvgAppTickTime = 0.0;
		m_avgAppRenderTime = 0.0;
		m_avgAppTickTime = 0.0;
		m_numAvgFrames = 0;
	}

	void Framework::ProcessAverage()
	{
		if (m_numAvgFrames >= 100)
		{
			m_lastAvgAppRenderTime = m_avgAppRenderTime / (double)m_numAvgFrames;
			m_lastAvgAppTickTime = m_avgAppTickTime / (double)m_numAvgFrames;

			m_avgAppRenderTime = 0.0;
			m_avgAppTickTime = 0.0;
			m_numAvgFrames = 0;
		}
	}

	//-----

	Timer::Timer()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);
		QueryPerformanceCounter((LARGE_INTEGER*)&m_base);
	}

	Timer::TTicks Timer::GetNow()
	{
		Timer::TTicks ret = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&ret);
		return ret;
	}

	double Timer::ToSeconds(TTicks time)
	{
		return (double)(time - m_base) / (double)m_freq;
	}

	double Timer::ToSecondsIntv(TTicks time)
	{
		return (double)(time) / (double)m_freq;
	}

	Timer Timer::st_instance;

	//-----

	bool Window::st_classRegistered = false;
	const wchar_t* Window::st_className = L"YigsoftTestFramework";

	Window::Window()
		: m_hwnd(NULL)
		, m_listener(nullptr)
	{
		RegisterWndClass();
		CreateWnd();
	}

	Window::~Window()
	{
		DestroyWnd();
	}

	void Window::SetListener(WindowListenerPtr listener)
	{
		m_listener = std::move(listener);
	}

	LRESULT CALLBACK Window::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_CREATE)
		{
			LPCREATESTRUCT data = (LPCREATESTRUCT)lParam;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data->lpCreateParams);
			auto* window = (Window*)data->lpCreateParams;
			window->m_hwnd = hWnd;
		}

		// Process messages by window message function
		auto window = (Window*) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (window)
		{
			return window->WndProc(uMsg, wParam, lParam);
		}
		else
		{
			return static_cast<LRESULT>(DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
	}

	LRESULT CALLBACK Window::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			// Window is closing
		case WM_CLOSE:
		{
			if (m_listener)
				m_listener->OnClose();

			PostQuitMessage(0);
			break;
		}

		// Key pressed
		case WM_KEYDOWN:
		{
			if (m_listener)
				m_listener->OnKeyPress((int)wParam);

			return 0;
		}
		}

		return static_cast<LRESULT>(DefWindowProc(m_hwnd, uMsg, wParam, lParam));
	}

	void Window::CreateWnd()
	{
		DWORD dwStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
		DWORD dxExStyle = 0;

		RECT windowRect;
		windowRect.left = 50;
		windowRect.top = 50;
		windowRect.right = windowRect.left + (int)Resolution::WIDTH;
		windowRect.bottom = windowRect.top + (int)Resolution::HEIGHT;

		AdjustWindowRect(&windowRect, dwStyle, FALSE);

		CreateWindowW(
			st_className,
			L"Yigsoft - Physics Test (C) 2010-2021. Have Fun!",
			dwStyle,
			windowRect.left, windowRect.top,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL, NULL, GetModuleHandle(NULL), this);

		assert(m_hwnd != NULL);
	}

	void Window::DestroyWnd()
	{
		if (m_hwnd != NULL)
		{
			DestroyWindow(m_hwnd);
			m_hwnd = nullptr;
		}
	}

	void Window::RegisterWndClass()
	{
		if (!st_classRegistered)
		{
			WNDCLASSEXW wcex;

			ZeroMemory(&wcex, sizeof(wcex));
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = &StaticWndProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = GetModuleHandle(NULL);
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = st_className;

			RegisterClassExW(&wcex);

			st_classRegistered = true;
		}
	}

	//-----

	RenderFrame::RenderFrame()
	{
		Reset();
	}

	void RenderFrame::Reset()
	{
		m_currentColor = 0xFFFFFFFF;
		m_writePtr = &m_vertices[0];
		m_endPtr = &m_vertices[MAX_VERTICES - 2];

		m_strings.clear();
	}

	void RenderFrame::AddString(const int x, const int y, const DWORD color, const char* txt, ...)
	{
		va_list args;
		char buf[1024];

		va_start(args, txt);
		vsprintf_s(buf, txt, args);
		va_end(args);

		RenderString info;
		info.x = x;
		info.y = y;
		info.color = color;
		info.text = buf;

		m_strings.push_back(info);
	}

	//-----

}