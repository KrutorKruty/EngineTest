#pragma once

#include "app.h"

namespace test
{
	const float AttractorRange = 40.0f;
	const float AttractorSteering = 0.077f;
	const float MaxSize = 15.0f;
	const float MinSize = 4.0f;
	const float BodySpeed = 50.0f;
	const int MaxBodies = 10000;
	const int MaxShapeTypes = 3;

	class PhysicsTestApp : public app::IApp
	{
	public:
		PhysicsTestApp( const char* appName );
		
		int GetScenario() const;
		virtual bool OnInit( const app::AppInitContext& initContext ) override;
		virtual void OnKeyPressed( const int keyCode ) override;
		virtual void OnRender( app::RenderFrame& frame ) const override;
		virtual void OnAppSwitched( app::IApp* prevApp ) override;
		void SetNumBodies(int numBodies);
		void SetScenario(int scenario);
		/// get number of bodies in the scene
		virtual int GetNumBodies() const = 0;

		/// add body to the scene
		virtual void AddBody( int shapeType, float x, float y, float r ) = 0;

		/// remove N bodies from the scene
		virtual void RemoveBodies( int numObjects ) = 0;

	private:
		void AddBodies( int numBodies );
		void ChangeScenario( int newScenario );

		const char*		 m_appName;
		int				m_scenario;
		int      m_currentScenario;
		int            m_numBodies;
		static const int NUM_SCENARIOS = 2;
	};

} // app
