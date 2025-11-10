#include "testApp.h"
#include "testShape.h"
#include "testBody.h"
#include "../external/framework/include/physicsTestApp.h"

namespace test
{
	// Define the ASCII key codes for clarity
	const int KEY_1 = 49;
	const int KEY_2 = 50;
	const int KEY_3 = 51;

	// Define the radius for the new bodies (adjust as needed)
	const float DEFAULT_RADIUS = 25.0f;
	// Define the spawn location (center of the screen/world)
	const float SPAWN_X = 0.0f;
	const float SPAWN_Y = 0.0f;


	App::App()
		: test::PhysicsTestApp("Your solution")
	{
		m_bodies.reserve(MaxBodies);
	}

	bool App::OnInit(const app::AppInitContext& initContext)
	{
		if (!PhysicsTestApp::OnInit(initContext))
			return false;

		return true;
	}

	void App::OnTick(const float timeDelta)
	{
		// FIX 1: Use the public GetScenario() method from the base class 
		// to retrieve the scenario ID, resolving the C2248 access error.
		const int currentScenario = GetScenario();

		for (auto* body : m_bodies)
		{
			// FIX 2: Pass the currentScenario ID to Body::Update.
			body->Update(m_bodies, currentScenario);
		}

		for (auto* body : m_bodies)
			body->Integrate(timeDelta);
	}

	void App::OnKeyPressed(const int keyCode)
	{
		PhysicsTestApp::OnKeyPressed(keyCode);

		// Custom keys connected here: Spawn a new body on key press
		switch (keyCode)
		{
		case KEY_1: // ASCII for '1'
		{
			// ShapeType 0: Triangle
			AddBody(0, SPAWN_X, SPAWN_Y, DEFAULT_RADIUS);
			break;
		}
		case KEY_2: // ASCII for '2'
		{
			// ShapeType 1: Quad
			AddBody(1, SPAWN_X, SPAWN_Y, DEFAULT_RADIUS);
			break;
		}
		case KEY_3: // ASCII for '3'
		{
			// ShapeType 2: Hexagon
			AddBody(2, SPAWN_X, SPAWN_Y, DEFAULT_RADIUS);
			break;
		}
		}
	}

	void App::OnRender(app::RenderFrame& frame) const
	{
		PhysicsTestApp::OnRender(frame);

		for (auto* body : m_bodies)
			body->Render(frame);

		// you can have additional rendering here
	}

	/// TestSpecific section

	int App::GetNumBodies() const
	{
		return (int)m_bodies.size();
	}

	void App::AddBody(int shapeType, float x, float y, float r)
	{
		IShape* shape = nullptr;
		unsigned int color = 0;

		switch (shapeType)
		{
		case 0:
		{
			shape = new TriShape(r);
			color = RGB(180, 64, 180);
			break;
		}

		case 1:
		{
			shape = new QuadShape(r);
			color = RGB(64, 180, 180);
			break;
		}

		case 2:
		{
			shape = new HexShape(r);
			color = RGB(180, 180, 64);
			break;
		}
		}

		if (shape)
		{
			// FIX 3: Pass the shapeType as the new shapeTypeID argument (shapeType)
			// to the Body constructor, ensuring the FindAttractor logic works.
			m_bodies.push_back(new Body(shape, color, x, y, shapeType));
		}
	}

	void App::RemoveBodies(int numObjects)
	{
		while (numObjects-- && !m_bodies.empty())
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