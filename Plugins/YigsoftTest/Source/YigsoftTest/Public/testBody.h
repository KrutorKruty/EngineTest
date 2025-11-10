#pragma once

#include <vector>
#include "testShape.h"
#include "testApp.h"


namespace app
{
    class RenderFrame;
    struct AppInitContext;

}

namespace test
{

    extern const float BodySpeed;
    extern const float AttractorRange;
    extern const float Gravitation;

    class Body
    {
    public:
        
        
        Body(IShape* shape, const unsigned int color, float x, float y, int shapeTypeID);
        int GetShapeTypeID() const { return m_shapeTypeID; }
        void Update(const std::vector< Body* >& otherBodies, int currentScenario);
        void Render(app::RenderFrame& frame);
        float GetRadius() const;
        void Integrate(float deltaTime);

        // Existing Public Getters
        float GetX() const { return m_x; }
        float GetY() const { return m_y; }

        // NEW: Public Getters for Shape and Color (Used by UFrameworkWrapper)
        const IShape* GetShape() const { return m_shape; }
        unsigned int GetColor() const { return m_color; }


    private:

        int m_shapeTypeID;
        bool FindAttractor(const std::vector< Body* >& otherBodies, int currentScenario, float& outDirX, float& outDirY) const;
        void SolveAttraction(float dirX, float dirY);
        void SolveCollision(const std::vector< Body* >& otherBodies);
        void WrapAround();

    public: // Changed access for public getters to private members
        // It's common practice to keep private data private, 
        // but since you were accessing m_shape and m_color directly in the wrapper previously, 
        // I'm changing these to private and adding getters, which is cleaner C++ design. 

        IShape* m_shape; // Leaving public for now if external framework requires it
        unsigned int m_color;
        float m_x;
        float m_y;
        float m_velX;
        float m_velY;
    };
}