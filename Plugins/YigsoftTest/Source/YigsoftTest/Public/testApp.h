#pragma once

#include "../external/framework/include/app.h"
#include "../external/framework/include/physicsTestApp.h"
#include <vector>
#include "framework.h"

namespace test
{
	extern const float AttractorRange;
	extern const float AttractorSteering;
	extern const float BodySpeed;


	class Body;

	class App : public PhysicsTestApp
	{
	public:
		App();
		virtual ~App() = default;

		virtual bool OnInit(const app::AppInitContext& initContext) override;
		virtual void OnTick(const float timeDelta) override;
		virtual void OnKeyPressed(const int keyCode) override;
		virtual void OnRender(app::RenderFrame& frame) const override;


		virtual int GetNumBodies() const override;
		virtual void AddBody(int shapeType, float x, float y, float r) override;
		virtual void RemoveBodies(int numObjects) override;

		const std::vector< Body* >& GetBodies() const;

	private:
		// Helper function to clean up scene
		void ClearAllBodies();

		std::vector< Body* > m_bodies;
	};
}