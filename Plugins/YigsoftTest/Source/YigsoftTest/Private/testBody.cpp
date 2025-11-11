

#include "testBody.h"
#include "testShape.h"


#include "../external/framework/include/framework.h"


#include <vector>
#include <cmath>


const float test::Gravitation = 100.0f;

namespace test
{

	float const BodySpeed = 500.0f;     
	float const AttractorRange = 1000.0f; 
	
	Body::Body(IShape* shape, const unsigned int color, float x, float y, int shapeTypeID) :
		m_shape(shape),
		m_color(color),
		m_x(x),
		m_y(y),
		m_velX((float)(rand() % 200 - 100)),
		m_velY((float)(rand() % 200 - 100)),
		m_shapeTypeID(shapeTypeID)
	{
	}

	
	void Body::Update(const std::vector< Body* >& otherBodies, int currentScenario)
	{
		float dirX = 0.0f;
		float dirY = 0.0f;

		if (FindAttractor(otherBodies, currentScenario, dirX, dirY))
		{
			SolveAttraction(dirX, dirY);
		}

		SolveCollision(otherBodies);

	}


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

			
			if (currentScenario == 1)
			{
				if (GetShapeTypeID() != otherBody->GetShapeTypeID())
				{
					continue; 
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
		
		const float CorrectionBias = 0.2f;

		
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

			
				float nx = dx / dist;
				float ny = dy / dist;

				float sumRadii = GetRadius() + otherBody->GetRadius();
				float penetration = sumRadii - dist;

				if (penetration > 0.0f)
				{
					
					float correctionMagnitude = penetration * CorrectionBias;

					
					float correctionX = correctionMagnitude * nx * 0.5f;
					float correctionY = correctionMagnitude * ny * 0.5f;

					m_x -= correctionX;
					m_y -= correctionY;

					otherBody->m_x += correctionX;
					otherBody->m_y += correctionY;
				}

			
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