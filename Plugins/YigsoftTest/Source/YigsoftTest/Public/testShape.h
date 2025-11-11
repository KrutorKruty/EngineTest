#pragma once

// Keep the includes for the framework types needed by the declarations
#include "../external/framework/include/app.h"


namespace test
{
	// Define the constant here, or it can be a static const in IShape, 
	// but putting it here is simplest if it's used globally in the namespace.
	const int MAX_EDGES_ASSUMED = 6;


	class IShape
	{
	public:
		IShape() {};
		virtual ~IShape() {};

		virtual float GetRadius() const = 0;

		// CRITICAL FIX: The Body constructor requires this virtual function.
		virtual float getMass() const = 0;

		/// Render shape at given position
		void Render(float x, float y, app::RenderFrame& frame) const; // DECLARATION ONLY

		/// Is given point inside the shape ?
		bool Contains(float x, float y) const; // DECLARATION ONLY

		/// Is this shape overlapping with the other shape ? (half of the test)
		bool CheckCollision(float thisShapeX, float thisShapeY, float otherShapeX, float otherShapeY, const IShape* otherShape) const; // DECLARATION ONLY

		/// are two shapes overlapping
		static bool TestOverlap(const IShape* a, float ax, float ay, const IShape* b, float bx, float by); // DECLARATION ONLY

		/// get assigned type of this shape
		virtual int GetType() const = 0;

		// -----------------------------------------------------------------
		// FIX IMPLEMENTED: Moved to public access
		// This function is needed by UFrameworkWrapper for debug drawing.
		// -----------------------------------------------------------------
		virtual void ComputeEdges(float* x, float* y, int& numEdges) const = 0;

	private:
		// Move this if it was intended to be private to IShape
		// static const int MAX_EDGES = 6; 
	};

	// --- Concrete Shapes ---

	class TriShape : public IShape
	{
	public:
		TriShape(const float size); // DECLARATION ONLY
		virtual float GetRadius() const override { return m_radius; }
		virtual float getMass() const override { return m_radius * m_radius; } // FIX: Required implementation

		// The override definitions below are fine as they were, they are defined in public.
		virtual void ComputeEdges(float* x, float* y, int& numEdges) const override;
		virtual int GetType() const override { return 0; }

	private:
		float	m_size;
		float m_radius;
	};

	class QuadShape : public IShape
	{
	public:
		QuadShape(const float size); // DECLARATION ONLY
		virtual float GetRadius() const override { return m_radius; }
		virtual float getMass() const override { return m_radius * m_radius; } // FIX: Required implementation

		virtual void ComputeEdges(float* x, float* y, int& numEdges) const override;
		virtual int GetType() const override { return 1; }

	private:
		float	m_size;
		float m_radius;
	};

	class HexShape : public IShape
	{
	public:
		HexShape(const float size); // DECLARATION ONLY
		virtual float GetRadius() const override { return m_radius; }
		virtual float getMass() const override { return m_radius * m_radius; } // FIX: Required implementation

		virtual void ComputeEdges(float* x, float* y, int& numEdges) const override;
		virtual int GetType() const override { return 2; }

	private:
		float	m_size;
		float m_radius;
	};

} // test