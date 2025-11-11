

#include "testApp.h" // <-- MUST be the first line
#include "app.h"
#include "testShape.h"
#include "testBody.h"
#include "physicsTestApp.h"
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
	const float SPAWN_Y = 0.0f; // Correct spelling here

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

	// FIX 1: New helper function to delete all body objects and clear the list.
	void App::ClearAllBodies()
	{
		for (Body* body : m_bodies)
		{
			// The Body destructor is responsible for deleting its associated m_shape
			delete body;
		}
		m_bodies.clear();
	}


	void App::OnTick(const float timeDelta)
	{

		const int currentScenario = test::PhysicsTestApp::GetScenario();


		test::PhysicsTestApp::SetNumBodies(GetNumBodies());


		test::PhysicsTestApp::SetScenario(currentScenario);



		for (auto* body : m_bodies)
		{

			body->Update(m_bodies, currentScenario);
		}

		for (auto* body : m_bodies)
			body->Integrate(timeDelta);
	}

	void App::OnKeyPressed(const int keyCode)
	{
		// 1. First, let the base class handle the key press.
		// This is where the base PhysicsTestApp changes the internal scenario index.
		PhysicsTestApp::OnKeyPressed(keyCode);

		// 2. Custom keys connected here: Spawn a new body on key press
		switch (keyCode)
		{
		case KEY_1: // ASCII for '1'
		case KEY_2: // ASCII for '2'
		case KEY_3: // ASCII for '3'
		{
			// FIX 2: If the intent is to switch the *entire scene* when 1, 2, or 3 is pressed, 
			// we must clear the old scene before adding the new shapes.
			ClearAllBodies();

			int shapeType = keyCode - KEY_1; // 0 for '1', 1 for '2', 2 for '3'

			// FIX: Corrected SPAAWN_Y to SPAWN_Y
			AddBody(shapeType, SPAWN_X, SPAWN_Y, DEFAULT_RADIUS);
			break;
		}
		// NOTE: If the base class uses SPACEBAR or ENTER to cycle scenarios, 
		// you might need additional logic here to call ClearAllBodies() when those keys are pressed, 
		// depending on how the base class handles body initialization.
		}
	}

	void App::OnRender(app::RenderFrame& frame) const
	{
		// This base call handles rendering the debug overlay text (now updated in OnTick)
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
			// Pass the shapeType as the new shapeTypeID argument to the Body constructor
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