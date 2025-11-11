#include "physicsTestApp.h"

namespace test
{

	
	int PhysicsTestApp::GetScenario() const
	{
		// Assumes 'm_currentScenario' is a private member variable
		return m_currentScenario;
	}
	
	void PhysicsTestApp::SetNumBodies(int numBodies)
	{
		// Assumes 'm_numBodies' is a private member variable
		m_numBodies = numBodies;
	}

	void PhysicsTestApp::SetScenario(int scenario)
	{
		// Assumes 'm_currentScenario' is a private member variable
		m_currentScenario = scenario;
	}


	PhysicsTestApp::PhysicsTestApp( const char* appName )
		: m_appName( appName )
		, m_scenario( 0 )
	{
	}

	bool PhysicsTestApp::OnInit( const app::AppInitContext& initContext )
	{
		AddBodies( 1000 );
		return true;
	}

	void PhysicsTestApp::OnKeyPressed( const int keyCode )
	{
		if ( keyCode == VK_OEM_PLUS || keyCode == VK_ADD )
		{
			AddBodies(100);
		}
		else if ( keyCode == VK_OEM_MINUS || keyCode == VK_SUBTRACT )
		{
			RemoveBodies(100);
		}
		else if ( keyCode == 's' || keyCode == 'S' )
		{
			ChangeScenario( (m_scenario+1) % NUM_SCENARIOS );
		}
	}

	void PhysicsTestApp::OnRender( app::RenderFrame& frame ) const
	{
		frame.AddString( 10, 70, RGB(255,255,200), "App: '%hs'", m_appName );
		frame.AddString( 10, 90, RGB(255,255,200), "Number of bodies: %d (+- to change)", GetNumBodies() );
		frame.AddString( 10, 110, RGB(200,255,200), "Scenario:: %d (S to change)", m_scenario );
	}

	void PhysicsTestApp::OnAppSwitched( app::IApp* prevApp )
	{
		// sync the number of bodies between test physics apps
		if ( prevApp && prevApp != this )
		{
			PhysicsTestApp* physicsApp = static_cast< PhysicsTestApp* >( prevApp );

			const int delta = physicsApp->GetNumBodies() - GetNumBodies();
			if ( delta > 0 )
			{
				AddBodies( delta );
			}
			else
			{
				RemoveBodies( -delta );
			}
		}
	}

	namespace helper
	{
		static inline float RandOne()
		{
			return (float)rand() / (float)RAND_MAX;
		}
	}

	void PhysicsTestApp::AddBodies( int numObjects )
	{
		if ( GetNumBodies() + numObjects >= MaxBodies )
			numObjects = (MaxBodies - GetNumBodies());
		
		for ( int i=0; i<numObjects; ++i )
		{
			if ( m_scenario == 0 )
			{
				const int type = rand() % MaxShapeTypes;
				const float x = (float)app::Resolution::WIDTH * helper::RandOne();
				const float y = (float)app::Resolution::HEIGHT * helper::RandOne();
				const float r = MinSize + (MaxSize - MinSize) * helper::RandOne();

				AddBody( type, x, y, r );
			}
			else if ( m_scenario == 1 )
			{
				const int type = rand() % MaxShapeTypes;

				const float w3 = (float)app::Resolution::WIDTH / 3.0f;
				const float x = (type*w3) + w3* helper::RandOne();
				const float y = (float)app::Resolution::HEIGHT * helper::RandOne();
				const float r = MinSize + (MaxSize - MinSize) * helper::RandOne();

				AddBody( type, x, y, r );
			}
		}
	}

	void PhysicsTestApp::ChangeScenario( int newScenario )
	{
		if ( m_scenario != newScenario )
		{
			const int numBodies = GetNumBodies();

			RemoveBodies( GetNumBodies() );
			m_scenario = newScenario;
			AddBodies( numBodies );
		}
	}

} // test

	