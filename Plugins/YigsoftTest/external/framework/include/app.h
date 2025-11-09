/// (C) Yigsoft 2023

#pragma once

#include "framework.h"

namespace app
{

	/// application interface from which the test apps are derived from
	class IApp
	{
	public:
		virtual ~IApp() {};

		/// called when application is initialized
		virtual bool OnInit( const AppInitContext& initContext ) = 0;

		// called every frame, the timeDelta contains the time from previous call of OnTick
		virtual void OnTick( const float timeDelta ) = 0;

		// called whenever user pressed a key
		virtual void OnKeyPressed( const int keyCode ) = 0;

		// rendering hook that allows to render the scene
		virtual void OnRender( RenderFrame& frame ) const = 0;

		// application was switched - you can sync state if you want to
		virtual void OnAppSwitched( app::IApp* prevApp ) {};
	};

} // app