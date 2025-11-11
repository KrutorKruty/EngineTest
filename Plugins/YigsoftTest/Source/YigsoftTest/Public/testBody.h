#pragma once


#include "testShape.h" 


namespace std {
    // This allows the compiler to see the reference type without the definition
    template <typename T, typename Allocator>
    class vector;
}

namespace app
{

    class RenderFrame;
    struct AppInitContext;
}

namespace test
{

    class Body;


    template <typename T>
    class vector;


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

        float GetX() const { return m_x; }
        float GetY() const { return m_y; }
        const IShape* GetShape() const { return m_shape; }
        unsigned int GetColor() const { return m_color; }


    private:
       
        bool FindAttractor(const std::vector< Body* >& otherBodies, int currentScenario, float& outDirX, float& outDirY) const;
       

        void SolveAttraction(float dirX, float dirY);
        void SolveCollision(const std::vector< Body* >& otherBodies);
        void WrapAround();

    public:
        IShape* m_shape;
        unsigned int m_color;
        float m_x;
        float m_y;
        float m_velX;
        float m_velY;
        int m_shapeTypeID;
    };
}