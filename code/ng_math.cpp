
#include "ng_math.h"

namespace ng {
    
    float Vector2d::getMagnitude() const
    {
        return sqrt(x*x + y*y);
    }

    void Vector2d::normalize()
    {
        float magnitude = getMagnitude();
        if (magnitude == 0)
            return;

        x = x / magnitude;
        y = y / magnitude;
    }

    void Vector2d::scale(float value)
    {
        x = x * value;
        y = y * value;
    }

    Vector2d Vector2d::operator+(const Vector2d *other) const
    {
        Vector2d newVector = Vector2d();
        newVector.x = x + other->x;
        newVector.y = y + other->y;
        return newVector;
    }
}