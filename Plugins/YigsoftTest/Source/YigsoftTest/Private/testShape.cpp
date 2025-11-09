// D:\Engine test\EngineTest\Plugins\YigsoftTest\Source\YigsoftTest\Private\testShape.cpp

// IMPORTANT: Use the correct path for your header. Assuming it's in the Public folder.
#include "../Public/testShape.h"

#undef SAFE_RELEASE

#include "../external/framework/include/framework.h"

// NOTE: MAX_EDGES constant is likely already defined in testShape.h. 
// If it is, delete the line 'const int MAX_EDGES = 6;' below to avoid redefinition errors.

namespace test
{
	// If MAX_EDGES is NOT in the header, define it here:
	const int MAX_EDGES = 6;

	namespace helper
	{
		// Definition of the helper function (must be here, or static inline in the header)
		static bool IsBehindEdge(float startX, float startY, float endX, float endY, float x, float y)
		{
			float nx = -(endY - startY);
			float ny = endX - startX;
			float dot = nx * (x - startX) + ny * (y - startY);
			return dot < 0;
		}
	}

	// --- IShape Implementations ---

	void IShape::Render(float x, float y, app::RenderFrame& frame) const
	{
		float ex[MAX_EDGES];
		float ey[MAX_EDGES];

		int numEdges = 0;
		ComputeEdges(ex, ey, numEdges);

		for (int i = 0; i < numEdges; ++i)
		{
			const float curX = ex[i];
			const float curY = ey[i];

			const float nextX = ex[(i + 1) % numEdges];
			const float nextY = ey[(i + 1) % numEdges];

			frame.AddLine(x + curX, y + curY, x + nextX, y + nextY);
		}
	}

	bool IShape::Contains(float x, float y) const
	{
		float ex[MAX_EDGES];
		float ey[MAX_EDGES];

		int numEdges = 0;
		ComputeEdges(ex, ey, numEdges);

		for (int i = 0; i < numEdges; ++i)
		{
			const float curX = ex[i];
			const float curY = ey[i];

			const float nextX = ex[(i + 1) % numEdges];
			const float nextY = ey[(i + 1) % numEdges];

			if (!helper::IsBehindEdge(curX, curY, nextX, nextY, x, y))
				return false;
		}

		return true;
	}

	bool IShape::CheckCollision(float thisShapeX, float thisShapeY, float otherShapeX, float otherShapeY, const IShape* otherShape) const
	{
		float ex[MAX_EDGES];
		float ey[MAX_EDGES];

		int numEdges = 0;
		ComputeEdges(ex, ey, numEdges);

		for (int i = 0; i < numEdges; ++i)
		{
			const float curX = ex[i] + thisShapeX;
			const float curY = ey[i] + thisShapeY;

			if (otherShape->Contains(curX - otherShapeX, curY - otherShapeY))
				return true;
		}

		return false;
	}

	bool IShape::TestOverlap(const IShape* a, float ax, float ay, const IShape* b, float bx, float by)
	{
		if (a->CheckCollision(ax, ay, bx, by, b))
			return true;

		if (b->CheckCollision(bx, by, ax, ay, a))
			return true;

		return false;
	}

	// --- TriShape Implementations ---

	TriShape::TriShape(const float size)
		: m_size(size)
		// FIX: Initialize m_radius member variable
		, m_radius(size / 2.0f)
	{
	}

	void TriShape::ComputeEdges(float* x, float* y, int& numEdges) const
	{
		numEdges = 3;

		x[0] = -m_size / 2.0f;
		y[0] = m_size / 2.0f;

		x[1] = m_size / 2.0f;
		y[1] = m_size / 2.0f;

		x[2] = 0.0f;
		y[2] = -m_size / 2.0f;
	}

	// NOTE: GetRadius should be defined inline in the header for cleaner code,
	// but if it MUST be here, it should be defined correctly (and cannot be 
	// defined here AND inline in the header). Assuming it's defined in the header.
	/*
	float TriShape::GetRadius() const
	{
		return m_size / 2.0f;
	}
	*/

	// --- QuadShape Implementations (Square) ---

	QuadShape::QuadShape(const float size)
		: m_size(size)
		// FIX: Initialize m_radius member variable
		, m_radius(size * 0.70710678f) // R = size * sqrt(2) / 2
	{
	}

	void QuadShape::ComputeEdges(float* x, float* y, int& numEdges) const
	{
		numEdges = 4;

		x[0] = m_size / 2.0f;
		y[0] = m_size / 2.0f;

		x[1] = m_size / 2.0f;
		y[1] = -m_size / 2.0f;

		x[2] = -m_size / 2.0f;
		y[2] = -m_size / 2.0f;

		x[3] = -m_size / 2.0f;
		y[3] = m_size / 2.0f;
	}

	// NOTE: GetRadius should be defined inline in the header.
	/*
	float QuadShape::GetRadius() const
	{
		return m_size * 0.70710678f; 
	}
	*/

	// --- HexShape Implementations ---

	HexShape::HexShape(const float size)
		: m_size(size)
		// FIX: Initialize m_radius member variable
		, m_radius(size / 2.0f)
	{
	}

	void HexShape::ComputeEdges(float* x, float* y, int& numEdges) const
	{
		numEdges = 6;

		const float R = m_size / 2.0f;

		x[0] = 0.0f;
		y[0] = R;

		x[1] = R * 0.866f;
		y[1] = R * 0.5f;

		x[2] = R * 0.866f;
		y[2] = R * -0.5f;

		x[3] = 0.0f;
		y[3] = -R;

		x[4] = R * -0.866f;
		y[4] = R * -0.5f;

		x[5] = R * -0.866f;
		y[5] = R * 0.5f;
	}

	// NOTE: GetRadius should be defined inline in the header.
	/*
	float HexShape::GetRadius() const
	{
		return m_size / 2.0f;
	}
	*/

} // test