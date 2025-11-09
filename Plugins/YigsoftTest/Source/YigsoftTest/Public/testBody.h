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
        Body(IShape* shape, const unsigned int color, float x, float y);
        ~Body() { delete m_shape; }

        void Update(const std::vector< Body* >& otherBodies);
        void Render(app::RenderFrame& frame);
        float GetRadius() const;
        void Integrate(float deltaTime);
        float GetX() const { return m_x; }
        float GetY() const { return m_y; }

    private:

        bool FindAttractor(const std::vector< Body* >& otherBodies, float& outDirX, float& outDirY) const;
        void SolveAttraction(float dirX, float dirY);
        void SolveCollision(const std::vector< Body* >& otherBodies);
        void WrapAround();

    private:

        IShape* m_shape;
        unsigned int m_color;
        float m_x;
        float m_y;
        float m_velX;
        float m_velY;
    };
}