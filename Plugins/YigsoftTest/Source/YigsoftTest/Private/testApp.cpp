#include "testApp.h"
#include "testShape.h"
#include "testBody.h"
#include "../external/framework/include/physicsTestApp.h"

namespace test
{


	App::App()
		: test::PhysicsTestApp( "Your solution" )
	{
		m_bodies.reserve( MaxBodies );
	}

	bool App::OnInit( const app::AppInitContext& initContext )
	{
		if ( !PhysicsTestApp::OnInit( initContext ) )
			return false;

		return true;
	}

	void App::OnTick( const float timeDelta )
	{
		for ( auto* body : m_bodies )
			body->Update( m_bodies );

		for ( auto* body : m_bodies )
			body->Integrate( timeDelta );
	}

	void App::OnKeyPressed( const int keyCode )
	{
		PhysicsTestApp::OnKeyPressed( keyCode );

		// you can have custom keys connected here
	}

	void App::OnRender( app::RenderFrame& frame ) const
	{
		PhysicsTestApp::OnRender( frame );

		for ( auto* body : m_bodies )
			body->Render( frame );

		// you can have additional rendering here
	}

	/// TestSpecific section

	int App::GetNumBodies() const
	{
		return (int) m_bodies.size();
	}

	void App::AddBody( int shapeType, float x, float y, float r )
	{
		IShape* shape = nullptr;

		unsigned int color = 0;

		switch ( shapeType )
		{
			case 0:
			{
				shape = new TriShape(r); 
				color = RGB(180,64,180);
				break;
			}

			case 1:
			{
				shape = new QuadShape(r);
				color = RGB(64,180,180);
				break;
			}

			case 2:
			{
				shape = new HexShape(r);
				color = RGB(180,180,64);
				break;
			}
		}

		if ( shape )
		{
			m_bodies.push_back( new Body( shape, color, x, y ) );
		}
	}

	void App::RemoveBodies( int numObjects )
	{
		while ( numObjects-- && !m_bodies.empty() )
		{
			delete m_bodies.back();
			m_bodies.pop_back();
		}
	}

	const std::vector< Body* >& App::GetBodies() const
	{
		return m_bodies;
	}

} // test