#ifndef __MATH__
#define __MATH__
namespace ng {
class Vector2d
{
public:
    float x;
    float y;
    Vector2d(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) {}

    float getMagnitude() const;
    void normalize();
    
    void scale(float value);
    Vector2d operator+(const Vector2d *other) const;
};
}
#endif