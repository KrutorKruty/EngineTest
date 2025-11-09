#include "testBody.h"
#include "testShape.h"
#include "../external/framework/include/framework.h" 


const float test::Gravitation = 100.0f;

namespace test
{


	Body::Body(IShape* shape, const unsigned int color, float x, float y) :
		m_shape(shape),
		m_color(color),
		m_x(x),
		m_y(y),
		m_velX((float)(rand() % 200 - 100)),
		m_velY((float)(rand() % 200 - 100))
	{
	}

	void Body::Update(const std::vector< Body* >& otherBodies)
	{
		float dirX = 0.0f;
		float dirY = 0.0f;

		if (FindAttractor(otherBodies, dirX, dirY))
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

	bool Body::FindAttractor(const std::vector< Body* >& otherBodies, float& outDirX, float& outDirY) const
	{
		for (auto* otherBody : otherBodies)
		{
			if (otherBody == this)
				continue;

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
		for (auto* otherBody : otherBodies)
		{
			if (otherBody == this)
				continue;

	
			if (IShape::TestOverlap(m_shape, m_x, m_y, otherBody->m_shape, otherBody->m_x, otherBody->m_y))
			{
				float dx = otherBody->m_x - m_x;
				float dy = otherBody->m_y - m_y;

				float distSq = dx * dx + dy * dy;

				m_velX -= dx * 0.1f;
				m_velY -= dy * 0.1f;
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