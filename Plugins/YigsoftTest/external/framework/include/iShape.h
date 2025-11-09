#pragma once

// Assumes 'RenderFrame' is defined in the framework
namespace app 
{
    class RenderFrame; 
}

namespace test
{
    // Interface for all physics shapes (Circle, Box, etc.)
    class IShape
    {
    public:
        // Must be virtual for correct memory cleanup
        virtual ~IShape() = default;

        // Used in Body::FindAttractor
        virtual int GetType() const = 0; 
        
        // Used in Body::Render
        virtual void Render( float x, float y, app::RenderFrame& frame ) const = 0;

        // CRITICAL: Used by Body::GetRadius() for Unreal Debug Draw
        virtual float GetRadius() const = 0; 
        
        // Used in Body::SolveCollision
        static bool TestOverlap( const IShape* s1, float x1, float y1, 
                                 const IShape* s2, float x2, float y2 );
    };
}