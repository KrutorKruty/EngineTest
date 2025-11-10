#include "testBody.h"
#include "testShape.h"
#include "../external/framework/include/framework.h"


const float test::Gravitation = 100.0f;

namespace test
{
	// Corrected Body Constructor Definition
	// The initialization list must precede the opening brace {
	Body::Body(IShape* shape, const unsigned int color, float x, float y, int shapeTypeID) :
		m_shape(shape),
		m_color(color),
		m_x(x),
		m_y(y),
		m_velX((float)(rand() % 200 - 100)),
		m_velY((float)(rand() % 200 - 100)),
		m_shapeTypeID(shapeTypeID)
	{
		// Note: The warning C5038 can be ignored here, but for clean code, 
		// m_shapeTypeID should be declared before m_velX/m_velY in testBody.h.
	}

	void Body::Update(const std::vector< Body* >& otherBodies, int currentScenario)
	{
		float dirX = 0.0f;
		float dirY = 0.0f;

		// The call to FindAttractor now correctly passes currentScenario
		if (FindAttractor(otherBodies, currentScenario, dirX, dirY))
		{
			SolveAttraction(dirX, dirY);
		}

		SolveCollision(otherBodies);

	} // Correct closing brace for Body::Update


	void Body::Integrate(float deltaTime)
	{
		const float length = sqrtf(m_velY * m_velY + m_velX * m_velX);

		if (length > 0.0f)
		{
			m_velX /= length;
			m_velY /= length;
		}

		m_x += m_velX * deltaTime * test::BodySpeed;
		m_y += m_velY * deltaTime * test::BodySpeed;

		WrapAround();
	}


	void Body::Render(app::RenderFrame& frame)
	{
		frame.SetColor(m_color);
		m_shape->Render(m_x, m_y, frame);
	}

	bool Body::FindAttractor(const std::vector< Body* >& otherBodies, int currentScenario, float& outDirX, float& outDirY) const
	{
		for (auto* otherBody : otherBodies)
		{
			if (otherBody == this)
				continue;

			// Scenario 1: Attraction only occurs between bodies of the same shape type
			if (currentScenario == 1)
			{
				if (GetShapeTypeID() != otherBody->GetShapeTypeID())
				{
					continue; // Skip attraction if shapes are different types.
				}
			}

			float dx = otherBody->m_x - m_x;
			float dy = otherBody->m_y - m_y;
			float distSq = dx * dx + dy * dy;

			if (distSq < test::AttractorRange * test::AttractorRange)
			{
				float dist = sqrtf(distSq);
				outDirX = dx / dist;
				outDirY = dy / dist;
				return true;
			}
		}

		return false;
	}

	void Body::SolveAttraction(float dirX, float dirY)
	{
		m_velX += dirX * test::Gravitation;
		m_velY += dirY * test::Gravitation;
	}

	void Body::SolveCollision(const std::vector< Body* >& otherBodies)
	{
		// Factor for Positional Correction. Determines how much of the overlap to fix.
		const float CorrectionBias = 0.2f;

		// Low repulsion factor (since we are also correcting position)
		const float RepulsionFactor = 0.1f;

		for (auto* otherBody : otherBodies)
		{
			if (otherBody == this)
				continue;

			if (IShape::TestOverlap(m_shape, m_x, m_y, otherBody->m_shape, otherBody->m_x, otherBody->m_y))
			{
				float dx = otherBody->m_x - m_x;
				float dy = otherBody->m_y - m_y;

				float distSq = dx * dx + dy * dy;

				if (distSq == 0.0f) continue;

				float dist = sqrtf(distSq);

				// 1. Calculate the Normalized Direction/Normal (nx, ny)
				float nx = dx / dist;
				float ny = dy / dist;

				// 2. Approximate Penetration Depth (p)
				float sumRadii = GetRadius() + otherBody->GetRadius();
				float penetration = sumRadii - dist;

				// --- POSITIONAL CORRECTION (Moves bodies out of overlap) ---
				if (penetration > 0.0f)
				{
					// Calculate how much to move THIS body and the OTHER body
					float correctionMagnitude = penetration * CorrectionBias;

					// Push THIS body away from the OTHER body (using -nx, -ny)
					// Split the correction evenly since we assume equal mass.
					float correctionX = correctionMagnitude * nx * 0.5f;
					float correctionY = correctionMagnitude * ny * 0.5f;

					m_x -= correctionX;
					m_y -= correctionY;

					otherBody->m_x += correctionX;
					otherBody->m_y += correctionY;
				}

				// --- VELOCITY CORRECTION (Applies a final separating push) ---
				// This is the original, but now weakened, velocity correction.
				m_velX -= dx * RepulsionFactor;
				m_velY -= dy * RepulsionFactor;
			}
		}
	}


	void Body::WrapAround()
	{
		if (m_x > (float)app::Resolution::WIDTH)
			m_x -= (float)app::Resolution::WIDTH;
		else if (m_x < 0.0f)
			m_x += (float)app::Resolution::WIDTH;

		if (m_y > (float)app::Resolution::HEIGHT)
			m_y -= (float)app::Resolution::HEIGHT;
		else if (m_y < 0.0f)
			m_y += (float)app::Resolution::HEIGHT;
	}


	float Body::GetRadius() const
	{
		return m_shape ? m_shape->GetRadius() : 0.0f;
	}


}